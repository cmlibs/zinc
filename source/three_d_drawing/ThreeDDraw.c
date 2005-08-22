/*******************************************************************************
FILE : ThreeDDraw.c

LAST MODIFIED : 03 May 2004

DESCRIPTION :
A simple widget suitable for using with OpenGL.

==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xatom.h>
#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#include <X11/StringDefs.h>
#include <X11/Xresource.h>
#include "three_d_drawing/ThreeDDraP.h"
#include "user_interface/message.h"
#include "general/debug.h"

/*
Module variables
----------------
*/

/*
Resources
---------
*/
static XtResource resources[]=
/*******************************************************************************
LAST MODIFIED : 04 May 2004

DESCRIPTION :
Resources for the 3-D drawing widget.
==============================================================================*/
{
	{
		XtNvisual,
		XtCVisual,
		XtRVisual,
		sizeof(Visual *),
		XtOffsetOf(ThreeDDrawingRec,three_d_drawing.normal_buffer.visual),
		XtRImmediate,
		NULL
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
	}
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
	int depth, i,number_of_colour_map_windows;
	ThreeDDrawingWidget drawing_widget;
	Position x,y;
	unsigned int number_of_children;
	Window *children,*colour_map_windows,*new_colour_map_windows,parent_window,
		root_window,top_level_window,window;
	Visual *visual;
	X3dThreeDDrawCallbackStruct call_data;

	window = (Window)NULL;
	if (drawing_widget=(ThreeDDrawingWidget)widget)
	{
		/* if the widget doesn't already have a window */
		if (None==(drawing_widget->core).window)
		{
			display=XtDisplay(drawing_widget);
			x = (drawing_widget->core).x;
			y = (drawing_widget->core).y;
			if ((depth = (drawing_widget->core).depth) &&
				(visual= (drawing_widget->three_d_drawing).normal_buffer.visual) &&
				(width = (drawing_widget->core).width) && (height = (drawing_widget->core).height))
			{
				attributes->colormap=XCreateColormap(display,
					RootWindow(display,XScreenNumberOfScreen(XtScreen(widget))),
					visual, AllocNone);
				*value_mask |= CWColormap;
				if (None!=attributes->colormap)
				{
					/* best performance if no backing store */
					attributes->backing_store=NotUseful;
					*value_mask |= CWBackingStore;
					/* create the window */
					if (window=XCreateWindow(display,XtWindow(XtParent(drawing_widget)),
						x,y,width,height,0,depth,
						(unsigned int)InputOutput,visual,*value_mask,
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
			display_message(ERROR_MESSAGE,"ThreeDDrawingRealize.  Could not set colour map windows list\n");
							}
							XFree(colour_map_windows);
							free(new_colour_map_windows);
						}
						else
						{
							/* a list of colour map windows dose not exist */
							if (!XSetWMColormapWindows(display,top_level_window,&window,1))
							{
			display_message(ERROR_MESSAGE,"ThreeDDrawingRealize.  Could not set colour map windows list\n");
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"ThreeDDrawingRealize.  Could not create window\n");
						XFreeColormap(display,attributes->colormap);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"ThreeDDrawingRealize.  Could not create colour map\n");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
		"ThreeDDrawingRealize.  Missing visual information and/or graphics info\n");
			}
			/* Stop this from crashing if the visual wasn't found. */
			if (window)
			{
				/* set up the callback data */
				call_data.reason=X3dCR_INITIALIZE;
				call_data.event=(XEvent *)NULL;
				call_data.window=XtWindow(widget);
				/*???DB.  May need something different for multiple buffers or if want
				  to pass GL window id */
				XtCallCallbackList(widget,(((ThreeDDrawingWidget)widget)->
					three_d_drawing).normal_buffer.initialize_callback,
					(XtPointer)&call_data);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"ThreeDDrawingRealize.  Missing widget\n");
	}
} /* ThreeDDrawingRealize */

static void ThreeDDrawingClassInitialize(void)
/*******************************************************************************
LAST MODIFIED : 29 April 1994

DESCRIPTION :
The class initialize method.
==============================================================================*/
{
	/* register type converters */
} /* ThreeDDrawingClassInitialize */

static void ThreeDDrawingInitialize(Widget request_widget,Widget new_widget,
	ArgList args,Cardinal *num_args)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
The initialize method.
==============================================================================*/
{
	USE_PARAMETER(args);
	USE_PARAMETER(num_args);
	if (!request_widget || !new_widget)
	{
		display_message(ERROR_MESSAGE,"ThreeDDrawingInitialize.  Invalid argument(s)\n");
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
	X3dThreeDDrawCallbackStruct call_data;

	USE_PARAMETER(region);
	/* set up the callback data */
	call_data.reason=X3dCR_EXPOSE;
	call_data.event=event;
	call_data.window=XtWindow(widget);
	XtCallCallbackList(widget,(((ThreeDDrawingWidget)widget)->three_d_drawing).
		normal_buffer.expose_callback,(XtPointer)&call_data);

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

static void ThreeDDrawingResize(Widget widget)
/*******************************************************************************
LAST MODIFIED : 11 February 1995

DESCRIPTION :
The resize method.
==============================================================================*/
{
	X3dThreeDDrawCallbackStruct call_data;

	/* set up the callback data */
	call_data.reason=X3dCR_RESIZE;
	call_data.event=(XEvent *)NULL;
	call_data.window=XtWindow(widget);
		/*???DB.  May need something different for multiple buffers or if want to
			pass GL window id */
	XtCallCallbackList(widget,
		(((ThreeDDrawingWidget)widget)->three_d_drawing).resize_callback,
		(XtPointer)&call_data);
} /* ThreeDDrawingResize */

static void ThreeDDrawingDestroy(Widget widget)
/*******************************************************************************
LAST MODIFIED : 17 November 1997

DESCRIPTION :
The destroy method.
==============================================================================*/
{
	USE_PARAMETER(widget);

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
LAST MODIFIED : 27 May 2004

DESCRIPTION :
Returns true if the X3dThreeD <widget> is initialised correctly.
==============================================================================*/
{
	int return_code;

	return_code = 0;
	if (True==IsThreeDDrawing(widget))
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"X3dThreeDDrawingMakeCurrent.  Missing or invalid widget\n");
		return_code = 0;
	}

	return (return_code);
} /* X3dThreeDisInitialised */
