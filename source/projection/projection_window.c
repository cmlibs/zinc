/*******************************************************************************
FILE : projection_window.c

LAST MODIFIED : 29 January 1999

DESCRIPTION :
???DB.  Started as mapping_window.c in emap
==============================================================================*/
#include <stddef.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xlib.h>
#include <X11/Composite.h>
#include <X11/Shell.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/LabelG.h>
#include <Xm/DialogS.h>
#include <Xm/DrawingA.h>
#include <Xm/RowColumn.h>
#include <Xm/PushBG.h>
#include <Xm/TextF.h>
#include <Xm/ToggleBG.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>
#include "command/parser.h"
#include "general/debug.h"
#include "general/image_utilities.h"
#include "general/postscript.h"
#include "graphics/spectrum.h"
#include "projection/projection.h"
#include "projection/projection_dialog.h"
#include "projection/projection_window.h"
#include "projection/projection_window.uidh"
#include "user_interface/filedir.h"
#include "user_interface/message.h"
#include "user_interface/printer.h"
#include "user_interface/user_interface.h"

/*
Module constants
----------------
*/
#define MAX_SPECTRUM_COLOURS 256
	/*???DB.  Duplicated from projection.c .  Not good */

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int projection_window_hierarchy_open=0;
static MrmHierarchy projection_window_hierarchy;
	/*???DB.  To start with only have one projection window */
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/

static void identify_projection_configure_b(Widget *widget,
	XtPointer projection_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Finds the id of the projection configure button.
==============================================================================*/
{
	struct Projection_window *window;

	ENTER(identify_projection_configure_b);
	USE_PARAMETER(call_data);
	if (window=(struct Projection_window *)projection_window)
	{
		window->configure_button= *widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_projection_configure_b.  Missing projection_window");
	}
	LEAVE;
} /* identify_projection_configure_b */

static void configure_projection(Widget widget,XtPointer projection_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Opens the dialog box associated with the configure button in the projection
window.
==============================================================================*/
{
	struct Projection_dialog *projection_dialog;
	struct Projection_window *window;
	static MrmRegisterArg identifier_list[]=
		{
			{"projection_window_structure",NULL}
		};

	ENTER(configure_projection);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((window=(struct Projection_window *)projection_window)&&
		window->projection)
	{
		if (!(projection_dialog=window->configure_dialog))
		{
			/* assign and register the identifiers */
			identifier_list[0].value=(XtPointer)window;
			if (MrmSUCCESS==MrmRegisterNamesInHierarchy(projection_window_hierarchy,
				identifier_list,XtNumber(identifier_list)))
			{
				projection_dialog=create_Projection_dialog(&(window->configure_dialog),
					window,window->configure_button,window->user_interface);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"configure_projection.  Could not register identifiers");
			}
		}
		if (projection_dialog)
		{
			open_projection_dialog(projection_dialog);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"configure_projection.  Missing projection_window");
	}
	LEAVE;
} /* configure_projection */

static void identify_projection_animate_but(Widget *widget,
	XtPointer projection_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 June 1995

DESCRIPTION :
Finds the id of the projection animate button.
==============================================================================*/
{
	struct Projection_window *window;

	ENTER(identify_projection_animate_but);
	USE_PARAMETER(call_data);
	if (window=(struct Projection_window *)projection_window)
	{
		window->animate_button= *widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_projection_animate_but.  Missing projection_window");
	}
	LEAVE;
} /* identify_projection_animate_but */

static void draw_animation_frame(XtPointer projection_window,
	XtIntervalId *timer_id)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Draws a frame in the projection animation.
==============================================================================*/
{
	Display *display;
	float contour_maximum,contour_minimum,maximum_value,minimum_value;
	int cell_number,i,number_of_contours,number_of_spectrum_colours;
	Pixel *spectrum_pixels;
	struct Projection_window *window;
	struct Projection *projection;
	XColor colour,spectrum_rgb[MAX_SPECTRUM_COLOURS];

	ENTER(draw_animation_frame);
	USE_PARAMETER(timer_id);
	if ((window=(struct Projection_window *)projection_window)&&
		(projection=window->projection))
	{
		number_of_spectrum_colours=projection->number_of_spectrum_colours;
		if ((0<=window->animation_front)&&
			(window->animation_front<number_of_spectrum_colours))
		{
			display=window->user_interface->display;
			/* use background drawing colour for the whole spectrum */
			colour.pixel=(projection->pixel).background_colour;
			XQueryColor(display,projection->colour_map,&colour);
			spectrum_pixels=(projection->pixel).spectrum;
			for (i=0;i<number_of_spectrum_colours;i++)
			{
				spectrum_rgb[i].pixel=spectrum_pixels[i];
				spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
				spectrum_rgb[i].red=colour.red;
				spectrum_rgb[i].blue=colour.blue;
				spectrum_rgb[i].green=colour.green;
			}
			if ((SHOW_CONTOURS==projection->contours_option)&&
				((VARIABLE_THICKNESS==projection->contour_thickness)||
				!(projection->pixel_values)))
			{
				colour.pixel=(projection->pixel).contour_colour;
				XQueryColor(display,projection->colour_map,&colour);
				number_of_contours=projection->number_of_contours;
				maximum_value=get_Spectrum_maximum(projection->spectrum);
				minimum_value=get_Spectrum_minimum(projection->spectrum);
				contour_maximum=projection->contour_maximum;
				contour_minimum=projection->contour_minimum;
				number_of_contours=projection->number_of_contours;
				for (i=0;i<number_of_contours;i++)
				{
					cell_number=(int)(((contour_maximum*(float)i+contour_minimum*
						(float)(number_of_contours-1-i))/(float)(number_of_contours-1)-
						minimum_value)/(maximum_value-minimum_value)*
						(float)(number_of_spectrum_colours-1)+0.5);
					spectrum_rgb[cell_number].pixel=spectrum_pixels[cell_number];
					spectrum_rgb[cell_number].flags=DoRed|DoGreen|DoBlue;
					spectrum_rgb[cell_number].red=colour.red;
					spectrum_rgb[cell_number].blue=colour.blue;
					spectrum_rgb[cell_number].green=colour.green;
				}
			}
			/* show the animation front */
			colour.pixel=(projection->pixel).contour_colour;
			XQueryColor(display,projection->colour_map,&colour);
			i=window->animation_front;
			spectrum_rgb[i].pixel=spectrum_pixels[i];
			spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
			spectrum_rgb[i].red=colour.red;
			spectrum_rgb[i].blue=colour.blue;
			spectrum_rgb[i].green=colour.green;
			XStoreColors(display,projection->colour_map,spectrum_rgb,
				number_of_spectrum_colours);
			/* show the map boundary */
			colour.pixel=(projection->pixel).boundary_colour;
			colour.flags=DoRed|DoGreen|DoBlue;
			XStoreColor(display,projection->colour_map,&colour);
			(window->animation_front)++;
			if (window->animation_front<number_of_spectrum_colours)
			{
				(void)XtAppAddTimeOut(window->user_interface->application_context,
					(long unsigned)100,draw_animation_frame,projection_window);
			}
			else
			{
				window->animation_front= -1;
				XtSetSensitive(window->animate_button,True);
				(void)update_colour_map(projection);
			}
		}
	}
	LEAVE;
} /* draw_animation_frame */

static void animate_projection(Widget widget,XtPointer projection_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Starts the projection animation.
==============================================================================*/
{
	struct Projection_window *window;

	ENTER(animate_projection);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((window=(struct Projection_window *)projection_window)&&
		(window->projection))
	{
		/* only one animation at a time */
		XtSetSensitive(window->animate_button,False);
		/* initialize the animation front */
		window->animation_front=0;
		/* start the animation */
		draw_animation_frame(projection_window,(XtIntervalId *)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"animate_projection.  Invalid projection_window");
	}
	LEAVE;
} /* animate_projection */

static void identify_projection_print_butto(Widget *widget,
	XtPointer projection_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Finds the id of the projection print button.
==============================================================================*/
{
	struct Projection_window *window;

	ENTER(identify_projection_print_butto);
	USE_PARAMETER(call_data);
	if (window=(struct Projection_window *)projection_window)
	{
		window->print_button= *widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_projection_print_butto.  Missing projection_window");
	}
	LEAVE;
} /* identify_projection_print_butto */

static void identify_projection_print_posts(Widget *widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 May 1997

DESCRIPTION :
Finds the id of the projection print postscript button.
==============================================================================*/
{
	struct Projection_window *projection;

	ENTER(identify_projection_print_posts);
	USE_PARAMETER(call_data);
	if (projection=(struct Projection_window *)client_data)
	{
		projection->print_menu.postscript_button= *widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_projection_print_posts.  client_data missing");
	}
	LEAVE;
} /* identify_projection_print_posts */

static void identify_projection_print_rgb_b(Widget *widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 May 1997

DESCRIPTION :
Finds the id of the projection print rgb button.
==============================================================================*/
{
	struct Projection_window *projection;

	ENTER(identify_projection_print_rgb_b);
	USE_PARAMETER(call_data);
	if (projection=(struct Projection_window *)client_data)
	{
		projection->print_menu.rgb_button= *widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_projection_print_rgb_b.  client_data missing");
	}
	LEAVE;
} /* identify_projection_print_rgb_b */

static void identify_projection_print_tiff(Widget *widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
Finds the id of the projection print tiff button.
==============================================================================*/
{
	struct Projection_window *projection;

	ENTER(identify_projection_print_tiff);
	USE_PARAMETER(call_data);
	if (projection=(struct Projection_window *)client_data)
	{
		projection->print_menu.tiff_button= *widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_projection_print_tiff.  client_data missing");
	}
	LEAVE;
} /* identify_projection_print_tiff */

static void identify_projection_close_butto(Widget *widget,
	XtPointer projection_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 1 November 1995

DESCRIPTION :
Finds the id of the projection close button.
==============================================================================*/
{
	struct Projection_window *window;

	ENTER(identify_projection_close_butto);
	USE_PARAMETER(call_data);
	if (window=(struct Projection_window *)projection_window)
	{
		window->close_button= *widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_projection_close_butto.  Missing projection_window");
	}
	LEAVE;
} /* identify_projection_close_butto */

static void identify_projection_drawing_are(Widget *widget,
	XtPointer projection_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Finds the id of the projection drawing area.
==============================================================================*/
{
	struct Projection_window *window;

	ENTER(identify_projection_drawing_are);
	USE_PARAMETER(call_data);
	if (window=(struct Projection_window *)projection_window)
	{
		window->projection_drawing_area= *widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_projection_drawing_are.  Missing projection_window");
	}
	LEAVE;
} /* identify_projection_drawing_are */

static void expose_projection_drawing_area(Widget widget,
	XtPointer projection_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
The callback for redrawing part of a projection drawing area.
==============================================================================*/
{
	Display *display;
#if defined (DEBUG)
	static int first_call=1;
#endif
	struct Projection_window *window;
	XmDrawingAreaCallbackStruct *callback;
	XExposeEvent *event;
	XWindowAttributes attributes;

	ENTER(expose_projection_drawing_area);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (window=(struct Projection_window *)projection_window)
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_EXPOSE==callback->reason)
			{
				if (callback->event)
				{
					if (Expose==callback->event->type)
					{
						display=window->user_interface->display;
#if defined (DEBUG)
						if (first_call&&(window->projection))
						{
							drawing_window=XtWindow(widget);
/*???debug */
printf("setting colour map %p %p %p\n",display,drawing_window,
	window->projection->colour_map);
							XSetWindowColormap(display,drawing_window,
								window->projection->colour_map);
							XtVaSetValues(widget,XmNcolormap,window->projection->colour_map,
								NULL);
							/* find the top level window */
							parent_window=drawing_window;
							do
							{
								top_level_window=parent_window;
								XQueryTree(display,top_level_window,&root_window,&parent_window,
									&children,&number_of_children);
								XFree(children);
							} while (parent_window!=root_window);
/*???debug */
printf("%p %p %p %p\n",drawing_window,top_level_window,parent_window,
	root_window);
/*              XSetWindowColormap(display,top_level_window,
								window->projection->colour_map);*/
							/* add the window to the list of windows that need special colour
								maps (used by the window manager) */
							if (XGetWMColormapWindows(display,top_level_window,
								&colour_map_windows,&number_of_colour_map_windows)&&
								ALLOCATE(new_colour_map_windows,Window,
								number_of_colour_map_windows+1))
							{
/*???debug */
printf("%d colour map windows\n",number_of_colour_map_windows);
								/* a list of colour map windows already exists */
								new_colour_map_windows[0]=drawing_window;
								for (i=1;i<=number_of_colour_map_windows;i++)
								{
									new_colour_map_windows[i]=colour_map_windows[i];
								}
								if (!XSetWMColormapWindows(display,top_level_window,
									new_colour_map_windows,number_of_colour_map_windows+1))
								{
									display_message(ERROR_MESSAGE,
		"expose_projection_drawing_area.  Could not set colour map windows list\n");
								}
								XFree(colour_map_windows);
								DEALLOCATE(new_colour_map_windows);
							}
							else
							{
/*???debug */
printf("1 colour map window\n");
								/* a list of colour map windows dose not exist */
								if (!XSetWMColormapWindows(display,top_level_window,
									&drawing_window,1))
								{
									display_message(ERROR_MESSAGE,
		"expose_projection_drawing_area.  Could not set colour map windows list\n");
								}
							}
							first_call=0;
						}
#endif
						event= &(callback->event->xexpose);
						if (window->projection_drawing_area)
						{
							if (!(window->projection_drawing))
							{
								/* determine the size of the drawing area */
								XGetWindowAttributes(display,
									XtWindow(window->projection_drawing_area),&attributes);
								/* create a pixel map */
								if (window->projection_drawing=
									create_Drawing_2d(window->projection_drawing_area,
									attributes.width,attributes.height,DRAWING_IMAGE,
									window->user_interface))
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
									create_Projection_drawing(window->projection_drawing_area,
									attributes.width,attributes.height,1,window->user_interface))
#endif /* defined (OLD_CODE) */
								{
									/* draw the projection */
									draw_projection(window->projection,2,
										window->projection_drawing);
								}
								else
								{
									display_message(ERROR_MESSAGE,
									"expose_projection_drawing_area.  Could not create drawing");
								}
							}
							/* redisplay the specified part of the pixmap */
							if (window->projection_drawing)
							{
								XCopyArea(display,window->projection_drawing->pixel_map,
									XtWindow(window->projection_drawing_area),
									(window->projection->graphics_context).copy,event->x,event->y,
									event->width,event->height,event->x,event->y);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"expose_projection_drawing_area.  Missing drawing area");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"expose_projection_drawing_area.  Incorrect event reason");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"expose_projection_drawing_area.  Missing event");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"expose_projection_drawing_area.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"expose_projection_drawing_area.  Missing call_data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"expose_projection_drawing_area.  Missing projection_window");
	}
	LEAVE;
} /* expose_projection_drawing_area */

static void resize_projection_drawing_area(Widget widget,
	XtPointer projection_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 May 1997

DESCRIPTION :
The callback for resizing a projection drawing area.
==============================================================================*/
{
	Display *display;
	int width,height;
	struct Projection_window *window;
	XmDrawingAreaCallbackStruct *callback;
	XWindowAttributes attributes;

	ENTER(resize_projection_drawing_area);
	USE_PARAMETER(widget);
	if (window=(struct Projection_window *)projection_window)
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_RESIZE==callback->reason)
			{
				/*??? during creation there are resize callbacks without windows */
				if (callback->window)
				{
					if (window->projection_drawing_area)
					{
						display=window->user_interface->display;
						/* find the size of the old rectangle */
						if (window->projection_drawing)
						{
							width=window->projection_drawing->width;
							height=window->projection_drawing->height;
							/* destroy the old pixmap */
							destroy_Drawing_2d(&(window->projection_drawing));
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
							destroy_Projection_drawing(&(window->projection_drawing));
#endif /* defined (OLD_CODE) */
						}
						else
						{
							width=0;
							height=0;
						}
						/* find the size of the new rectangle */
						XGetWindowAttributes(display,callback->window,&attributes);
						/* create a new pixmap */
						if (window->projection_drawing=create_Drawing_2d(
							window->projection_drawing_area,attributes.width,
							attributes.height,DRAWING_IMAGE,window->user_interface))
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
						if (window->projection_drawing=create_Projection_drawing(
							window->projection_drawing_area,attributes.width,
							attributes.height,1,window->user_interface))
#endif /* defined (OLD_CODE) */
						{
							/* redraw the map */
							draw_projection(window->projection,2,window->projection_drawing);
							/* display the intersection of the old rectangle and the new
								rectangle */
							if (attributes.width<width)
							{
								width=attributes.width;
							}
							if (attributes.height<height)
							{
								height=attributes.height;
							}
							XCopyArea(display,window->projection_drawing->pixel_map,
								XtWindow(window->projection_drawing_area),
								(window->projection->graphics_context).copy,0,0,width,height,0,
								0);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"resize_projection_drawing_area.  Missing drawing area");
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"resize_projection_drawing_area.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"resize_projection_drawing_area.  Missing call_data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"resize_projection_drawing_area.  Missing projection_window");
	}
	LEAVE;
} /* resize_projection_drawing_area */

static void identify_projection_spectrum_ar(Widget *widget,
	XtPointer projection_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Finds the id of the spectrum_drawing_area.
==============================================================================*/
{
	struct Projection_window *window;

	ENTER(identify_projection_spectrum_ar);
	USE_PARAMETER(call_data);
	if (window=(struct Projection_window *)projection_window)
	{
		window->spectrum_drawing_area= *widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_projection_spectrum_ar.  Missing projection_window");
	}
	LEAVE;
} /* identify_projection_spectrum_ar */

static void expose_projection_spectrum_area(Widget widget,
	XtPointer projection_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 3 January 1997

DESCRIPTION :
The callback for redrawing part of spectrum drawing area.
==============================================================================*/
{
	Display *display;
#if defined (DEBUG)
	static int first_call=1;
#endif
	struct Projection_window *window;
	XmDrawingAreaCallbackStruct *callback;
	XExposeEvent *event;
	XWindowAttributes attributes;

	ENTER(expose_projection_spectrum_area);
	USE_PARAMETER(widget);
	if (window=(struct Projection_window *)projection_window)
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_EXPOSE==callback->reason)
			{
				if (callback->event)
				{
					if (Expose==callback->event->type)
					{
						event= &(callback->event->xexpose);
						if (window->spectrum_drawing_area)
						{
							display=window->user_interface->display;
#if defined (DEBUG)
							if (first_call&&(window->projection))
							{
/*                XSetWindowColormap(display,XtWindow(widget),
									window->projection->colour_map);*/
								first_call=0;
							}
#endif
							if (!(window->spectrum_drawing))
							{
								/* determine the size of the drawing area */
								XGetWindowAttributes(display,
									XtWindow(window->spectrum_drawing_area),&attributes);
								/* create a pixel map */
								if (window->spectrum_drawing=create_Drawing_2d(
									window->spectrum_drawing_area,attributes.width,
									attributes.height,DRAWING_IMAGE,window->user_interface))
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
								if (window->spectrum_drawing=create_Projection_drawing(
									window->spectrum_drawing_area,attributes.width,
									attributes.height,1,window->user_interface))
#endif /* defined (OLD_CODE) */
								{
									/* draw the colour bar or the auxiliary devices */
									draw_spectrum_area(window->projection,
										window->spectrum_drawing);
								}
								else
								{
									display_message(ERROR_MESSAGE,
									"expose_projection_spectrum_area.  Could not create drawing");
								}
							}
							/* redisplay the specified part of the pixmap */
							if (window->spectrum_drawing)
							{
								XCopyArea(display,window->spectrum_drawing->pixel_map,
									XtWindow(window->spectrum_drawing_area),
									(window->projection->graphics_context).copy,event->x,event->y,
									event->width,event->height,event->x,event->y);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"expose_projection_spectrum_area.  Missing drawing area");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"expose_projection_spectrum_area.  Incorrect event reason");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"expose_projection_spectrum_area.  Missing event");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"expose_projection_spectrum_area.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"expose_projection_spectrum_area.  Missing call_data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"expose_projection_spectrum_area.  Missing projection_window");
	}
	LEAVE;
} /* expose_projection_spectrum_area */

static void resize_projection_spectrum_area(Widget widget,
	XtPointer projection_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 3 January 1997

DESCRIPTION :
The callback for resizing a spectrum drawing area.
==============================================================================*/
{
	Display *display;
	struct Projection_window *window;
	XmDrawingAreaCallbackStruct *callback;
	XWindowAttributes attributes;
	int width,height;

	ENTER(resize_projection_spectrum_area);
	USE_PARAMETER(widget);
	if (window=(struct Projection_window *)projection_window)
	{
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_RESIZE==callback->reason)
			{
				/*??? during creation there are resize callbacks without windows */
				if (callback->window)
				{
					if (window->spectrum_drawing_area)
					{
						display=window->user_interface->display;
						/* find the size of the old rectangle */
						if (window->spectrum_drawing)
						{
							width=window->spectrum_drawing->width;
							height=window->spectrum_drawing->height;
							/* destroy the old pixmap */
							destroy_Drawing_2d(&(window->spectrum_drawing));
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
							destroy_Projection_drawing(&(window->spectrum_drawing));
#endif /* defined (OLD_CODE) */
						}
						else
						{
							width=0;
							height=0;
						}
						/* find the size of the new rectangle */
						XGetWindowAttributes(display,callback->window,&attributes);
						/* create a new pixmap */
						if (window->spectrum_drawing=create_Drawing_2d(
							window->spectrum_drawing_area,attributes.width,attributes.height,
							DRAWING_IMAGE,window->user_interface))
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
						if (window->spectrum_drawing=create_Projection_drawing(
							window->spectrum_drawing_area,attributes.width,attributes.height,
							1,window->user_interface))
#endif /* defined (OLD_CODE) */
						{
							/* redraw the projection */
							draw_spectrum_area(window->projection,window->spectrum_drawing);
							/* display the intersection of the old rectangle and the new
								rectangle */
							if (attributes.width<width)
							{
								width=attributes.width;
							}
							if (attributes.height<height)
							{
								height=attributes.height;
							}
							XCopyArea(display,window->spectrum_drawing->pixel_map,
								XtWindow(window->spectrum_drawing_area),
								(window->projection->graphics_context).copy,0,0,width,height,0,
								0);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"resize_projection_spectrum_area.  Missing drawing area");
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"resize_projection_spectrum_area.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"resize_projection_spectrum_area.  Missing call_data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"resize_projection_spectrum_area.  Missing projection_window");
	}
	LEAVE;
} /* resize_projection_spectrum_area */

static void destroy_Projection_window(Widget widget,XtPointer projection_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 3 January 1997

DESCRIPTION :
This function expects <client_data> to be a pointer to a mapping window.  If the
<address> field of the mapping window is not NULL, <*address> is set to NULL.
If the <activation> field is not NULL, the <activation> widget is unghosted.
The function frees the memory associated with the fields of the mapping window
and frees the memory for the mapping window.
==============================================================================*/
{
	struct Projection_window *window;

	ENTER(destroy_Projection_window);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (window=(struct Projection_window *)projection_window)
	{
		/* set the pointer to the projection window to NULL */
		if (window->address)
		{
			*(window->address)=(struct Projection_window *)NULL;
		}
		/* unghost the creator widget */
		if (window->creator)
		{
			XtSetSensitive(window->creator,True);
		}
		/*??? destroy configure dialog */
		/* destroy the projection */
		DESTROY(Projection)(&(window->projection));
		/* destroy projection drawing */
		destroy_Drawing_2d(&(window->projection_drawing));
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
		destroy_Projection_drawing(&(window->projection_drawing));
#endif /* defined (OLD_CODE) */
		/* free the projection window memory */
		DEALLOCATE(window);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_Projection_window.  Missing projection_window");
	}
	LEAVE;
} /* destroy_Projection_window */

static int write_projection_postscript_fil(char *file_name,
	void *projection_window)
/*******************************************************************************
LAST MODIFIED : 31 May 1997

DESCRIPTION :
This function writes the postscript for drawing the projection associated with
the projection_window.
==============================================================================*/
{
	float page_height,page_width,pixel_aspect_ratio,postscript_page_height,
		postscript_page_width;
	int return_code;
	struct Printer printer;
	struct Projection *projection;
	struct Projection_window *window;

	ENTER(write_projection_postscript_fil);
	if ((window=(struct Projection_window *)projection_window)&&
		(projection=window->projection)&&(window->user_interface))
	{
		if (open_printer(&printer,window->user_interface))
		{
			/* open the postscript file */
			if (open_postscript(file_name,PORTRAIT,projection->colour_map,
				(projection->pixel).spectrum,projection->number_of_spectrum_colours,
				(projection->pixel).background_colour,&(printer.background_colour),
				(projection->pixel).contour_colour,projection->font,
				(float)(printer.page_width_mm),(float)(printer.page_height_mm),
				(float)(printer.page_left_margin_mm),
				(float)(printer.page_right_margin_mm),
				(float)(printer.page_top_margin_mm),
				(float)(printer.page_bottom_margin_mm),window->user_interface))
			{
				if (get_postscript_page_size(&postscript_page_width,
					&postscript_page_height))
				{
					projection->print=1;
					if ((SHOW_COLOUR==projection->colour_option)&&
						(projection->print_spectrum))
					{
						/* show colour bar */
						/* set the area of the postscript page for the projection */
						if (projection->maintain_aspect_ratio)
						{
							pixel_aspect_ratio=get_pixel_aspect_ratio(window->user_interface->
								display);
							if ((float)(window->projection_drawing->height)*
								pixel_aspect_ratio*postscript_page_width<
								0.85*postscript_page_height*
								(float)(window->projection_drawing->width))
							{
								page_height=postscript_page_width*pixel_aspect_ratio*
									(float)(window->projection_drawing->height)/
									(float)(window->projection_drawing->width);
								set_postscript_display_transfor(0,0.85*postscript_page_height-
									page_height,postscript_page_width,page_height,0,0,
									(float)(window->projection_drawing->width),
									(float)(window->projection_drawing->height));
							}
							else
							{
								page_width=0.85*postscript_page_height*
									(float)(window->projection_drawing->width)/
									(pixel_aspect_ratio*
									(float)(window->projection_drawing->height));
								set_postscript_display_transfor(
									(postscript_page_width-page_width)/2,0,page_width,
									0.85*postscript_page_height,0,0,
									(float)(window->projection_drawing->width),
									(float)(window->projection_drawing->height));
							}
						}
						else
						{
							set_postscript_display_transfor(0,0,postscript_page_width,
								0.85*postscript_page_height,0,0,
								(float)(window->projection_drawing->width),
								(float)(window->projection_drawing->height));
						}
						/* draw the projection to the postscript page */
						draw_projection(projection,0,window->projection_drawing);
						/* set the area of the postscript page for the colour bar */
						set_postscript_display_transfor(0,0.9*postscript_page_height,
							postscript_page_width,0.1*postscript_page_height,0,0,
							(float)(window->spectrum_drawing->width),
							(float)(window->spectrum_drawing->height));
						/* draw the colour bar to the postscript page */
						draw_spectrum_area(projection,window->spectrum_drawing);
					}
					else
					{
						/* no colour bar */
						/* set the area of the postscript page for the projection */
						if (projection->maintain_aspect_ratio)
						{
							pixel_aspect_ratio=get_pixel_aspect_ratio(window->user_interface->
								display);
							if ((float)(window->projection_drawing->height)*
								pixel_aspect_ratio*postscript_page_width<postscript_page_height*
								(float)(window->projection_drawing->width))
							{
								page_height=postscript_page_width*pixel_aspect_ratio*
									(float)(window->projection_drawing->height)/
									(float)(window->projection_drawing->width);
								set_postscript_display_transfor(0,postscript_page_height-
									page_height,postscript_page_width,page_height,0,0,
									(float)(window->projection_drawing->width),
									(float)(window->projection_drawing->height));
							}
							else
							{
								page_width=postscript_page_height*
									(float)(window->projection_drawing->width)/
									(pixel_aspect_ratio*
									(float)(window->projection_drawing->height));
								set_postscript_display_transfor(
									(postscript_page_width-page_width)/2,0,page_width,
									postscript_page_height,0,0,
									(float)(window->projection_drawing->width),
									(float)(window->projection_drawing->height));
							}
						}
						else
						{
							set_postscript_display_transfor(0,0,postscript_page_width,
								postscript_page_height,0,0,
								(float)(window->projection_drawing->width),
								(float)(window->projection_drawing->height));
						}
						/* draw the projection to the postscript page */
						draw_projection(projection,0,window->projection_drawing);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"write_map_postscript_file.  Could not get postscript page size");
					return_code=0;
				}
				/* write the ending for the postscript file */
				(void)close_postscript();
				projection->print=0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_map_postscript_file.  Could not open file");
				return_code=0;
			}
			close_printer(&printer);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_projection_postscript_fil.  Could not open printer");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"write_projection_postscript_fil.  Missing projection_window or projection or user_interface");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_projection_postscript_fil */

static int write_projection_rgb_file(char *file_name,void *projection_window)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
This function writes the rgb for drawing the projection associated with the
projection_window.
==============================================================================*/
{
	int return_code;
	struct Projection_window *window;
	unsigned long *image;

	ENTER(write_projection_rgb_file);
	if ((window = (struct Projection_window *)projection_window) &&
		 window->projection)
	{
		if (image=get_Drawing_2d_image(window->projection_drawing))
		{
			/* write the file */
#if defined (IMAGEMAGICK)
			char *extended_filename;

			if ((!strchr(file_name, ':'))&&
				(ALLOCATE(extended_filename, char, strlen(file_name) + 6)))
			{
				sprintf(extended_filename, "sgi:%s", file_name);
				return_code=write_image_file(extended_filename,/*components*/3, /*bytes_per_component*/1,
					window->projection_drawing->height,window->projection_drawing->width,
					0, image);
				DEALLOCATE(extended_filename);
			}
			else
			{
				return_code=write_image_file(file_name,/*components*/3, /*bytes_per_component*/1,
					window->projection_drawing->height,window->projection_drawing->width,
					0, image);
			}
#else /* defined (IMAGEMAGICK) */
			return_code=write_rgb_image_file(file_name,/*components*/3, /*bytes_per_component*/1,
				window->projection_drawing->height,window->projection_drawing->width,
				0, image);
#endif /* defined (IMAGEMAGICK) */
			DEALLOCATE(image);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_projection_rgb_file.  Could not get image");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_projection_rgb_file.  Missing projection_window or projection");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_projection_rgb_file */

static int write_projection_tiff_file(char *file_name,void *projection_window)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
This function writes the tiff for drawing the projection associated with the
projection_window.
==============================================================================*/
{
	int return_code;
	struct Projection_window *window;
	unsigned long *image;

	ENTER(write_projection_tiff_file);
	if ((window = (struct Projection_window *)projection_window) &&
		window->projection)
	{
		if (image=get_Drawing_2d_image(window->projection_drawing))
		{
			/* write the file */
#if defined (IMAGEMAGICK)
			char *extended_filename;

			if ((!strchr(file_name, ':'))&&
				(ALLOCATE(extended_filename, char, strlen(file_name) + 6)))
			{
				sprintf(extended_filename, "tiff:%s", file_name);
				return_code=write_image_file(extended_filename,/*components*/3, /*bytes_per_component*/1,
					window->projection_drawing->height,window->projection_drawing->width,
					0, image);
				DEALLOCATE(extended_filename);
			}
			else
			{
				return_code=write_image_file(file_name,/*components*/3, /*bytes_per_component*/1,
					window->projection_drawing->height,window->projection_drawing->width,
					0, image);
			}
#else /* defined (IMAGEMAGICK) */
			return_code=write_tiff_image_file(file_name,/*components*/3, /*bytes_per_component*/1,
				window->projection_drawing->height,window->projection_drawing->width,
				0, TIFF_PACK_BITS_COMPRESSION,image);
#endif /* defined (IMAGEMAGICK) */
			DEALLOCATE(image);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_projection_tiff_file.  Could not get image");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_projection_tiff_file.  Missing projection_window or projection");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_projection_tiff_file */

static struct Projection_window *create_Projection_window(
	struct Projection_window **address,Widget creator,Widget shell,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
This function allocates the memory for a Projection_window and sets the fields
to the specified values (<address>, <projection>).  It then retrieves a
projection window widget with the specified <shell> and assigns the widget ids
to the appropriate fields of the structure.  If successful it returns a pointer
to the created projection window and, if <address> is not NULL, makes <*address>
point to the created projection window.  If unsuccessful, NULL is returned.
==============================================================================*/
{
	MrmType projection_window_class;
	static MrmRegisterArg callback_list[]={
		{"destroy_Projection_window",(XtPointer)destroy_Projection_window},
		{"identify_projection_configure_b",
			(XtPointer)identify_projection_configure_b},
		{"configure_projection",(XtPointer)configure_projection},
		{"identify_projection_animate_but",
			(XtPointer)identify_projection_animate_but},
		{"animate_projection",(XtPointer)animate_projection},
		{"identify_projection_print_butto",
			(XtPointer)identify_projection_print_butto},
		{"identify_projection_print_posts",
			(XtPointer)identify_projection_print_posts},
		{"identify_projection_print_rgb_b",
			(XtPointer)identify_projection_print_rgb_b},
		{"identify_projection_print_tiff",
			(XtPointer)identify_projection_print_tiff},
		{"open_file_and_write",(XtPointer)open_file_and_write},
		{"identify_projection_close_butto",
			(XtPointer)identify_projection_close_butto},
		{"identify_projection_drawing_are",
			(XtPointer)identify_projection_drawing_are},
		{"expose_projection_drawing_area",
			(XtPointer)expose_projection_drawing_area},
		{"resize_projection_drawing_area",
			(XtPointer)resize_projection_drawing_area},
		{"identify_projection_spectrum_ar",
			(XtPointer)identify_projection_spectrum_ar},
		{"expose_projection_spectrum_area",
			(XtPointer)expose_projection_spectrum_area},
		{"resize_projection_spectrum_area",
			(XtPointer)resize_projection_spectrum_area}};
	static MrmRegisterArg identifier_list[]={
		{"projection_window_structure",NULL},
		{"write_projection_postscript_dat",NULL},
		{"write_projection_rgb_file_data",NULL},
		{"write_projection_tiff_file_data",NULL},
		{"widget_spacing",(XtPointer)5}};
			/*???DB.  How to have global uil identifiers ? */
	struct Projection_window *window;

	ENTER(create_Projection_window);
	/* check arguments */
	if (user_interface)
	{
		if (MrmOpenHierarchy_base64_string(projection_window_uidh,
			&projection_window_hierarchy,&projection_window_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(window,struct Projection_window,1))
			{
				/* assign fields */
				window->address=address;
				window->creator=creator;
				window->shell=shell;
				window->open=0;
				window->projection=(struct Projection *)NULL;
				window->window=(Widget)NULL;
				window->configure_button=(Widget)NULL;
				window->configure_dialog=(struct Projection_dialog *)NULL;
				window->print_button=(Widget)NULL;
				window->print_menu.postscript_button=(Widget)NULL;
				window->print_menu.rgb_button=(Widget)NULL;
				window->print_menu.tiff_button=(Widget)NULL;
				window->close_button=(Widget)NULL;
				window->projection_drawing_area=(Widget)NULL;
				window->projection_drawing=(struct Drawing_2d *)NULL;
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
				window->projection_drawing=(struct Projection_drawing *)NULL;
#endif /* defined (OLD_CODE) */
				window->spectrum_drawing_area=(Widget)NULL;
				window->spectrum_drawing=(struct Drawing_2d *)NULL;
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
				window->spectrum_drawing=(struct Projection_drawing *)NULL;
#endif /* defined (OLD_CODE) */
				window->animation_front= -1;
				window->user_interface=user_interface;
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(projection_window_hierarchy,
					callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)window;
					identifier_list[1].value=(XtPointer)create_File_open_data(".ps",
						REGULAR,write_projection_postscript_fil,(void *)window,0,
						user_interface);
					identifier_list[2].value=(XtPointer)create_File_open_data(".rgb",
						REGULAR,write_projection_rgb_file,(void *)window,0,user_interface);
					identifier_list[3].value=(XtPointer)create_File_open_data(".tif",
						REGULAR,write_projection_tiff_file,(void *)window,0,user_interface);
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						projection_window_hierarchy,identifier_list,
						XtNumber(identifier_list)))
					{
						/* fetch projection window widget */
						if (MrmSUCCESS==MrmFetchWidget(projection_window_hierarchy,
							"projection_window",shell,&(window->window),
							&projection_window_class))
						{
							/*??? more to do ? */
							if (address)
							{
								*address=window;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_Projection_window.  Could not fetch projection window");
							DEALLOCATE(window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Projection_window.  Could not register identifiers");
						DEALLOCATE(window);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Projection_window.  Could not register callbacks");
					DEALLOCATE(window);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
	"create_Projection_window.  Could not allocate projection window structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Projection_window.  Could not open hierarchy");
			window=(struct Projection_window *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Projection_window.  Missing user_interface");
		window=(struct Projection_window *)NULL;
	}
	LEAVE;

	return (window);
} /* create_Projection_window */

static int set_Projection_type_cylindrical(struct Parse_state *state,
	void *projection_type_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 25 August 1996

DESCRIPTION :
Sets the projection type to cylindrical.
==============================================================================*/
{
	char *current_token;
	enum Projection_type *projection_type;
	int return_code;

	ENTER(set_Projection_type_cylindrical);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (!(current_token=state->current_token)||
			(strcmp(PARSER_HELP_STRING,current_token)&&
			strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
		{
			if (projection_type=(enum Projection_type *)projection_type_void)
			{
				*projection_type=CYLINDRICAL_PROJECTION;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_Projection_type_cylindrical.  Invalid argument(s)");
				return_code=0;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Projection_type_cylindrical.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Projection_type_cylindrical */

static int set_Projection_type_hammer(struct Parse_state *state,
	void *projection_type_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 25 August 1996

DESCRIPTION :
Sets the projection type to hammer.
==============================================================================*/
{
	char *current_token;
	enum Projection_type *projection_type;
	int return_code;

	ENTER(set_Projection_type_hammer);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (!(current_token=state->current_token)||
			(strcmp(PARSER_HELP_STRING,current_token)&&
			strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
		{
			if (projection_type=(enum Projection_type *)projection_type_void)
			{
				*projection_type=HAMMER_PROJECTION;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_Projection_type_hammer.  Invalid argument(s)");
				return_code=0;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Projection_type_hammer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Projection_type_hammer */

static int set_Projection_type_polar(struct Parse_state *state,
	void *projection_type_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 25 August 1996

DESCRIPTION :
Sets the projection type to polar.
==============================================================================*/
{
	char *current_token;
	enum Projection_type *projection_type;
	int return_code;

	ENTER(set_Projection_type_polar);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (!(current_token=state->current_token)||
			(strcmp(PARSER_HELP_STRING,current_token)&&
			strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
		{
			if (projection_type=(enum Projection_type *)projection_type_void)
			{
				*projection_type=POLAR_PROJECTION;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_Projection_type_polar.  Invalid argument(s)");
				return_code=0;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Projection_type_polar.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Projection_type_polar */

static int set_Projection_type(struct Parse_state *state,
	void *projection_type_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 18 January 1997

DESCRIPTION :
Used in command parsing to translate a projection type name into an projection
type.
???DB.  Should it be here ?
==============================================================================*/
{
	char *current_token;
	enum Projection_type *projection_type;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"cylindrical",NULL,NULL,set_Projection_type_cylindrical},
		{"hammer",NULL,NULL,set_Projection_type_hammer},
		{"polar",NULL,NULL,set_Projection_type_polar},
		{NULL,NULL,NULL,NULL}
	};
	struct Modifier_entry *entry;

	ENTER(set_Projection_type);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (projection_type_void)
				{
					(option_table[0]).to_be_modified=projection_type_void;
					(option_table[1]).to_be_modified=projection_type_void;
					(option_table[2]).to_be_modified=projection_type_void;
					return_code=process_option(state,option_table);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Projection_type.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				/*???DB.  Can this be included inside process_option some how ? */
				entry=option_table;
				if (entry->option)
				{
					display_message(INFORMATION_MESSAGE," %s",entry->option);
					entry++;
					while (entry->option)
					{
						display_message(INFORMATION_MESSAGE,"|%s",entry->option);
						entry++;
					}
				}
#if defined (OLD_CODE)
				return_code=process_option(state,option_table);
#endif /* defined (OLD_CODE) */
				if (projection_type=(enum Projection_type *)projection_type_void)
				{
					switch (*projection_type)
					{
						case CYLINDRICAL_PROJECTION:
						{
							display_message(INFORMATION_MESSAGE,"[cylindrical]");
						} break;
						case HAMMER_PROJECTION:
						{
							display_message(INFORMATION_MESSAGE,"[hammer]");
						} break;
						case POLAR_PROJECTION:
						{
							display_message(INFORMATION_MESSAGE,"[polar]");
						} break;
					}
				}
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing projection type");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Projection_type.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Projection_type */

/*
Global functions
----------------
*/
int open_projection_window(struct Parse_state *state,
	void *projection_window_address_void,void *open_projection_window_data_void)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
If the projection window does not exist it is created.  The projection window is
then opened with the specified <modifications>.
==============================================================================*/
{
	Colormap colour_map;
	enum Projection_type projection_type;
	FE_value xi_3;
	int element_line_discretization,return_code,seed_point_discretization;
	static struct Modifier_entry option_table[]=
	{
		{"anatomical",NULL,NULL,set_FE_field},
		{"coordinate",NULL,NULL,set_FE_field},
		{"element_line_discretization",NULL,NULL,set_int},
		{"group",NULL,NULL,set_FE_element_group},
		{"scalar",NULL,NULL,set_FE_field_component},
		{"seed_point_discretization",NULL,NULL,set_int},
		{"spectrum",NULL,NULL,set_Spectrum},
		{"type",NULL,NULL,set_Projection_type},
		{"xi_3",NULL,NULL,set_float_0_to_1_inclusive},
		{NULL,NULL,NULL,NULL}
	};
	struct FE_field *coordinate_field,*fibre_field;
	struct FE_field_component component;
	struct GROUP(FE_element) *element_group;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
	struct Open_projection_window_data *open_projection_window_data;
	struct Projection *projection;
	struct Projection_window *projection_window,**projection_window_address;
	struct Spectrum *spectrum;
	struct User_interface *user_interface;
	Widget shell;

	ENTER(open_projection_window);
	/* check arguments */
	if ((open_projection_window_data=
		(struct Open_projection_window_data *)open_projection_window_data_void)&&
		(element_group_manager=open_projection_window_data->element_group_manager)&&
		(projection_window_address=
		(struct Projection_window **)projection_window_address_void))
	{
		element_group=(struct GROUP(FE_element) *)NULL;
		coordinate_field=(struct FE_field *)NULL;
		fibre_field=(struct FE_field *)NULL;
		projection_type=HAMMER_PROJECTION;
		component.field=(struct FE_field *)NULL;
		component.number= -1;
		element_line_discretization=10;
		seed_point_discretization=3;
		spectrum=ACCESS(Spectrum)(open_projection_window_data->default_spectrum);
		xi_3=1;
		/* check the arguments */
		if (state)
		{
			(option_table[0]).to_be_modified= &fibre_field;
			(option_table[0]).user_data=open_projection_window_data->fe_field_manager;
			(option_table[1]).to_be_modified= &coordinate_field;
			(option_table[1]).user_data=open_projection_window_data->fe_field_manager;
			(option_table[2]).to_be_modified=(void *)(&element_line_discretization);
			(option_table[3]).to_be_modified=(void *)(&element_group);
			(option_table[3]).user_data=element_group_manager;
			(option_table[4]).to_be_modified=(void *)(&component);
			(option_table[4]).user_data=open_projection_window_data->fe_field_manager;
			(option_table[5]).to_be_modified=(void *)(&seed_point_discretization);
			(option_table[6]).to_be_modified=(void *)(&spectrum);
			(option_table[6]).user_data=open_projection_window_data->spectrum_manager;
			(option_table[7]).to_be_modified=(void *)(&projection_type);
			(option_table[8]).to_be_modified=(void *)(&xi_3);
			return_code=process_multiple_options(state,option_table);
		}
		else
		{
			return_code=1;
		}
		if (return_code)
		{
			if ((user_interface=open_projection_window_data->user_interface)
				&&(projection_window= *projection_window_address))
			{
				/* update the projection settings */
				if (projection=projection_window->projection)
				{
					DEACCESS(FE_field)(&(projection->coordinate_field));
					if (coordinate_field)
					{
						projection->coordinate_field=ACCESS(FE_field)(coordinate_field);
					}
					else
					{
						projection->coordinate_field=(struct FE_field *)NULL;
					}
					DEACCESS(FE_field)(&(projection->fibre_field));
					if (fibre_field)
					{
						projection->fibre_field=ACCESS(FE_field)(fibre_field);
					}
					else
					{
						projection->fibre_field=(struct FE_field *)NULL;
					}
					DEACCESS(FE_field)(&(projection->field));
					if (component.field)
					{
						projection->field=ACCESS(FE_field)(component.field);
						projection->field_component=component.number;
					}
					else
					{
						projection->field=(struct FE_field *)NULL;
						projection->field_component=0;
					}
					DEACCESS(GROUP(FE_element))(&(projection->element_group));
					if (element_group)
					{
						projection->element_group=
							ACCESS(GROUP(FE_element))(element_group);
					}
					else
					{
						projection->element_group=(struct GROUP(FE_element) *)NULL;
					}
					projection->type=projection_type;
					projection->xi_3=xi_3;
					projection->spectrum=spectrum;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"open_projection_window.  Missing projection");
					return_code=0;
				}
			}
			else
			{
				if (colour_map=XCreateColormap(user_interface->display,
					XDefaultRootWindow(user_interface->display),
					XDefaultVisual(user_interface->display,
					XDefaultScreen(user_interface->display)),AllocNone))
				{
					/* create the projection window shell */
					if (shell=XtVaCreatePopupShell(
						"projection_window_shell",topLevelShellWidgetClass,
						user_interface->application_shell,
						XtNcolormap,colour_map,
						NULL))
					{
						/* create the projection window */
						if (projection_window=create_Projection_window(
							projection_window_address,(Widget)NULL,shell,user_interface))
						{
							/* create the projection */
							if (projection=CREATE(Projection)(projection_window->window,
								coordinate_field,fibre_field,component.field,component.number,
								element_group,xi_3,HIDE_COLOUR,HIDE_CONTOURS,HIDE_NODES,
								SHOW_ELEMENT_NAMES_AND_BOUNDARIES,HIDE_FIBRES,HIDE_LANDMARKS,
								HIDE_EXTREMA,0,1,projection_type,VARIABLE_THICKNESS,spectrum,
								seed_point_discretization,element_line_discretization,
								open_projection_window_data->element_manager,user_interface))
							{
								projection_window->projection=projection;
								/* set the height and background colour of the interval
									drawing area */
								XtVaSetValues(projection_window->spectrum_drawing_area,
/*									XmNheight,screen_height/16,*/
									XmNbackground,(projection->pixel).background_colour,
									NULL);
								/* set the background colour of the map drawing area */
								XtVaSetValues(projection_window->projection_drawing_area,
									XmNbackground,(projection->pixel).background_colour,
									NULL);
								/* manage the projection window */
								XtManageChild(projection_window->window);
								/* realize the projection window shell */
								XtRealizeWidget(shell);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"open_projection_window.  Could not create projection window");
							XtDestroyWidget(shell);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
					"open_projection_window.  Could not create projection window shell");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
					 "open_projection_window.  Could not create colour map");
					return_code=0;
				}
			}
			if (return_code)
			{
				/* check if the projection window shell has been popped up */
				if (!(projection_window->open))
				{
					projection_window->open=1;
					/* pop up the projection window shell */
					XtPopup(projection_window->shell,XtGrabNone);
					/* ghost the creator widget */
					if (projection_window->creator)
					{
						XtSetSensitive(projection_window->creator,False);
					}
				}
				/* update the projection window */
				update_projection_drawing_area(projection_window,2);
				update_projection_spectrum(projection_window);
			}
		}
		DEACCESS(Spectrum)(&spectrum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"open_projection_window.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* open_projection_window */

int update_projection_drawing_area(struct Projection_window *window,
	int recalculate)
/*******************************************************************************
LAST MODIFIED : 24 May 1997

DESCRIPTION :
This function for redrawing the <mapping> drawing area.  If <recalculate> is >0
then the colours for the pixels are recalculated.  If <recalculate> is >1 then
the interpolation functions are also recalculated.
==============================================================================*/
{
	int return_code;

	ENTER(update_projection_drawing_area);
	if (window&&(window->projection)&&(window->projection_drawing)&&
		(window->projection_drawing_area))
	{
		draw_projection(window->projection,recalculate,window->projection_drawing);
		XCopyArea(window->user_interface->display,
			window->projection_drawing->pixel_map,
			XtWindow(window->projection_drawing_area),
			(window->projection->graphics_context).copy,0,0,
			window->projection_drawing->width,window->projection_drawing->height,0,0);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* update_projection_drawing_area */

int update_projection_spectrum(struct Projection_window *window)
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
The callback for redrawing the colour bar or auxiliary devices drawing area.
==============================================================================*/
{
	int return_code;

	ENTER(update_projection_spectrum);
	if (window&&(window->projection)&&(window->spectrum_drawing)&&
		(window->spectrum_drawing_area))
	{
		draw_spectrum_area(window->projection,window->spectrum_drawing);
		XCopyArea(window->user_interface->display,
			window->spectrum_drawing->pixel_map,
			XtWindow(window->spectrum_drawing_area),
			(window->projection->graphics_context).copy,0,0,
			window->spectrum_drawing->width,window->spectrum_drawing->height,0,0);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* update_projection_spectrum */
