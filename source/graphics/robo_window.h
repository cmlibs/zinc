/*******************************************************************************
FILE : robo_window.h

LAST MODIFIED : 2 November 1995

DESCRIPTION :
Definitions for a controlling window for representation of robot
==============================================================================*/
#if !defined (ROBO_WINDOW_H)
#define ROBO_WINDOW_H

#include <stddef.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>

struct Robo_window
{
	MrmHierarchy hierarchy;
	Widget app_shell,window_shell,main_window;


	Widget btn1,btn2,btn3;
	Widget slider1,
	slider2,
	slider3,
	slider4,
	slider5,
	slider6,
	slider7;

	struct Graphics_window *owner;
};

/* Global Functions */
struct Robo_window *create_robo_window(struct Graphics_window *owner);
void destroy_robo_window_callback(Widget widget_id, XtPointer robo_window,
	XtPointer call_data);
void close_robo_window(struct Robo_window *the_window);

void close_roboCB(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs);
void robobtn1CB(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs);
void robobtn3CB(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs);
void roboslider1CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs);
void roboslider2CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs);
void roboslider3CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs);
void roboslider4CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs);
void roboslider5CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs);
void roboslider6CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs);
void roboslider7CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs);

void identifyslider1(Widget w,XtPointer
		client_data,XmScrollBarCallbackStruct *cbs);
void identifyslider2(Widget w,XtPointer
		client_data,XmScrollBarCallbackStruct *cbs);
void identifyslider3(Widget w,XtPointer
		client_data,XmScrollBarCallbackStruct *cbs);
void identifyslider4(Widget w,XtPointer
		client_data,XmScrollBarCallbackStruct *cbs);
void identifyslider5(Widget w,XtPointer
		client_data,XmScrollBarCallbackStruct *cbs);
void identifyslider6(Widget w,XtPointer
		client_data,XmScrollBarCallbackStruct *cbs);
void identifyslider7(Widget w,XtPointer
		client_data,XmScrollBarCallbackStruct *cbs);

void set_robo_sliders(struct Drawing *drawing,struct Robo_window *robo_window);
void makerobotprimitives(void);
void maketool(void);
#endif
