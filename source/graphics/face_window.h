/*******************************************************************************
FILE : face_window.h

LAST MODIFIED : 20 June 1996

DESCRIPTION :
Definitions for a controlling window for animation of face
==============================================================================*/
#if !defined (FACE_WINDOW_H)
#define FACE_WINDOW_H

#include <stddef.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Mrm/MrmPublic.h>

/*
Global types
------------
*/
struct Face_window
{
	MrmHierarchy hierarchy;
	Widget app_shell,window_shell,main_window;
	Widget btn1,btn2;
	Widget slider1,slider2,slider3;
	struct Graphics_window *owner;
};

/*
Global functions
----------------
*/
/*struct Face_window *create_face_window(struct Graphics_window *owner);*/

void destroy_face_window_callback(Widget widget_id, XtPointer face_window,
	XtPointer call_data);
void close_face_window(struct Face_window *the_window);
void close_faceCB(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs);
void facebtn1CB(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs);
void faceslider1CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs);
void faceslider2CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs);
void faceslider3CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs);
#endif
