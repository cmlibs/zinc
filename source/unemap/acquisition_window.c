/*******************************************************************************
FILE : acquisition_window.c

LAST MODIFIED : 24 November 1999

DESCRIPTION :
==============================================================================*/
#include <stddef.h>
#if defined (MOTIF)
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Composite.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/ToggleBG.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "unemap/acquisition.h"
#include "unemap/acquisition_window.h"
#include "unemap/acquisition_window.uidh"
#include "unemap/drawing_2d.h"
#include "unemap/mapping_window.h"
#include "user_interface/filedir.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#if defined (WINDOWS)
#include "unemap/vunemapd.h"
#endif /* defined (WINDOWS) */

#if defined (WINDOWS)
#define USE_DIALOG
#endif /* defined (WINDOWS) */

/*
Module types
------------
*/
typedef struct Acquisition_window Acquisition_window_settings;

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int acquisition_window_hierarchy_open=0;
static MrmHierarchy acquisition_window_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
#if defined (MOTIF)
static void identify_acquisition_acquire_bu(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the acquisition acquire button.
==============================================================================*/
{
	struct Acquisition_window *acquisition;

	ENTER(identify_acquisition_acquire_bu);
	USE_PARAMETER(call_data);
	if (acquisition=(struct Acquisition_window *)client_data)
	{
		acquisition->acquire_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_acquisition_acquire_bu.  client_data missing");
	}
	LEAVE;
} /* identify_acquisition_acquire_bu */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_acquisition_monitoring(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the acquisition monitoring toggle.
==============================================================================*/
{
	struct Acquisition_window *acquisition;

	ENTER(identify_acquisition_monitoring);
	USE_PARAMETER(call_data);
	if (acquisition=(struct Acquisition_window *)client_data)
	{
		acquisition->monitoring_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_acquisition_monitoring.  client_data missing");
	}
	LEAVE;
} /* identify_acquisition_monitoring */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_acquisition_experiment(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the acquisition experiment toggle.
==============================================================================*/
{
	struct Acquisition_window *acquisition;

	ENTER(identify_acquisition_experiment);
	USE_PARAMETER(call_data);
	if (acquisition=(struct Acquisition_window *)client_data)
	{
		acquisition->experiment_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_acquisition_experiment.  client_data missing");
	}
	LEAVE;
} /* identify_acquisition_experiment */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_acquisition_calibrate(Widget *widget_id,
	XtPointer acquisition_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the acquisition calibrate button.
==============================================================================*/
{
	struct Acquisition_window *acquisition;

	ENTER(identify_acquisition_calibrate);
	USE_PARAMETER(call_data);
	if (acquisition=(struct Acquisition_window *)acquisition_window)
	{
		acquisition->calibrate_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_acquisition_calibrate.  acquisition_window missing");
	}
	LEAVE;
} /* identify_acquisition_calibrate */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_acquisition_page_butto(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the acquisition page button.
==============================================================================*/
{
	struct Acquisition_window *acquisition;

	ENTER(identify_acquisition_page_butto);
	USE_PARAMETER(call_data);
	if (acquisition=(struct Acquisition_window *)client_data)
	{
		acquisition->page_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_acquisition_page_butto.  client_data missing");
	}
	LEAVE;
} /* identify_acquisition_page_butto */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_acquisition_close_butt(Widget *widget_id,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the acquisition close button.
==============================================================================*/
{
	struct Acquisition_window *acquisition;

	ENTER(identify_acquisition_close_butt);
	USE_PARAMETER(call_data);
	if (acquisition=(struct Acquisition_window *)client_data)
	{
		acquisition->close_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_acquisition_close_butt.  client_data missing");
	}
	LEAVE;
} /* identify_acquisition_close_butt */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void identify_acquisition_drawing_ar(Widget *widget_id,
	XtPointer acquisition_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the acquisition drawing area.
==============================================================================*/
{
	struct Acquisition_window *acquisition;

	ENTER(identify_acquisition_close_butt);
	USE_PARAMETER(call_data);
	if (acquisition=(struct Acquisition_window *)acquisition_window)
	{
		acquisition->drawing_area= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_acquisition_drawing_ar.  acquisition_window missing");
	}
	LEAVE;
} /* identify_acquisition_drawing_ar */
#endif /* defined (MOTIF) */

static int draw_acquisition_interval(int start_time,int acquire_time,
	int end_time,float frequency,
#if defined (MOTIF)
	Drawable drawable,
#endif /* defined (MOTIF) */
	int width,int height,int *start_point,int *acquire_point,int *end_point,
	int *marker_bottom,int *marker_centre,int *marker_top,
	struct Acquisition_window *acquisition)
/*******************************************************************************
LAST MODIFIED : 26 March 1997

DESCRIPTION :
Draws the line segment that represents the current acquisition interval.
???DB.  Just pass acquisition ?
==============================================================================*/
{
	char number_string[7];
#if defined (MOTIF)
	Display *display;
#endif /* defined (MOTIF) */
	float time,x_scale;
#if defined (MOTIF)
	int ascent,descent,direction,text_x,text_y;
#endif /* defined (MOTIF) */
	int maximum_interval_length,return_code,string_length,widget_spacing;
#if defined (MOTIF)
	XCharStruct bounds;
	XFontStruct *font;
#endif /* defined (MOTIF) */

	ENTER(draw_acquisition_interval);
	if ((start_time<=acquire_time)&&(acquire_time<=end_time)&&
#if defined (MOTIF)
		drawable&&
#endif /* defined (MOTIF) */
		start_point&&acquire_point&&end_point&&marker_bottom&&marker_centre&&
		marker_top&&acquisition)
	{
#if defined (MOTIF)
		display=acquisition->user_interface->display;
		widget_spacing=acquisition->user_interface->widget_spacing;
		font=acquisition->user_interface->normal_font;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
		widget_spacing=5;
			/*???DB.  Pass widget_spacing ? */
#endif /* defined (WINDOWS) */
		/* clear the drawing */
#if defined (MOTIF)
		XFillRectangle(display,drawable,
			(acquisition->graphics_context).background_drawing_colour,
			0,0,width,height);
#endif /* defined (MOTIF) */
		/* check that there is enough room */
		if (((maximum_interval_length=width-2*widget_spacing-1)>0)&&
			(2*widget_spacing<height))
		{
			/* convert the times into positions */
			x_scale=SCALE_FACTOR(MAXIMUM_NUMBER_OF_SAMPLES,maximum_interval_length);
			*start_point=SCALE_X(start_time,0,widget_spacing,x_scale);
			*acquire_point=SCALE_X(acquire_time,0,widget_spacing,x_scale);
			*end_point=SCALE_X(end_time,0,widget_spacing,x_scale);
			/* write the acquire time */
			time=(float)(acquire_time-start_time)/frequency;
			sprintf(number_string,"%.3f",time);
			string_length=strlen(number_string);
#if defined (MOTIF)
			XTextExtents(font,number_string,string_length,&direction,&ascent,&descent,
				&bounds);
			text_x= *acquire_point-(bounds.rbearing+bounds.lbearing)/2;
			if (text_x+bounds.rbearing>width-widget_spacing)
			{
				text_x=width-widget_spacing-bounds.rbearing;
			}
			if (text_x+bounds.lbearing<widget_spacing)
			{
				text_x=widget_spacing-bounds.lbearing;
			}
			text_y=height-widget_spacing-descent;
			*marker_bottom=text_y-ascent-widget_spacing;
			XDrawString(display,drawable,
				(acquisition->graphics_context).acquisition_interval_colour_tex,text_x,
				text_y,number_string,string_length);
#endif /* defined (MOTIF) */
			/* write the end time */
			time=(float)(end_time-start_time)/frequency;
			sprintf(number_string,"%.3f",time);
			string_length=strlen(number_string);
#if defined (MOTIF)
			XTextExtents(font,number_string,string_length,&direction,&ascent,&descent,
				&bounds);
			text_x= *end_point-(bounds.rbearing+bounds.lbearing)/2;
			if (text_x+bounds.rbearing>width-widget_spacing)
			{
				text_x=width-widget_spacing-bounds.rbearing;
			}
			if (text_x+bounds.lbearing<widget_spacing)
			{
				text_x=widget_spacing-bounds.lbearing;
			}
			text_y=widget_spacing+ascent;
			*marker_top=text_y+descent+widget_spacing;
			XDrawString(display,drawable,
				(acquisition->graphics_context).acquisition_interval_colour_tex,text_x,
				text_y,number_string,string_length);
#endif /* defined (MOTIF) */
			/* draw the line segment */
			*marker_centre=(*marker_top+*marker_bottom)/2;
#if defined (MOTIF)
			XDrawLine(display,drawable,
				(acquisition->graphics_context).acquisition_interval_colour,
				*start_point,*marker_centre,*end_point,*marker_centre);
			/* draw the markers */
			XDrawLine(display,drawable,
				(acquisition->graphics_context).acquisition_interval_colour,
				*start_point,*marker_top,*start_point,(*marker_centre)-1);
			XDrawLine(display,drawable,
				(acquisition->graphics_context).acquisition_interval_colour,
				*acquire_point,(*marker_centre)+1,*acquire_point,*marker_bottom);
			XDrawLine(display,drawable,
				(acquisition->graphics_context).acquisition_interval_colour,
				*end_point,*marker_top,*end_point,(*marker_centre)-1);
#endif /* defined (MOTIF) */
		}
		else
		{
			*start_point= -1;
			*acquire_point= -1;
			*end_point= -1;
			*marker_bottom= -1;
			*marker_centre= -1;
			*marker_top= -1;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_acquisition_interval.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_acquisition_interval */

#if defined (MOTIF)
static void expose_acquisition_drawing_area(Widget widget,
	XtPointer acquisition_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
The callback for redrawing part of an acquisition drawing area.
==============================================================================*/
{
	struct Acquisition_window *acquisition;
	XmDrawingAreaCallbackStruct *callback;
	XExposeEvent *event;
	XWindowAttributes attributes;
	float frequency;

	ENTER(expose_acquisition_drawing_area);
	USE_PARAMETER(widget);
	if (acquisition=(struct Acquisition_window *)acquisition_window)
	{
		frequency=(float)(acquisition->sampling_frequency_hz);
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_EXPOSE==callback->reason)
			{
				if (callback->event)
				{
					if (Expose==callback->event->type)
					{
						event= &(callback->event->xexpose);
						if (acquisition->drawing_area)
						{
							if (!(acquisition->drawing))
							{
								/* determine the size of the drawing area */
								XGetWindowAttributes(acquisition->user_interface->display,
									XtWindow(acquisition->drawing_area),&attributes);
								/* create a pixel map */
									/*???not used because of problems with fonts */
								if (acquisition->drawing=
									create_Drawing_2d(acquisition->drawing_area,attributes.width,
									attributes.height,NO_DRAWING_IMAGE,
									acquisition->user_interface))
								{
									/* draw the acquisition interval */
									draw_acquisition_interval(acquisition->start_time,
										acquisition->acquire_time,acquisition->end_time,frequency,
										acquisition->drawing->pixel_map,
										acquisition->drawing->width,acquisition->drawing->height,
										&(acquisition->start_point),&(acquisition->acquire_point),
										&(acquisition->end_point),&(acquisition->marker_bottom),
										&(acquisition->marker_centre),&(acquisition->marker_top),
										acquisition);
								}
								else
								{
									display_message(ERROR_MESSAGE,
									"expose_acquisition_drawing_area.  Could not create drawing");
								}
							}
							/* redisplay the specified part of the pixmap */
							if (acquisition->drawing)
							{
								XCopyArea(acquisition->user_interface->display,
									acquisition->drawing->pixel_map,
									XtWindow(acquisition->drawing_area),
									(acquisition->graphics_context).copy,event->x,event->y,
									event->width,event->height,event->x,event->y);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"expose_acquisition_drawing_area.  Missing drawing area");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"expose_acquisition_drawing_area.  Incorrect event reason");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"expose_acquisition_drawing_area.  Missing event");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"expose_acquisition_drawing_area.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"expose_acquisition_drawing_area.  Missing call_data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"expose_acquisition_drawing_area.  Missing acquisition_window");
	}
	LEAVE;
} /* expose_acquisition_drawing_area */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void resize_acquisition_drawing_area(Widget widget,
	XtPointer acquisition_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
The callback for resizing a acquisition drawing area.
==============================================================================*/
{
	struct Acquisition_window *acquisition;
	XmDrawingAreaCallbackStruct *callback;
	XWindowAttributes attributes;
	int width,height;
	float frequency;

	ENTER(resize_acquisition_drawing_area);
	USE_PARAMETER(widget);
	if (acquisition=(struct Acquisition_window *)acquisition_window)
	{
		frequency=(float)(acquisition->sampling_frequency_hz);
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_RESIZE==callback->reason)
			{
				/*??? during creation there are resize callbacks without windows */
				if (callback->window)
				{
					if (acquisition->drawing_area)
					{
						/* find the size of the old rectangle */
						if (acquisition->drawing)
						{
							width=acquisition->drawing->width;
							height=acquisition->drawing->height;
							/* destroy the old pixmap */
							destroy_Drawing_2d(&(acquisition->drawing));
						}
						else
						{
							width=0;
							height=0;
						}
						/* find the size of the new rectangle */
						XGetWindowAttributes(acquisition->user_interface->display,
							callback->window,&attributes);
						/* create a new pixmap */
						if (acquisition->drawing=create_Drawing_2d(
							acquisition->drawing_area,attributes.width,attributes.height,
							NO_DRAWING_IMAGE,acquisition->user_interface))
						{
							/* redraw the acquisition interval */
							draw_acquisition_interval(acquisition->start_time,
								acquisition->acquire_time,acquisition->end_time,frequency,
								acquisition->drawing->pixel_map,acquisition->drawing->width,
								acquisition->drawing->height,&(acquisition->start_point),
								&(acquisition->acquire_point),&(acquisition->end_point),
								&(acquisition->marker_bottom),&(acquisition->marker_centre),
								&(acquisition->marker_top),acquisition);
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
							XCopyArea(acquisition->user_interface->display,
								acquisition->drawing->pixel_map,
								XtWindow(acquisition->drawing_area),
								(acquisition->graphics_context).copy,0,0,width,height,0,0);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"resize_acquisition_drawing_area.  Missing drawing area");
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"resize_acquisition_drawing_area.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"resize_acquisition_drawing_area.  call_data missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"resize_acquisition_drawing_area.  acquisition window missing");
	}
	LEAVE;
} /* resize_acquisition_drawing_area */
#endif /* defined (MOTIF) */

enum Moving_status
{
	MOVING_NONE,
	MOVING_START,
	MOVING_ACQUIRE,
	MOVING_END
};

#if defined (MOTIF)
static void select_acquisition_drawing_area(Widget widget,
	XtPointer acquisition_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
The callback for modifying the acquisition interval in the acquisition drawing
area.
==============================================================================*/
{
	Boolean owner_events;
	char number_string[7];
	Cursor cursor;
	Display *display;
	enum Moving_status moving;
	float frequency,time;
	GC acquisition_interval_colour,acquisition_interval_colour_tex;
	int acquire,ascent,axes_left,axes_right,descent,direction,end,height,
		keyboard_mode,pointer_mode,pointer_sensitivity,pointer_x,pointer_y,
		previous_acquire,previous_end,string_length,text_x,text_y,widget_spacing,
		width;
	Pixmap pixel_map;
	struct Acquisition_window *acquisition;
	struct Drawing_2d *drawing;
	unsigned int working_button;
	Window confine_to,working_window;
	XButtonEvent *button_event;
	XCharStruct bounds;
	XEvent xevent;
	XmDrawingAreaCallbackStruct *callback;
	XFontStruct *font;

	ENTER(select_acquisition_drawing_area);
	USE_PARAMETER(widget);
	if ((acquisition=(struct Acquisition_window *)acquisition_window)&&
		(drawing=acquisition->drawing))
	{
		frequency=(float)(acquisition->sampling_frequency_hz);
		display=acquisition->user_interface->display;
		widget_spacing=acquisition->user_interface->widget_spacing;
		font=acquisition->user_interface->normal_font;
		acquisition_interval_colour=
			(acquisition->graphics_context).acquisition_interval_colour;
		acquisition_interval_colour_tex=
			(acquisition->graphics_context).acquisition_interval_colour_tex;
		if (callback=(XmDrawingAreaCallbackStruct *)call_data)
		{
			if (XmCR_INPUT==callback->reason)
			{
				if ((callback->event)&&(ButtonPress==callback->event->type))
				{
					button_event= &(callback->event->xbutton);
					pointer_x=button_event->x;
					pointer_y=button_event->y;
					working_button=button_event->button;
					pointer_sensitivity=acquisition->pointer_sensitivity;
					if (((end=acquisition->end_point)>=0)&&
						(pointer_x>=end-pointer_sensitivity)&&
						(pointer_x<=end+pointer_sensitivity)&&
						(pointer_y>=acquisition->marker_top-pointer_sensitivity)&&
						(pointer_y<=acquisition->marker_centre+pointer_sensitivity))
					{
						moving=MOVING_END;
						XWarpPointer(display,None,None,0,0,0,0,end-pointer_x,0);
						pointer_x=end;
						acquire=acquisition->acquire_point;
					}
					else
					{
						if (((acquire=acquisition->acquire_point)>=0)&&
							(pointer_x>=acquire-pointer_sensitivity)&&
							(pointer_x<=acquire+pointer_sensitivity)&&
							(pointer_y<=acquisition->marker_bottom+pointer_sensitivity)&&
							(pointer_y>=acquisition->marker_centre-pointer_sensitivity))
						{
							moving=MOVING_ACQUIRE;
							XWarpPointer(display,None,None,0,0,0,0,acquire-pointer_x,0);
							pointer_x=acquire;
							end=acquisition->end_point;
						}
						else
						{
							moving=MOVING_NONE;
						}
					}
					if (MOVING_NONE!=moving)
					{
						cursor=XCreateFontCursor(display,XC_sb_h_double_arrow);
						/* grab the pointer */
						owner_events=True;
						pointer_mode=GrabModeAsync;
						keyboard_mode=GrabModeAsync;
						confine_to=None;
						if (GrabSuccess==XtGrabPointer(acquisition->drawing_area,
							owner_events,ButtonMotionMask|ButtonPressMask|ButtonReleaseMask,
							pointer_mode,keyboard_mode,confine_to,cursor,CurrentTime))
						{
							width=drawing->width;
							height=drawing->height;
							axes_left=widget_spacing;
							axes_right=width-widget_spacing;
							working_window=XtWindow(acquisition->drawing_area);
							pixel_map=acquisition->drawing->pixel_map;
							/* clear the time for the acquire point */
							time=(float)(acquisition->acquire_time-acquisition->start_time)/
								frequency;
							sprintf(number_string,"%.3f",time);
							string_length=strlen(number_string);
							XTextExtents(font,number_string,string_length,&direction,&ascent,
								&descent,&bounds);
							text_x=acquire-(bounds.rbearing+bounds.lbearing)/2;
							if (text_x+bounds.rbearing>width-widget_spacing)
							{
								text_x=width-widget_spacing-bounds.rbearing;
							}
							if (text_x+bounds.lbearing<widget_spacing)
							{
								text_x=widget_spacing-bounds.lbearing;
							}
							text_y=height-widget_spacing-descent;
							XDrawString(display,pixel_map,acquisition_interval_colour_tex,
								text_x,text_y,number_string,string_length);
							XDrawString(display,working_window,acquisition_interval_colour,
								text_x,text_y,number_string,string_length);
							/* clear the time for the end point */
							time=(float)(acquisition->end_time-acquisition->start_time)/
								frequency;
							sprintf(number_string,"%.3f",time);
							string_length=strlen(number_string);
							XTextExtents(font,number_string,string_length,&direction,&ascent,
								&descent,&bounds);
							text_x=end-(bounds.rbearing+bounds.lbearing)/2;
							if (text_x+bounds.rbearing>width-widget_spacing)
							{
								text_x=width-widget_spacing-bounds.rbearing;
							}
							if (text_x+bounds.lbearing<widget_spacing)
							{
								text_x=widget_spacing-bounds.lbearing;
							}
							text_y=widget_spacing+ascent;
							XDrawString(display,pixel_map,acquisition_interval_colour_tex,
								text_x,text_y,number_string,string_length);
							XDrawString(display,working_window,acquisition_interval_colour,
								text_x,text_y,number_string,string_length);
							while (moving!=MOVING_NONE)
							{
								XNextEvent(display,&xevent);
								switch (xevent.type)
								{
									case MotionNotify:
									{
										previous_acquire=acquire;
										previous_end=end;
										/* reduce the number of motion events displayed */
										while (XCheckMaskEvent(display,ButtonMotionMask,&xevent));
										pointer_x=xevent.xmotion.x;
										pointer_y=xevent.xmotion.y;
										if (xevent.xmotion.window==working_window)
										{
											switch (moving)
											{
												case MOVING_END:
												{
													if (pointer_x>axes_right)
													{
														end=axes_right;
													}
													else
													{
														if (pointer_x<axes_left)
														{
															end=axes_left;
														}
														else
														{
															end=pointer_x;
														}
													}
													if (end<acquire)
													{
														acquire=end;
													}
												} break;
												case MOVING_ACQUIRE:
												{
													if (pointer_x>axes_right)
													{
														acquire=axes_right;
													}
													else
													{
														if (pointer_x<axes_left)
														{
															acquire=axes_left;
														}
														else
														{
															acquire=pointer_x;
														}
													}
													if (end<acquire)
													{
														end=acquire;
													}
												} break;
											}
											if (acquire!=previous_acquire)
											{
												/* clear the previous acquire marker */
												XDrawLine(display,pixel_map,acquisition_interval_colour,
													previous_acquire,(acquisition->marker_centre)+1,
													previous_acquire,acquisition->marker_bottom);
												XDrawLine(display,working_window,
													acquisition_interval_colour,
													previous_acquire,(acquisition->marker_centre)+1,
													previous_acquire,acquisition->marker_bottom);
												/* draw the new acquire marker */
												XDrawLine(display,pixel_map,acquisition_interval_colour,
													acquire,(acquisition->marker_centre)+1,
													acquire,acquisition->marker_bottom);
												XDrawLine(display,working_window,
													acquisition_interval_colour,
													acquire,(acquisition->marker_centre)+1,
													acquire,acquisition->marker_bottom);
											}
											if (end!=previous_end)
											{
												/* clear the previous end marker */
												XDrawLine(display,pixel_map,acquisition_interval_colour,
													previous_end,acquisition->marker_top,
													previous_end,(acquisition->marker_centre)-1);
												XDrawLine(display,working_window,
													acquisition_interval_colour,
													previous_end,acquisition->marker_top,
													previous_end,(acquisition->marker_centre)-1);
												/* redraw the interval */
												if (end<previous_end)
												{
													XDrawLine(display,pixel_map,
														acquisition_interval_colour,
														end+1,acquisition->marker_centre,
														previous_end,acquisition->marker_centre);
													XDrawLine(display,working_window,
														acquisition_interval_colour,
														end+1,acquisition->marker_centre,
														previous_end,acquisition->marker_centre);
												}
												else
												{
													XDrawLine(display,pixel_map,
														acquisition_interval_colour,
														previous_end+1,acquisition->marker_centre,
														end,acquisition->marker_centre);
													XDrawLine(display,working_window,
														acquisition_interval_colour,
														previous_end+1,acquisition->marker_centre,
														end,acquisition->marker_centre);
												}
												/* draw the new end marker */
												XDrawLine(display,pixel_map,acquisition_interval_colour,
													end,acquisition->marker_top,
													end,(acquisition->marker_centre)-1);
												XDrawLine(display,working_window,
													acquisition_interval_colour,
													end,acquisition->marker_top,
													end,(acquisition->marker_centre)-1);
											}
										}
									} break;
									case ButtonPress:
									{
										if (xevent.xbutton.button==working_button)
										{
											display_message(ERROR_MESSAGE,
									"select_acquisition_drawing_area.  Unexpected button press");
											moving=MOVING_NONE;
										}
									} break;
									case ButtonRelease:
									{
										if (xevent.xbutton.button==working_button)
										{
											if (xevent.xbutton.window==working_window)
											{
												if (acquire!=acquisition->acquire_point)
												{
													acquisition->acquire_time=SCALE_X(acquire,
														acquisition->start_point,acquisition->start_time,
														SCALE_FACTOR(axes_right-axes_left,
														MAXIMUM_NUMBER_OF_SAMPLES));
													acquisition->acquire_point=
														SCALE_X(acquisition->acquire_time,
														acquisition->start_time,acquisition->start_point,
														SCALE_FACTOR(MAXIMUM_NUMBER_OF_SAMPLES,
														axes_right-axes_left));
													if (acquire!=acquisition->acquire_point)
													{
														previous_acquire=acquire;
														acquire=acquisition->acquire_point;
														/* clear the previous acquire marker */
														XDrawLine(display,pixel_map,
															acquisition_interval_colour,
															previous_acquire,(acquisition->marker_centre)+1,
															previous_acquire,acquisition->marker_bottom);
														XDrawLine(display,working_window,
															acquisition_interval_colour,
															previous_acquire,(acquisition->marker_centre)+1,
															previous_acquire,acquisition->marker_bottom);
														/* draw the new acquire marker */
														XDrawLine(display,pixel_map,
															acquisition_interval_colour,
															acquire,(acquisition->marker_centre)+1,
															acquire,acquisition->marker_bottom);
														XDrawLine(display,working_window,
															acquisition_interval_colour,
															acquire,(acquisition->marker_centre)+1,
															acquire,acquisition->marker_bottom);
													}
												}
												if (end!=acquisition->end_point)
												{
													acquisition->end_time=SCALE_X(end,
														acquisition->start_point,acquisition->start_time,
														SCALE_FACTOR(axes_right-axes_left,
														MAXIMUM_NUMBER_OF_SAMPLES));
													acquisition->end_point=SCALE_X(acquisition->end_time,
														acquisition->start_time,acquisition->start_point,
														SCALE_FACTOR(MAXIMUM_NUMBER_OF_SAMPLES,
														axes_right-axes_left));
													if (end!=acquisition->end_point)
													{
														previous_end=end;
														end=acquisition->end_point;
														/* clear the previous end marker */
														XDrawLine(display,pixel_map,
															acquisition_interval_colour,
															previous_end,acquisition->marker_top,
															previous_end,(acquisition->marker_centre)-1);
														XDrawLine(display,working_window,
															acquisition_interval_colour,
															previous_end,acquisition->marker_top,
															previous_end,(acquisition->marker_centre)-1);
														/* redraw the interval */
														if (end<previous_end)
														{
															XDrawLine(display,pixel_map,
																acquisition_interval_colour,
																end+1,acquisition->marker_centre,
																previous_end,acquisition->marker_centre);
															XDrawLine(display,working_window,
																acquisition_interval_colour,
																end+1,acquisition->marker_centre,
																previous_end,acquisition->marker_centre);
														}
														else
														{
															XDrawLine(display,pixel_map,
																acquisition_interval_colour,
																previous_end+1,acquisition->marker_centre,
																end,acquisition->marker_centre);
															XDrawLine(display,working_window,
																acquisition_interval_colour,
																previous_end+1,acquisition->marker_centre,
																end,acquisition->marker_centre);
														}
														/* draw the new end marker */
														XDrawLine(display,pixel_map,
															acquisition_interval_colour,
															end,acquisition->marker_top,
															end,(acquisition->marker_centre)-1);
														XDrawLine(display,working_window,
															acquisition_interval_colour,
															end,acquisition->marker_top,
															end,(acquisition->marker_centre)-1);
													}
												}
											}
											else
											{
												if (acquire!=acquisition->acquire_point)
												{
													previous_acquire=acquire;
													acquire=acquisition->acquire_point;
													/* clear the new acquire marker */
													XDrawLine(display,pixel_map,
														acquisition_interval_colour,
														previous_acquire,(acquisition->marker_centre)+1,
														previous_acquire,acquisition->marker_bottom);
													XDrawLine(display,working_window,
														acquisition_interval_colour,
														previous_acquire,(acquisition->marker_centre)+1,
														previous_acquire,acquisition->marker_bottom);
													/* draw the old acquire marker */
													XDrawLine(display,pixel_map,
														acquisition_interval_colour,
														acquire,(acquisition->marker_centre)+1,
														acquire,acquisition->marker_bottom);
													XDrawLine(display,working_window,
														acquisition_interval_colour,
														acquire,(acquisition->marker_centre)+1,
														acquire,acquisition->marker_bottom);
												}
												if (end!=acquisition->end_point)
												{
													previous_end=end;
													end=acquisition->end_point;
													/* clear the new end marker */
													XDrawLine(display,pixel_map,
														acquisition_interval_colour,
														previous_end,acquisition->marker_top,
														previous_end,(acquisition->marker_centre)-1);
													XDrawLine(display,working_window,
														acquisition_interval_colour,
														previous_end,acquisition->marker_top,
														previous_end,(acquisition->marker_centre)-1);
													/* redraw the interval */
													if (end<previous_end)
													{
														XDrawLine(display,pixel_map,
															acquisition_interval_colour,
															end+1,acquisition->marker_centre,
															previous_end,acquisition->marker_centre);
														XDrawLine(display,working_window,
															acquisition_interval_colour,
															end+1,acquisition->marker_centre,
															previous_end,acquisition->marker_centre);
													}
													else
													{
														XDrawLine(display,pixel_map,
															acquisition_interval_colour,
															previous_end+1,acquisition->marker_centre,
															end,acquisition->marker_centre);
														XDrawLine(display,working_window,
															acquisition_interval_colour,
															previous_end+1,acquisition->marker_centre,
															end,acquisition->marker_centre);
													}
													/* draw the old end marker */
													XDrawLine(display,pixel_map,
														acquisition_interval_colour,
														end,acquisition->marker_top,
														end,(acquisition->marker_centre)-1);
													XDrawLine(display,working_window,
														acquisition_interval_colour,
														end,acquisition->marker_top,
														end,(acquisition->marker_centre)-1);
												}
											}
											/* write the acquire time */
											time=(float)(acquisition->acquire_time-
												acquisition->start_time)/frequency;
											sprintf(number_string,"%.3f",time);
											string_length=strlen(number_string);
											XTextExtents(font,number_string,string_length,&direction,
												&ascent,&descent,&bounds);
											text_x=acquire-(bounds.rbearing+bounds.lbearing)/2;
											if (text_x+bounds.rbearing>width-widget_spacing)
											{
												text_x=width-widget_spacing-bounds.rbearing;
											}
											if (text_x+bounds.lbearing<widget_spacing)
											{
												text_x=widget_spacing-bounds.lbearing;
											}
											text_y=height-widget_spacing-descent;
											XDrawString(display,pixel_map,
												acquisition_interval_colour_tex,
												text_x,text_y,number_string,string_length);
											XDrawString(display,working_window,
												acquisition_interval_colour,text_x,
												text_y,number_string,string_length);
											/* write the end time */
											time=(float)(acquisition->end_time-
												acquisition->start_time)/frequency;
											sprintf(number_string,"%.3f",time);
											string_length=strlen(number_string);
											XTextExtents(font,number_string,string_length,
												&direction,&ascent,&descent,&bounds);
											text_x=end-(bounds.rbearing+bounds.lbearing)/2;
											width=drawing->width;
											if (text_x+bounds.rbearing>width-widget_spacing)
											{
												text_x=width-widget_spacing-bounds.rbearing;
											}
											if (text_x+bounds.lbearing<widget_spacing)
											{
												text_x=widget_spacing-bounds.lbearing;
											}
											text_y=widget_spacing+ascent;
											XDrawString(display,pixel_map,
												acquisition_interval_colour_tex,
												text_x,text_y,number_string,string_length);
											XDrawString(display,working_window,
												acquisition_interval_colour,
												text_x,text_y,number_string,string_length);
											moving=MOVING_NONE;
										}
									} break;
									default:
									{
										XtDispatchEvent(&xevent);
									}
								}
							}
							/* release the pointer */
							XtUngrabPointer(acquisition->drawing_area,CurrentTime);
						}
						XFreeCursor(display,cursor);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"select_acquisition_drawing_area.  Incorrect reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"select_acquisition_drawing_area.  Missing call_data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"select_acquisition_drawing_area.  Missing acquisition_window");
	}
	LEAVE;
} /* select_acquisition_drawing_area */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void destroy_Acquisition_window(Widget widget,
	XtPointer acquisition_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
If the <address> field of the <acquisition_window> is not NULL, <*address> is
set to NULL.  If the <activation> field is not NULL, the <activation> widget is
unghosted.  If there is an experiment running it is stopped.  The function
frees the memory associated with the fields of the <acquisition_window> and
frees the memory associated with the <acquisition_window>.
==============================================================================*/
{
	struct Acquisition_window *acquisition;
#if defined (POLLING)
	int number_of_devices;
	struct Buffer *buffer;
	struct Device *device;
	struct Rig *rig;
#endif /* defined (POLLING) */

	ENTER(destroy_Acquisition_window);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (acquisition=(struct Acquisition_window *)acquisition_window)
	{
		/* determine if there is an experiment running */
		if (True==XmToggleButtonGadgetGetState(acquisition->experiment_toggle))
		{
			/*???would like to call start_stop_experiment, but requires
				acquisition work area ? */
#if defined (UNIMA)
			/* reset the Unima system */
			U00SysReset();
			/* reset the Unima adapter */
			UAReset();
			UASetAdapter(DISABLE);
#endif
			if (acquisition->rig_address)
			{
#if defined (INTERRUPT)
				/* stop the interrupting */
				stop_acquisition_interrupt(&acquisition_interrupt_data);
#endif
#if defined (POLLING)
				if ((rig= *(acquisition->rig_address))&&(device=rig->devices)&&
					((number_of_devices=rig->number_of_devices)>0))
				{
					buffer= (*device)->signal->buffer;
					/* destroy signals */
					index=0;
					while (index<number_of_devices)
					{
						index++;
						destroy_Signal(&((*device)->signal));
						device++;
					}
					/* destroy the buffer */
					destroy_Signal_buffer(&buffer);
				}
#endif
			}
		}
		/* destroy the file open data structures */
		destroy_File_open_data(&(acquisition->acquire_file_open_data));
		/*???DB.  Temporary ? */
		destroy_File_open_data(&(acquisition->monitoring_file_open_data));
		destroy_File_open_data(&(acquisition->experiment_file_open_data));
		/* set the pointer to the acquisition window to NULL */
		if (acquisition->address)
		{
			*(acquisition->address)=(struct Acquisition_window *)NULL;
		}
		/* unghost the activation button */
		if (acquisition->activation)
		{
			XtSetSensitive(acquisition->activation,True);
		}
		/* free the acquisition window memory */
		DEALLOCATE(acquisition);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_Acquisition_window.  client_data missing");
	}
	LEAVE;
} /* destroy_Acquisition_window */
#endif /* defined (MOTIF) */

#if defined (WINDOWS)
static LRESULT CALLBACK Acquisition_window_class_proc(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 5 April 1997

DESCRIPTION :
???DB.  Should return 0 if it processes the message.
==============================================================================*/
{
	struct Acquisition_window *acquisition_window;
	CREATESTRUCT *wm_create_structure;
	static HBRUSH fill_brush;
	HDC device_context;
	LRESULT return_code;
	RECT drawing_rectangle;
/*???debug */
static POINT scroll_line[2],signal_line[2],x_axis[2];
static RECT fill_rectangle;

	ENTER(Acquisition_window_class_proc);
	switch (message_identifier)
	{
		case WM_CLOSE:
		{
			/* destroy fill brush */
			DeleteObject(fill_brush);
			DestroyWindow(window);
			return_code=0;
		} break;
		case WM_CREATE:
		{
			/* set the fill colour and type */
			fill_brush=CreateSolidBrush(0x00ffffff);
#if defined (USE_DIALOG)
			return_code=DefDlgProc(window,message_identifier,first_message,
				second_message);
#endif /* defined (USE_DIALOG) */
#if defined (USE_WINDOW)
			/* retrieve the acquisition window out of <window> */
			wm_create_structure=(CREATESTRUCT *)second_message;
			acquisition_window=
				(struct Acquisition_window *)(wm_create_structure->lpCreateParams);
				/*???DB.  Check for NT - see WM_CREATE */
			/* set the data */
			SetWindowLong(window,0,(LONG)acquisition_window);
			/* start the interrupting */
			start_interrupting(acquisition_window->device_driver,window,WM_USER,
				(LPARAM)acquisition_window);
			/*???DB.  Any other processing ? */
			return_code=DefWindowProc(window,message_identifier,first_message,
				second_message);
#endif /* defined (USE_WINDOW) */
		} break;
		case WM_USER:
		{
			device_context=GetDC(window);
			GetClientRect(window,&drawing_rectangle);
				/*???DB.  Seems to return a nonzero on success (not TRUE) */
			{
				FillRect(device_context,&fill_rectangle,fill_brush);
				Polyline(device_context,x_axis,2);
				Polyline(device_context,scroll_line,2);
				(fill_rectangle.left)++;
				(fill_rectangle.right)++;
				(x_axis[1].x)++;
				(scroll_line[0].x)++;
				(scroll_line[1].x)++;
				signal_line[0].x=signal_line[1].x;
				signal_line[0].y=signal_line[1].y;
				signal_line[1].x=x_axis[0].x;
				signal_line[1].y=((/*1000*/32768-(short)first_message)*
					drawing_rectangle.bottom)/ /*2000*/65536;
				if (x_axis[0].x>0)
				{
				  Polyline(device_context,signal_line,2);
				}
				(x_axis[0].x)++;
				if (x_axis[0].x>drawing_rectangle.right)
				{
					x_axis[0].x=0;
					x_axis[1].x=2;
					scroll_line[0].x=1;
					scroll_line[1].x=1;
					fill_rectangle.left=0;
					fill_rectangle.right=2;
				}
			}
			return_code=0;
		} break;
		case WM_DESTROY:
		{
#if defined (USE_WINDOW)
/*???debug */
unsigned long interrupt_count,start;

acquisition_window=(struct Acquisition_window *)GetWindowLong(window,0);
stop_interrupting(acquisition_window->device_driver,&interrupt_count,&start);
#endif /* defined (USE_WINDOW) */
			PostQuitMessage(0);
			return_code=0;
		} break;
		case WM_PAINT:
		{
			/* redraw the client area */
			device_context=GetDC(window);
			GetClientRect(window,&fill_rectangle);
			(fill_rectangle.right)++;
			(fill_rectangle.bottom)++;
				/*???DB.  Seems to return a nonzero on success (not TRUE) */
			FillRect(device_context,&fill_rectangle,fill_brush);
			x_axis[0].x=0;
			x_axis[0].y=fill_rectangle.bottom/2;
			x_axis[1].x=fill_rectangle.right;
			x_axis[1].y=x_axis[0].y;
			Polyline(device_context,x_axis,2);
			fill_rectangle.right=2;
			x_axis[1].x=2;
			scroll_line[0].x=1;
			scroll_line[0].y=0;
			scroll_line[1].x=1;
			scroll_line[1].y=fill_rectangle.bottom;
			return_code=0;
		}
		default:
		{
#if defined (USE_DIALOG)
			return_code=DefDlgProc(window,message_identifier,first_message,
				second_message);
#endif /* defined (USE_DIALOG) */
#if defined (USE_WINDOW)
			return_code=DefWindowProc(window,message_identifier,first_message,
				second_message);
#endif /* defined (USE_WINDOW) */
		} break;
	}
	LEAVE;

	return (return_code);
} /* Acquisition_window_class_proc */
#endif /* defined (WINDOWS) */

#if defined (WINDOWS)
#if defined (USE_DIALOG)
static void Acquisition_window_WM_COMMAND_handler(HWND window,
	int item_control_accelerator_id,HWND control_window,UINT notify_code)
/*******************************************************************************
LAST MODIFIED : 26 March 1997

DESCRIPTION :
==============================================================================*/
{
	ENTER(Acquisition_window_WM_COMMAND_handler);
	switch (item_control_accelerator_id)
	{
		case IDCANCEL:
		{
			DestroyWindow(window);
		} break;
	}
	LEAVE;
} /* Acquisition_window_WM_COMMAND_handler */
#endif /* defined (USE_DIALOG) */
#endif /* defined (WINDOWS) */

#if defined (WINDOWS)
#if defined (USE_DIALOG)
static BOOL CALLBACK Acquisition_window_dialog_proc(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 26 March 1997

DESCRIPTION :
==============================================================================*/
{
	BOOL return_code;

	ENTER(Acquisition_window_dialog_proc);
	return_code=FALSE;
	switch (message_identifier)
	{
		case WM_COMMAND:
		{
			return_code=(BOOL)HANDLE_WM_COMMAND(window,first_message,second_message,
				Acquisition_window_WM_COMMAND_handler);
		} break;
	}
	LEAVE;

	return (return_code);
} /* Acquisition_window_dialog_proc */
#endif /* defined (USE_DIALOG) */
#endif /* defined (WINDOWS) */

/*
Global functions
----------------
*/
struct Acquisition_window *create_Acquisition_window(
	struct Acquisition_window **address,
#if defined (MOTIF)
	Widget activation,Widget parent,
#endif /* defined (MOTIF) */
	struct Rig **rig_address,
#if defined (MOTIF)
	Pixel identifying_colour,
#endif /* defined (MOTIF) */
	struct Mapping_window **mapping_window_address,int pointer_sensitivity,
	char *signal_file_extension_write,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
This function allocates the memory for an acquisition window and sets the fields
to the specified values (<address>, <activation>, <rig_address>).  It then
retrieves an acquisition window widget with the specified <parent> and assigns
the widget ids to the appropriate fields of the structure.  If successful it
returns a pointer to the created acquisition window and, if <address> is not
NULL, makes <*address> point to the created acquisition window.  If
unsuccessful, NULL is returned.
???DB.  Creating shell shouldn't be separate
==============================================================================*/
{
#if defined (MOTIF)
	Display *display;
	int depth;
	Pixmap depth_screen_drawable;
	MrmType acquisition_window_class;
	static MrmRegisterArg callback_list[]=
	{
		{"identify_acquisition_acquire_bu",
			(XtPointer)identify_acquisition_acquire_bu},
		{"acquire_data",(XtPointer)acquire_data},
		{"identify_acquisition_monitoring",
			(XtPointer)identify_acquisition_monitoring},
		{"start_stop_monitoring",(XtPointer)start_stop_monitoring},
		{"identify_acquisition_experiment",
			(XtPointer)identify_acquisition_experiment},
		{"start_stop_experiment",(XtPointer)start_stop_experiment},
		{"identify_acquisition_calibrate",
			(XtPointer)identify_acquisition_calibrate},
		{"set_calibrate_interface_module",
			(XtPointer)set_calibrate_interface_module},
		{"calibrate_channels",(XtPointer)calibrate_channels},
		{"identify_acquisition_page_butto",
			(XtPointer)identify_acquisition_page_butto},
		{"identify_acquisition_close_butt",
			(XtPointer)identify_acquisition_close_butt},
		{"identify_acquisition_drawing_ar",
			(XtPointer)identify_acquisition_drawing_ar},
		{"expose_acquisition_drawing_area",
			(XtPointer)expose_acquisition_drawing_area},
		{"resize_acquisition_drawing_area",
			(XtPointer)resize_acquisition_drawing_area},
		{"select_acquisition_drawing_area",
			(XtPointer)select_acquisition_drawing_area},
		{"destroy_Acquisition_window",(XtPointer)destroy_Acquisition_window}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"acquisition_window_structure",(XtPointer)NULL},
		{"identifying_colour",(XtPointer)NULL}
	};
#define XmNacquisitionIntervalColour "acquisitionIntervalColour"
#define XmCAcquisitionIntervalColour "AcquisitionIntervalColour"
#define XmNcalibrationDirectory "calibrationDirectory"
#define XmCCalibrationDirectory "CalibrationDirectory"
#define XmNdrawingBackgroundColour "drawingBackgroundColour"
#define XmCDrawingBackgroundColour "DrawingBackgroundColour"
#define XmNsamplingFrequencyHz "samplingFrequencyHz"
#define XmCSamplingFrequencyHz "SamplingFrequencyHz"
	static XtResource resources[]=
	{
		{
			XmNacquisitionIntervalColour,
			XmCAcquisitionIntervalColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Acquisition_window_settings,acquisition_interval_colour),
			XmRString,
			"purple"
		},
		{
			XmNcalibrationDirectory,
			XmCCalibrationDirectory,
			XmRString,
			sizeof(char *),
			XtOffsetOf(Acquisition_window_settings,calibration_directory),
			XmRString,
			""
		},
		{
			XmNdrawingBackgroundColour,
			XmCDrawingBackgroundColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(Acquisition_window_settings,background_drawing_colour),
			XmRString,
			"lightgray"
		},
		{
			XmNsamplingFrequencyHz,
			XmCSamplingFrequencyHz,
			XmRInt,
			sizeof(int),
			XtOffsetOf(Acquisition_window_settings,sampling_frequency_hz),
			XmRString,
			"1000"
		},
	};
	unsigned long mask;
	XGCValues values;
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
	BOOL win32_return_code;
	static char *class_name="Acquisition_window";
	WNDCLASSEX class_information;
#endif /* defined (WINDOWS) */
	struct Acquisition_window *acquisition;

	ENTER(create_Acquisition_window);
	/* check arguments */
	if (user_interface)
	{
#if defined (MOTIF)
		if (MrmOpenHierarchy_base64_string(acquisition_window_uidh,
			&acquisition_window_hierarchy,&acquisition_window_hierarchy_open))
		{
#endif /* defined (MOTIF) */
			/* allocate memory */
			if (ALLOCATE(acquisition,struct Acquisition_window,1))
			{
				/* assign fields */
				acquisition->address=address;
#if defined (WINDOWS)
				acquisition->device_driver=CreateFile("\\\\.\\VUNEMAPD.VXD",0,0,NULL,
					OPEN_EXISTING,FILE_FLAG_DELETE_ON_CLOSE,0);
#endif /* defined (WINDOWS) */
#if defined (MOTIF)
				acquisition->activation=activation;
#endif /* defined (MOTIF) */
				acquisition->rig_address=rig_address;
				acquisition->signal_file_extension_write=signal_file_extension_write;
				acquisition->mapping_window_address=mapping_window_address;
#if defined (MOTIF)
				acquisition->window=(Widget)NULL;
				acquisition->acquire_button=(Widget)NULL;
#endif /* defined (MOTIF) */
				acquisition->acquire_file_open_data=(struct File_open_data *)NULL;
#if defined (MOTIF)
				acquisition->monitoring_toggle=(Widget)NULL;
				/*???DB.  Temporary ? */
#endif /* defined (MOTIF) */
				acquisition->monitoring_file_open_data=(struct File_open_data *)NULL;
#if defined (MOTIF)
				acquisition->experiment_toggle=(Widget)NULL;
#endif /* defined (MOTIF) */
				acquisition->experiment_file_open_data=(struct File_open_data *)NULL;
#if defined (MOTIF)
				acquisition->calibrate_button=(Widget)NULL;
				acquisition->page_button=(Widget)NULL;
				acquisition->close_button=(Widget)NULL;
				acquisition->drawing_area=(Widget)NULL;
#endif /* defined (MOTIF) */
				acquisition->drawing=(struct Drawing_2d *)NULL;
				acquisition->start_time=0;
				acquisition->acquire_time=0;
				acquisition->end_time=MAXIMUM_NUMBER_OF_SAMPLES;
				acquisition->start_point= -1;
				acquisition->acquire_point= -1;
				acquisition->end_point= -1;
				acquisition->marker_bottom= -1;
				acquisition->marker_centre= -1;
				acquisition->marker_top= -1;
				acquisition->pointer_sensitivity=pointer_sensitivity;
				acquisition->user_interface=user_interface;
#if defined (MOTIF)
				/* retrieve the settings */
				XtVaGetApplicationResources(user_interface->application_shell,
					acquisition,resources,XtNumber(resources),NULL);
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
					acquisition_window_hierarchy,callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)acquisition;
					identifier_list[1].value=(XtPointer)identifying_colour;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						acquisition_window_hierarchy,identifier_list,
						XtNumber(identifier_list)))
					{
						/* fetch the acquisition window widget */
						if (MrmSUCCESS==MrmFetchWidget(acquisition_window_hierarchy,
							"acquisition_window",parent,&(acquisition->window),
							&acquisition_window_class))
						{
							/* set the background colour for the acquisition drawing area */
							XtVaSetValues(acquisition->drawing_area,
								XmNbackground,acquisition->background_drawing_colour,
								NULL);
							/* set ghosting for the buttons */
							if (rig_address)
							{
								if (*rig_address)
								{
									XtSetSensitive(acquisition->experiment_toggle,True);
									if (EXPERIMENT_ON==(*rig_address)->experiment)
									{
										XtSetSensitive(acquisition->monitoring_toggle,True);
										if ((*rig_address)->page_list)
										{
											XtSetSensitive(acquisition->page_button,True);
										}
										else
										{
											XtSetSensitive(acquisition->page_button,False);
										}
										if (MONITORING_ON==(*rig_address)->monitoring)
										{
											XtSetSensitive(acquisition->acquire_button,True);
										}
										else
										{
											XtSetSensitive(acquisition->acquire_button,False);
										}
									}
									else
									{
										XtSetSensitive(acquisition->acquire_button,False);
										XtSetSensitive(acquisition->page_button,False);
										XtSetSensitive(acquisition->monitoring_toggle,False);
									}
								}
								else
								{
									XtSetSensitive(acquisition->acquire_button,False);
									XtSetSensitive(acquisition->page_button,False);
									XtSetSensitive(acquisition->monitoring_toggle,False);
									XtSetSensitive(acquisition->experiment_toggle,False);
								}
							}
							else
							{
								XtSetSensitive(acquisition->acquire_button,False);
								XtSetSensitive(acquisition->page_button,False);
								XtSetSensitive(acquisition->monitoring_toggle,False);
								XtSetSensitive(acquisition->experiment_toggle,False);
							}
							/* create the graphics contexts */
							display=user_interface->display;
							/* the drawable has to have the correct depth and screen */
							XtVaGetValues(user_interface->application_shell,XmNdepth,&depth,
								NULL);
							depth_screen_drawable=XCreatePixmap(user_interface->display,
								XRootWindow(user_interface->display,
								XDefaultScreen(user_interface->display)),1,1,depth);
							mask=GCLineStyle|GCBackground|GCFont|GCForeground|GCFunction;
							values.font=user_interface->normal_font->fid;
							values.line_style=LineSolid;
							values.background=acquisition->background_drawing_colour;
							values.foreground=acquisition->acquisition_interval_colour^
								acquisition->background_drawing_colour;
							values.function=GXxor;
							(acquisition->graphics_context).acquisition_interval_colour=
								XCreateGC(display,depth_screen_drawable,mask,&values);
							(acquisition->graphics_context).acquisition_interval_colour_tex=
								XCreateGC(display,depth_screen_drawable,mask,&values);
							values.foreground=acquisition->background_drawing_colour;
							values.function=GXcopy;
							(acquisition->graphics_context).background_drawing_colour=
								XCreateGC(display,depth_screen_drawable,mask,&values);
							(acquisition->graphics_context).copy=
								XCreateGC(display,depth_screen_drawable,mask,&values);
							XFreePixmap(user_interface->display,depth_screen_drawable);
							if (address)
							{
								*address=acquisition;
							}
							/* manage the acquisition window */
							XtManageChild(acquisition->window);
						}
						else
						{
							display_message(ERROR_MESSAGE,
			"create_Acquisition_window.  Could not fetch acquisition window widget");
							DEALLOCATE(acquisition);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Acquisition_window.  Could not register the identifiers");
						DEALLOCATE(acquisition);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Acquisition_window.  Could not register the callbacks");
					DEALLOCATE(acquisition);
				}
#endif /* defined (MOTIF) */
#if defined (WINDOWS)
				/* check if the class is registered */
				if (TRUE!=(win32_return_code=GetClassInfoEx(user_interface->instance,
					class_name,&class_information)))
				{
					class_information.cbClsExtra=0;
#if defined (USE_DIALOG)
					class_information.cbWndExtra=
						DLGWINDOWEXTRA+sizeof(struct Acquisition_window *);
#endif /* defined (USE_DIALOG) */
#if defined (USE_WINDOW)
					class_information.cbWndExtra=sizeof(struct Acquisition_window *);
#endif /* defined (USE_WINDOW) */
					class_information.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
					class_information.hCursor=LoadCursor(NULL,IDC_ARROW);
					class_information.hIcon=(HICON)NULL;
/*					class_information.hIcon=LoadIcon(user_interface->instance,class_name);*/
						/*???DB.  Do I need an icon ? */
					class_information.hInstance=user_interface->instance;
					class_information.lpfnWndProc=Acquisition_window_class_proc;
					class_information.lpszClassName=class_name;
					class_information.style=CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
					/* allow resource to specify the menu */
					class_information.lpszMenuName=NULL;
					/*???DB.  Extra in WNDCLASSEX over WNDCLASS */
					class_information.cbSize=sizeof(WNDCLASSEX);
					class_information.hIconSm=(HICON)NULL;
/*					class_information.hIconSm=LoadIcon(user_interface->instance,
						"Acquisition_window" "_small");*/
						/*???DB.  Do I need an icon ? */
					if (RegisterClassEx(&class_information))
					{
						win32_return_code=TRUE;
					}
				}
				/* create the window */
				if (TRUE==win32_return_code)
				{
#if defined (USE_DIALOG)
					if (acquisition->window=CreateDialogParam(
						user_interface->instance,"Acquisition_window",(HWND)NULL,
						Acquisition_window_dialog_proc,(LPARAM)NULL))
#endif /* defined (USE_DIALOG) */
#if defined (USE_WINDOW)
					if (acquisition->window=CreateWindowEx(
						WS_EX_OVERLAPPEDWINDOW,
							/* extended window styles */
						class_name,
							/* class name */
						"Acquisition",
							/* window name */
							/*???DB.  Is this what goes in the title bar ? */
						WS_OVERLAPPEDWINDOW,
							/* window styles */
						CW_USEDEFAULT,
							/* horizontal position - default */
						CW_USEDEFAULT,
							/* vertical position - default */
						CW_USEDEFAULT,
							/* width - default */
						CW_USEDEFAULT,
							/* height - default */
						(HWND)NULL,
							/* parent or owner window - none */
						(HMENU)NULL,
							/* menu to be used - use class menu */
						user_interface->instance,
							/* instance handle */
						acquisition
							/* window creation data */
						))
#endif /* defined (USE_WINDOW) */
					{
						if (address)
						{
							*address=acquisition;
						}
						ShowWindow(acquisition->window,SW_SHOWDEFAULT);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_Acquisition_window.  Could not create dialog");
						DEALLOCATE(acquisition);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Acquisition_window.  Unable to register class information");
					DEALLOCATE(acquisition);
				}
#endif /* defined (WINDOWS) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
"create_Acquisition_window.  Could not allocate acquisition window structure");
			}
#if defined (MOTIF)
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Acquisition_window.  Could not open hierarchy");
			acquisition=(struct Acquisition_window *)NULL;
		}
#endif /* defined (MOTIF) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Acquisition_window.  Missing user_interface");
		acquisition=(struct Acquisition_window *)NULL;
	}
	LEAVE;

	return (acquisition);
} /* create_Acquisition_window */

#if defined (MOTIF)
Widget create_acquisition_window_shell(Widget *address,Widget parent,
	int screen_width,int screen_height,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 19 May 1998

DESCRIPTION :
Creates a popup shell widget for an acquisition window.  If <address> is not
NULL, <*address> is set to the id of the created shell and on destruction
<*address> will be set to NULL.  The id of the created widget is returned.
==============================================================================*/
{
	Widget shell;

	ENTER(create_acquisition_window_shell);
	/* create and place the acquisition window shell */
	if (shell=XtVaCreatePopupShell("acquisition_window_shell",
		topLevelShellWidgetClass,parent,
		XmNallowShellResize,True,
		XmNx,0,
		XmNy,0,
		XmNwidth,screen_width/2,
		XmNheight,screen_height/8,
		XmNuserData,user_interface,
		NULL))
	{
		if (address)
		{
			*address=shell;
			/* add the destroy callback */
			XtAddCallback(shell,XmNdestroyCallback,destroy_window_shell,
				(XtPointer)create_Shell_list_item(address,user_interface));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"create_acquisition_window_shell.  Could create the acquisition window shell");
	}
	LEAVE;

	return (shell);
} /* create_acquisition_window_shell */
#endif /* defined (MOTIF) */
