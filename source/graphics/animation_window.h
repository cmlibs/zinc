/*******************************************************************************
FILE : animation_window.h

LAST MODIFIED : 19 June 1996

DESCRIPTION :
Definitions for a controlling window for creation of animations
==============================================================================*/
#if !defined (ANIMATION_WINDOW_H)
#define ANIMATION_WINDOW_H

#include <stddef.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Mrm/MrmPublic.h>
#include "user_interface/user_interface.h"

#define N_TEXT_FIELDS 9

/* the text fields represent :
0,1,2   eyepoint x,y,z
3,4,5   point of interest x,y,z
6       point number,
7       fov,
8       time
*/


struct Animation_Point_List_Item
{
float point_number;
double eye[3];
double poi[3];
double fov;
struct Animation_Point_List_Item *next;

/* time at desired point in video :knot parameter */
double time;
};

struct Animation_window
{
	MrmHierarchy hierarchy;
	Widget app_shell,window_shell,main_window;

	float point_index;
	Widget animation_text_fields[N_TEXT_FIELDS];
	struct Animation_Point_List_Item *animation_point_list;
	struct Graphics_window *owner;
	double *eye_spline_x,*eye_spline_y,*eye_spline_z,
	*poi_spline_x,*poi_spline_y,*poi_spline_z,
	*spline_knots,*fov_spline;
	int n_spline_points;
	int n_frames;

};

/* Global Functions */
struct Animation_window *create_animation_window(struct Graphics_window *owner,
	struct User_interface *user_interface);
void destroy_animation_window_callback(Widget widget_id, XtPointer seat_window,
	XtPointer call_data);
#if defined (OLD_CODE)
void destroy_animation_window_callback(Widget widget_id, XtPointer seat_window,
	XmAnyCallbackStruct *call_data);
#endif

void animation_close(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs);
void animation_inc_point(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs);
void animation_dec_point(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs);
void animation_text_value_changed(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs);
void animation_text_field_created(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs);
void animation_accept(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs);
void animation_clear(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs);
void animation_create(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs);
void animation_load(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs);

#endif
