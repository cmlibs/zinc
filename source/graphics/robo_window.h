/*******************************************************************************
FILE : robo_window.h

LAST MODIFIED : 2 November 1995

DESCRIPTION :
Definitions for a controlling window for representation of robot
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
