/*******************************************************************************
FILE : robo_window.c

LAST MODIFIED : 4 January 1998

DESCRIPTION :
Management routines for the robot command window.
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

#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/List.h>
#include <Xm/Protocols.h>
#include <math.h>
#include "general/debug.h"
#include "graphics/robo_window.h"
#include "graphics/graphics_window.h"
#include "graphics/graphics_library.h"
#include "graphics/scene.h"
#include "user_interface/message.h"

/*
Module variables
----------------
*/
static int robo_window_hierarchy_open=0;
static MrmHierarchy robo_window_hierarchy;

double slidermax =  1000.0;
double sliderscale = 0.2;
double rsliderscale = 0.2;

extern void remakerobot(struct Drawing *);
/* robot primitives */
int cylinder = 9998, scalpel = 9999,robot = 9997,tool = 9996;
int cone = 9995,scalpel2=9994;
int light_tool = 9993,syringe = 9992;
int vector = 9991,wound = 9990,stretcher = 9989,sphere = 9988,block = 9987;
int stapler = 9986,cutter = 9985,gripper = 9984, scope = 9983,fixation = 9982,guarded_blade = 9981;
int scalpel3 = 9980;
int robot_7dof = 9979;
double cf;
float cube_vert[8][3]=
{
	{-1,-1,-1},
	{1,-1,-1},
	{1,1,-1},
	{-1,1,-1},
	{-1,-1,1},
	{1,-1,1},
	{1,1,1},
	{-1,1,1}
};
float cube_normals[6][3]=
{
	{0,0,-1},
	{0,0,1},
	{0,-1,0},
	{1,0,0},
	{0,1,0},
	{-1,0,0}
};



void makeannulus(double r1,double r2,double t1,double t2,double h)
{
static int wedge[4][4] = { {0,2,3,1},{4,5,7,6},{0,4,6,2},{1,3,7,5}};
double res = 40;
double t;
double x[8][3];
double tstep,rstep;
float n[3],v[3];
int i,j,k,poly,vertex;
rstep = r2-r1;

tstep = (t2-t1)/res;

for (t = t1;t<t2;t+=tstep)
	{
	for (k=0;k<2;k++)
		{
		for (j=0;j<2;j++)
			{
			for (i=0;i<2;i++)
				{
				x[i+2*j+4*k][0] = (r1 + i*rstep)*cos(t+j*tstep);
				x[i+2*j+4*k][1] = (r1 + i*rstep)*sin(t+j*tstep);
				x[i+2*j+4*k][2] = (-h/2.0+k*h);
				}
			}
		}
	for (poly =0;poly<4;poly++)
		{
		bgnpolygon();

		for (vertex=0;vertex<4;vertex++)
			{
			switch (poly)
				{
				case 0:  n[0]=0;n[1]=0,n[2]=-1;
				break;
				case 1: n[0]=0;n[1]=0,n[2]=1;
				break;
				case 2:
				n[0] = x[wedge[poly][vertex]][0]/r1;
				n[1] = x[wedge[poly][vertex]][1]/r1;
				n[2]=0;
				break;
				case 3:
				n[0] = x[wedge[poly][vertex]][0]/r2;
				n[1] = x[wedge[poly][vertex]][1]/r2;
				n[2]=0;
				break;
				default: break;
				};
			for (i=0;i<3;i++)
				{
				v[i] = x[wedge[poly][vertex]][i];
				}
			n3f(n);
			v3f(v);
			}
		endpolygon();
		}
	}
}


void *create_application_window2(struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
Create the structures and retrieve the command window from the uil file.
==============================================================================*/
{
	struct Robo_window *robo_window;
	MrmType robo_window_class;
	static MrmRegisterArg callback_list[] = {
		{ "robobtn1CB", (XtPointer)robobtn1CB },
	{ "robobtn3CB", (XtPointer)robobtn3CB },
		{ "close_roboCB", (XtPointer)close_roboCB},
	{ "roboslider1CB", (XtPointer)roboslider1CB},
	{ "roboslider2CB", (XtPointer)roboslider2CB},
		{ "roboslider3CB", (XtPointer)roboslider3CB},
	{ "roboslider4CB", (XtPointer)roboslider4CB},
	{ "roboslider5CB", (XtPointer)roboslider5CB},
	{ "roboslider6CB", (XtPointer)roboslider6CB},
	{ "roboslider7CB", (XtPointer)roboslider7CB },
	{ "identifyslider1", (XtPointer)identifyslider1 },
	{ "identifyslider2", (XtPointer)identifyslider2 },
	{ "identifyslider3", (XtPointer)identifyslider3 },
	{ "identifyslider4", (XtPointer)identifyslider4 },
	{ "identifyslider5", (XtPointer)identifyslider5 },
	{ "identifyslider6", (XtPointer)identifyslider6 },
	{ "identifyslider7", (XtPointer)identifyslider7 }

	};
	static MrmRegisterArg identifiers[] =
	{
		{"robo_window_pointer",(XtPointer)0}
	};

	ENTER(create_robo_window);
	if (specialized_MrmOpenHierarchy("robo_window.uid",&robo_window_hierarchy,
		&robo_window_hierarchy_open))
	{
		if (ALLOCATE(robo_window,struct Robo_window, 1))
		{
			robo_window->app_shell = (Widget) NULL;
			robo_window->window_shell = (Widget) NULL;
			robo_window->main_window = (Widget) NULL;
			robo_window->btn1 = (Widget) NULL;
			robo_window->btn2 = (Widget) NULL;
			robo_window->slider1 = (Widget) NULL;
			robo_window->slider2 = (Widget) NULL;
			robo_window->slider3 = (Widget) NULL;
			robo_window->slider4 = (Widget) NULL;
			robo_window->slider5 = (Widget) NULL;
			robo_window->slider6 = (Widget) NULL;
			robo_window->slider7 = (Widget) NULL;
			robo_window->owner = graphics_window;
			if(robo_window->window_shell = XtVaCreatePopupShell("robo_window_shell",
				transientShellWidgetClass, application_shell,
					/*???DB.  application_shell should be passed ? */
				XmNmwmDecorations, MWM_DECOR_ALL|MWM_DECOR_MAXIMIZE,
				XmNmwmFunctions, MWM_FUNC_ALL|MWM_FUNC_MAXIMIZE,
				XmNtransient, FALSE,
				NULL))
			{
				Atom WM_DELETE_WINDOW;  /*Delete callback thinggy.      */

				/* Add destroy callback */
				XtAddCallback(robo_window->window_shell, XmNdestroyCallback,
					destroy_robo_window_callback, (XtPointer) robo_window);
					WM_DELETE_WINDOW = XmInternAtom(XtDisplay(robo_window->window_shell),
					"WM_DELETE_WINDOW",FALSE);
				/* pass the robo_window_pointer to the uid stuff */
				/* Register callbacks in UIL */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(robo_window_hierarchy,
					callback_list,XtNumber(callback_list)))
				{
					/* pass the robo_window_pointer to the uid stuff */
					identifiers[0].value = (XtPointer)robo_window;
					if (MrmSUCCESS!=MrmRegisterNamesInHierarchy(robo_window_hierarchy,
						identifiers,XtNumber(identifiers)))
					{
						if (MrmSUCCESS==MrmFetchWidget(robo_window_hierarchy,"robo_window",
							robo_window->window_shell,&(robo_window->main_window),
							&robo_window_class))
						{
							XtManageChild(robo_window->main_window);
							XtRealizeWidget(robo_window->window_shell);
							XtPopup(robo_window->window_shell, XtGrabNone);
						}
						else
						{
							display_message(ERROR_MESSAGE, "Could not retrieve robo widget");
							DEALLOCATE(robo_window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE, "Could not register names");
						DEALLOCATE(robo_window);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "Could not register callbacks");
					DEALLOCATE(robo_window);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "Could not create shell");
				DEALLOCATE(robo_window);
			}
		}
		else
		{
			display_message(CM_MEM_NOT_ALLOC);
			robo_window = (struct Robo_window *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Could not open hierarchy");
		robo_window = (struct Robo_window *)NULL;
	}
	LEAVE;

	return ((void *) robo_window);
} /* create_robo_window */

void close_robo_window(
	struct Robo_window  *the_window    /*Window to close and destroy.  */
	)
/*********************************************************************************
DESCRIPTION:
		Closes and destroys the window pointed to by the window.  After this the
		window must be recreated.  The routine relies on close_help_window_callback
		to actually dispose of the window description structure and call the
		destroy function (if necessary).
================================================================================*/
{
	ENTER(close_robo_window);

printf("in close_robo_window\n");
	XtPopdown(the_window->window_shell);
	XtDestroyWidget(the_window->window_shell);

	LEAVE;
} /*close_robo_window*/


void destroy_robo_window_callback(
	Widget    caller,      /*Widget sending the callback.  */
	XtPointer  the_window,    /*Window description structure. */
	XtPointer caller_data    /*Callback data.    */
	)
/*********************************************************************************
DESCRIPTION:
		Callback for when the window is closed and destroyed.  Calls the destroy_func
		if necessary and disposes of the window description structure.
================================================================================*/
{
	ENTER(destroy_robo_window_callback);

	printf("destroying robo_window\n");
	DEALLOCATE(the_window);

	LEAVE;
}/*destroy_robo_window_callback*/


void close_roboCB(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
{
printf("close_roboCB called\n");
close_robo_window((struct Robo_window *) client_data);
}

void robobtn1CB(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
{
struct Robo_window *robo_window;
struct Graphics_window *graphics_window;
struct Drawing *drawing;
robo_window = (struct Robo_window *) client_data;
drawing = robo_window->owner->drawing;
printf("btn1CB called\n");
drawing->tool_index++;
if (drawing->tool_index >= drawing->num_tools)
	drawing->tool_index = 0;

printf("current tool = %d\n",drawing->tool_index);
printf("r.x = %d\n",(int)drawing->robot[drawing->tool_index].x+50);

XtVaSetValues(robo_window->slider1,XmNvalue,drawing->robot[drawing->tool_index].value1,NULL);
XtVaSetValues(robo_window->slider2,XmNvalue,drawing->robot[drawing->tool_index].value2,NULL);
XtVaSetValues(robo_window->slider3,XmNvalue,drawing->robot[drawing->tool_index].value3,NULL);
XtVaSetValues(robo_window->slider4,XmNvalue,drawing->robot[drawing->tool_index].value4,NULL);
XtVaSetValues(robo_window->slider5,XmNvalue,drawing->robot[drawing->tool_index].value5,NULL);
XtVaSetValues(robo_window->slider6,XmNvalue,drawing->robot[drawing->tool_index].value6,NULL);
XtVaSetValues(robo_window->slider7,XmNvalue,drawing->robot[drawing->tool_index].value7,NULL);

drawing->robot[drawing->tool_index].visible = 1;
	/*  !drawing->robot[drawing->tool_index].visible;  */
remakerobot(drawing);
drawing_changed(drawing);
}

void robobtn3CB(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
{
struct Robo_window *robo_window;
struct Graphics_window *graphics_window;
struct Drawing *drawing;
robo_window = (struct Robo_window *) client_data;
drawing = robo_window->owner->drawing;
printf("btn3CB called\n");
drawing->robot[drawing->tool_index].visible =
		!drawing->robot[drawing->tool_index].visible;
remakerobot(drawing);
drawing_changed(drawing);
}




void roboslider1CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs)
{
struct Robo_window *robo_window;
struct Graphics_window *graphics_window;
struct Drawing *drawing;
printf("roboslider1CB called\n");
robo_window = (struct Robo_window *) client_data;
drawing = robo_window->owner->drawing;
drawing->robot[drawing->tool_index].value1 = cbs->value;
drawing->robot[drawing->tool_index].x = (float) cbs->value * sliderscale;
remakerobot(drawing);
drawing_changed(drawing);
}
void roboslider2CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs)
{
struct Robo_window *robo_window;
struct Graphics_window *graphics_window;
struct Drawing *drawing;
printf("roboslider2CB called\n");
robo_window = (struct Robo_window *) client_data;
drawing = robo_window->owner->drawing;
drawing->robot[drawing->tool_index].value2 = cbs->value;
drawing->robot[drawing->tool_index].y = (float) cbs->value*sliderscale;
remakerobot(drawing);
drawing_changed(drawing);
}
void roboslider3CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs)
{
struct Robo_window *robo_window;
struct Graphics_window *graphics_window;
struct Drawing *drawing;
printf("roboslider3CB called\n");
robo_window = (struct Robo_window *) client_data;
drawing = robo_window->owner->drawing;
drawing->robot[drawing->tool_index].value3 = cbs->value;
drawing->robot[drawing->tool_index].z = (float) cbs->value*sliderscale;
remakerobot(drawing);
drawing_changed(drawing);

}
void roboslider4CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs)
{
struct Robo_window *robo_window;
struct Graphics_window *graphics_window;
struct Drawing *drawing;
printf("roboslider4CB called\n");
robo_window = (struct Robo_window *) client_data;
drawing = robo_window->owner->drawing;
drawing->robot[drawing->tool_index].value4 = cbs->value;
drawing->robot[drawing->tool_index].t = ((float) cbs->value ) *
PI/slidermax; remakerobot(drawing);
drawing_changed(drawing);
}
void roboslider5CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs)
{
struct Robo_window *robo_window;
struct Graphics_window *graphics_window;
struct Drawing *drawing;
printf("roboslider5CB called\n");
robo_window = (struct Robo_window *) client_data;
drawing = robo_window->owner->drawing;
drawing->robot[drawing->tool_index].value5 = cbs->value;
drawing->robot[drawing->tool_index].p = ((float) cbs->value ) *
PI/slidermax; remakerobot(drawing);
drawing_changed(drawing);
}

void roboslider6CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs)
{
struct Robo_window *robo_window;
struct Graphics_window *graphics_window;
struct Drawing *drawing;
printf("roboslider6CB called\n");
robo_window = (struct Robo_window *) client_data;
drawing = robo_window->owner->drawing;
drawing->robot[drawing->tool_index].value6 = cbs->value;
drawing->robot[drawing->tool_index].r = cbs->value * rsliderscale +50.0;
/* at present -> remakescene to set robot visibility for diff advances */
remakerobot(drawing);
remakescene(drawing);
drawing_changed(drawing);
}
void roboslider7CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs)
{
struct Robo_window *robo_window;
struct Graphics_window *graphics_window;
struct Drawing *drawing;
printf("roboslider7CB called\n");
robo_window = (struct Robo_window *) client_data;
drawing = robo_window->owner->drawing;
drawing->robot[drawing->tool_index].value7 = cbs->value;
drawing->robot[drawing->tool_index].rot = ((float) cbs->value ) *
PI/slidermax;
remakerobot(drawing);
drawing_changed(drawing);
}

void identifyslider1(Widget w,XtPointer
		client_data,XmScrollBarCallbackStruct *cbs)
{
struct Robo_window *robo_window;
robo_window = (struct Robo_window *) client_data;
robo_window->slider1 = w;
}
void identifyslider2(Widget w,XtPointer
		client_data,XmScrollBarCallbackStruct *cbs)
{
struct Robo_window *robo_window;
robo_window = (struct Robo_window *) client_data;
robo_window->slider2 = w;
}
void identifyslider3(Widget w,XtPointer
		client_data,XmScrollBarCallbackStruct *cbs)
{
struct Robo_window *robo_window;
robo_window = (struct Robo_window *) client_data;
robo_window->slider3 = w;
}
void identifyslider4(Widget w,XtPointer
		client_data,XmScrollBarCallbackStruct *cbs)
{
struct Robo_window *robo_window;
robo_window = (struct Robo_window *) client_data;
robo_window->slider4 = w;
}
void identifyslider5(Widget w,XtPointer
		client_data,XmScrollBarCallbackStruct *cbs)
{
struct Robo_window *robo_window;
robo_window = (struct Robo_window *) client_data;
robo_window->slider5 = w;
}
void identifyslider6(Widget w,XtPointer
		client_data,XmScrollBarCallbackStruct *cbs)
{
struct Robo_window *robo_window;
robo_window = (struct Robo_window *) client_data;
robo_window->slider6 = w;
}
void identifyslider7(Widget w,XtPointer
		client_data,XmScrollBarCallbackStruct *cbs)
{
struct Robo_window *robo_window;
robo_window = (struct Robo_window *) client_data;
robo_window->slider7 = w;
}

void set_robo_sliders(struct Drawing *drawing,struct Robo_window *robo_window)
{
int i;
for (i=0;i< drawing->num_tools;i++)
	{
	drawing->robot[i].value1 = (int) ((double) drawing->robot[i].x /(double) sliderscale);
	drawing->robot[i].value2 = (int) ((double) drawing->robot[i].y /(double) sliderscale);
	drawing->robot[i].value3 = (int) ((double) drawing->robot[i].z /(double) sliderscale);
	drawing->robot[i].value4 = (int) ((double) drawing->robot[i].t * (double) slidermax/ PI);
	drawing->robot[i].value5 = (int) ((double) drawing->robot[i].p * (double) slidermax/ PI);
	drawing->robot[i].value6 = (int) ((double) (drawing->robot[i].r - 50.0) /(double) rsliderscale);
	drawing->robot[i].value7 = (int) ((double) drawing->robot[i].rot * (double) slidermax/ PI);
	}

XtVaSetValues(robo_window->slider1,XmNvalue,drawing->robot[drawing->tool_index].value1,NULL);
XtVaSetValues(robo_window->slider2,XmNvalue,drawing->robot[drawing->tool_index].value2,NULL);
XtVaSetValues(robo_window->slider3,XmNvalue,drawing->robot[drawing->tool_index].value3,NULL);
XtVaSetValues(robo_window->slider4,XmNvalue,drawing->robot[drawing->tool_index].value4,NULL);
XtVaSetValues(robo_window->slider5,XmNvalue,drawing->robot[drawing->tool_index].value5,NULL);
XtVaSetValues(robo_window->slider6,XmNvalue,drawing->robot[drawing->tool_index].value6,NULL);
XtVaSetValues(robo_window->slider7,XmNvalue,drawing->robot[drawing->tool_index].value7,NULL);
}




void makerobotprimitives()
{

double a,ia;
int i,dis = 20;
float norm[3],vert[3];
float metal[] = {
	AMBIENT, 0.4,0.4,0.5,
	SPECULAR, 1.0,1.0,1.0,
	DIFFUSE, 0.7,0.7,0.8,
	SHININESS,120.0,
	LMNULL};
float endo1[] = {
	DIFFUSE, 0.90000, 0.90000, 0.90000,
	SPECULAR, 1.000000, 1.000000, 1.000000,
	AMBIENT, 0.70000, 0.70000, 0.70000,
	SHININESS, 64.000000,
	LMNULL};
float endo2[] = {
	DIFFUSE, 0.60000, 0.60000, 0.60000,
	SPECULAR, 1.000000, 1.000000, 1.000000,
	AMBIENT, 0.50000, 0.50000, 0.50000,
	SHININESS, 32.000000,
	LMNULL};
float endo3[] = {
	DIFFUSE, 0.310000, 0.330000, 0.40000,
	SPECULAR, 1.000000, 1.000000, 1.000000,
	AMBIENT, 0.30000, 0.30000, 0.40000,
	SHININESS, 20.000000,
	LMNULL};
float guarded_blade1[] = {
	DIFFUSE, 0.9, 0.75, 0.0,
	SPECULAR, 0.9, 0.8, 0.7,
	AMBIENT, 0.5, 0.450000, 0.3,
	SHININESS, 30.000000,
	LMNULL};
float lighttip[] = {
	EMISSION, 0.85,0.85,0.85,
	SPECULAR, 0.0,0.0,0.0,
	DIFFUSE, 0.0,0.0,0.0,
	AMBIENT, 0.0,0.0,0.0,
	LMNULL};

lmdef(DEFMATERIAL,999,15,metal);
lmdef(DEFMATERIAL,998,17,lighttip);
lmdef(DEFMATERIAL,997,15,endo1);
lmdef(DEFMATERIAL,996,15,endo2);
lmdef(DEFMATERIAL,995,15,endo3);
lmdef(DEFMATERIAL,994,15,guarded_blade1);
/*
lmdef(DEFLIGHT,7,14,light7);
*/






ia = 2.0*PI/((double) dis);

makeobj(block);

bgnpolygon();
n3f(cube_normals[0]);
v3f(cube_vert[0]);
v3f(cube_vert[1]);
v3f(cube_vert[2]);
v3f(cube_vert[3]);
endpolygon();
bgnpolygon();
n3f(cube_normals[1]);
v3f(cube_vert[4]);
v3f(cube_vert[5]);
v3f(cube_vert[6]);
v3f(cube_vert[7]);
endpolygon();
bgnpolygon();
n3f(cube_normals[2]);
v3f(cube_vert[0]);
v3f(cube_vert[1]);
v3f(cube_vert[5]);
v3f(cube_vert[4]);
endpolygon();
bgnpolygon();
n3f(cube_normals[3]);
v3f(cube_vert[1]);
v3f(cube_vert[2]);
v3f(cube_vert[6]);
v3f(cube_vert[5]);
endpolygon();
bgnpolygon();
n3f(cube_normals[4]);
v3f(cube_vert[2]);
v3f(cube_vert[3]);
v3f(cube_vert[7]);
v3f(cube_vert[6]);
endpolygon();
bgnpolygon();
n3f(cube_normals[5]);
v3f(cube_vert[3]);
v3f(cube_vert[0]);
v3f(cube_vert[4]);
v3f(cube_vert[7]);
endpolygon();
closeobj();

makeobj(cylinder);
for (i=0,a=0;i<dis;i++,a+=ia)
	{
	bgnpolygon();
	norm[0] = vert[0] = cos(a);
	norm[1] = vert[1] = sin(a);
	norm[2] = vert[2] = 0.0;

	n3f(norm);
	v3f(vert);

	vert[2] = 1.0;
	v3f(vert);

	norm[0] = vert[0] = cos(a+ia);
	norm[1] = vert[1] = sin(a+ia);

	n3f(norm);
	v3f(vert);

	vert[2] = 0.0;
	v3f(vert);
	endpolygon();

	}

for (i=0,a=0;i<dis;i++,a+=ia)
	{
	bgnpolygon();
	norm[0] = norm[1] = 0.0;
	norm[2] = -1.0;

	vert[0] = cos(a);
	vert[1] = sin(a);
	vert[2] = 0.0;

	n3f(norm);
	v3f(vert);

	vert[0] = cos(a+ia);
	vert[1] = sin(a+ia);
	v3f(vert);

	vert[0] = vert[1] = 0.0;
	v3f(vert);
	endpolygon();

	bgnpolygon();
	norm[0] = norm[1] = 0.0;
	norm[2] = 1.0;

	vert[0] = cos(a);
	vert[1] = sin(a);
	vert[2] = 1.0;

	n3f(norm);
	v3f(vert);

	vert[0] = cos(a+ia);
	vert[1] = sin(a+ia);
	v3f(vert);

	vert[0] = vert[1] = 0.0;
	v3f(vert);
	endpolygon();
	}
closeobj();

makeobj(cone);
cf = 0.6 /* 0.3 */;
for (i=0,a=0;i<dis;i++,a+=ia)
	{
	bgnpolygon();
	norm[0] = vert[0] = cos(a);
	norm[1] = vert[1] = sin(a);
	norm[2] = vert[2] = 0.0;

	n3f(norm);
	v3f(vert);

	vert[2] = 1.0;
	vert[0] *= cf; vert[1] *= cf;
	v3f(vert);

	norm[0] = vert[0] = cos(a+ia);
	norm[1] = vert[1] = sin(a+ia);
	vert[0] *= cf;
	vert[1] *= cf;
	n3f(norm);
	v3f(vert);

	norm[0] = vert[0] = cos(a+ia);
	norm[1] = vert[1] = sin(a+ia);

	vert[2] = 0.0;
	v3f(vert);
	endpolygon();

	}

for (i=0,a=0;i<dis;i++,a+=ia)
	{
	bgnpolygon();
	norm[0] = norm[1] = 0.0;
	norm[2] = -1.0;

	vert[0] = cos(a);
	vert[1] = sin(a);
	vert[2] = 0.0;

	n3f(norm);
	v3f(vert);

	vert[0] = cos(a+ia);
	vert[1] = sin(a+ia);
	v3f(vert);

	vert[0] = vert[1] = 0.0;
	v3f(vert);
	endpolygon();

	bgnpolygon();
	norm[0] = norm[1] = 0.0;
	norm[2] = 1.0;

	vert[0] = cf*cos(a);
	vert[1] = cf*sin(a);
	vert[2] = 1.0;

	n3f(norm);
	v3f(vert);

	vert[0] = cf*cos(a+ia);
	vert[1] = cf*sin(a+ia);
	v3f(vert);

	vert[0] = vert[1] = 0.0;
	v3f(vert);
	endpolygon();
	}
closeobj();
/*----------------------*/



makeobj(sphere);
{
int i,j,k,l;
float r,ia,ib,a,b,vert[4][3],norm[4][3];
float dis1 = 12.0;
r= 1.0;
ia = PI/6;
ib = 2.0 * PI / 6;
for (i=0,a=PI;i<=6;++i,a+=ia) {
	for (j=0,b=0;j<=6;++j,b+=ib) {
		vert[3][0] = r * sin(a) * cos(b);
		vert[3][1] = r * sin(a) * sin(b);
		vert[3][2] = r * cos(a);

		vert[2][0] = r * sin(a) * cos(b + ib);
		vert[2][1] = r * sin(a) * sin(b + ib);
		vert[2][2] = r * cos(a);

		vert[1][0] = r * sin(a + ia) * cos(b + ib);
		vert[1][1] = r * sin(a + ia) * sin(b + ib);
		vert[1][2] = r * cos(a + ia);

		vert[0][0] = r * sin(a + ia) * cos(b);
		vert[0][1] = r * sin(a + ia) * sin(b);
		vert[0][2] = r * cos(a + ia);

		for (k = 0;k<=3;k++) {
			for (l=0;l<=2;++l) {
				norm[k][l] = vert[k][l] / r;
				}
			}
		bgnpolygon();
			for (k=0;k<4;k++)
				{
				n3f(norm[k]);
				v3f(vert[k]);
				}
		endpolygon();
		}
	}
}
closeobj();



/*----------------------*/
makeobj(scalpel);
norm[0] = 0.0;
norm[1] = -1.0;
norm[2] = 0.0;
bgnpolygon();
n3f(norm);
vert[0] = -1.0;
vert[1] = vert[2] = 0;

v3f(vert);
vert[0] = 0; vert[2] = 2.0;
v3f(vert);
vert[0] = 1.0; vert[2] = 0;
v3f(vert);
endpolygon();
closeobj();

makeobj(scalpel2);
norm[0] = 0.0;
norm[1] = -1.0;
norm[2] = 0.0;

bgnpolygon();
n3f(norm);
vert[0] = 0.0;
vert[1] = vert[2] = 0;

v3f(vert);
vert[0] = 1.0; vert[2] = 1.0;
v3f(vert);
vert[0] = 0.0; vert[2] = 2.0;
v3f(vert);
vert[0] = -1.0; vert[2] = 1.0;
v3f(vert);
endpolygon();
closeobj();

makeobj(scalpel3);
norm[0] = 0.0;
norm[1] = 1.0;
norm[2] = 0.0;

bgnpolygon();
n3f(norm);
vert[0] = -1.0;
vert[1] = 1.0; vert[2] = 0;

v3f(vert);
vert[1] = 0.0;
vert[0] = -1.0; vert[2] = 1.8;
v3f(vert);

vert[0] = 1.0; vert[2] = 1.35;
v3f(vert);
vert[0] = 1.0; vert[2] = 0.0;
v3f(vert);
endpolygon();
norm[0] = 0.0;
norm[1] = -1.0;
norm[2] = 0.0;


bgnpolygon();
n3f(norm);
vert[0] = -1.0;
vert[1] = -1.0; vert[2] = 0;

v3f(vert);
vert[1] = 0.0;
vert[0] = -1.0; vert[2] = 1.9;
v3f(vert);

vert[0] = 1.0; vert[2] = 1.35;
v3f(vert);
vert[0] = 1.0; vert[2] = 0.0;
v3f(vert);
endpolygon();

norm[0] = -1.0;
norm[1] = 0.0;
norm[2] = 0.0;


bgnpolygon();
n3f(norm);
vert[0] = -1.0;
vert[1] = -1.0; vert[2] = 0;

v3f(vert);
vert[1] = 0.0;
vert[0] = -1.0; vert[2] = 1.9;
v3f(vert);

vert[1] = 1.0;
vert[0] = -1.0; vert[2] = 0.0;
v3f(vert);
endpolygon();
closeobj();

}

void maketool()
{

makeobj(tool);

lmbind(MATERIAL,999);

pushmatrix();
scale(0.75,0.75,25.0);
callobj(cylinder);
popmatrix();
translate(0,0,25.0);
pushmatrix();
scale(0.6,0.6,10.7);
callobj(cone);
popmatrix();
translate(0,0,10.2);
scale(0.4,1.0,1.5);
callobj(scalpel2);

closeobj();

makeobj(syringe);
lmbind(MATERIAL,999);
pushmatrix();
scale(2.0,2.0,0.5);
callobj(cylinder);
popmatrix();

translate(0,0,0.5);
pushmatrix();
scale(0.25,0.25,5.0);
callobj(cylinder);
popmatrix();
translate(0,0,5.0);
pushmatrix();
scale(2.0,2.0,15.0);
callobj(cylinder);
popmatrix();
translate(0,0,15.0);
pushmatrix();
scale(1.5,1.5,4.0);
callobj(cylinder);
popmatrix();
translate(0,0,4.0);
pushmatrix();
scale(0.1,0.1,15.0);
callobj(cylinder);
popmatrix();
closeobj();

makeobj(light_tool);
lmbind(MATERIAL,999);
pushmatrix();
scale(1.75,1.75,40.0);
callobj(cylinder);
popmatrix();
translate(0,0,40.0);
lmbind(MATERIAL,998);
pushmatrix();
scale(1.5,1.5,2.0);
callobj(cylinder);
popmatrix();
closeobj();

makeobj(vector);
/*
move(0,0,0);
draw(0,0,1.0);
move(0,0,1.25);
draw(.25,0,1.0);
draw(-.25,0,1.0);
draw(0,0,1.25);
*/
{
int i,j;
float
polydata[10][4][3]={{{-.1,-.1,0.2},{.1,-.1,0.2},{.1,.1,0.2},{-.1,.1,0.2}},
			{{-.1,-.1,0.2},{-.1,.1,0.2},{-.1,.1,1.0},{-.1,-.1,1.0}},
			{{.1,-.1,0.2},{.1,.1,0.2},{.1,.1,1.0},{.1,-.1,1.0}},
			{{-.2,.1,1.0},{.2,.1,1.0},{.2,-.1,1.0},{-.2,-.1,1.0}},
			{{-.2,-.1,1.0},{-.2,.1,1.0},{0,.1,1.2},{0,-.1,1.2}},
			{{.2,-.1,1.0},{.2,.1,1.0},{0,.1,1.2},{0,-.1,1.2}},
			{{-.1,-.1,0.2},{.1,-.1,0.2},{.1,-.1,1.0},{-.1,-.1,1.0}},
			{{-.1,.1,0.2},{.1,.1,0.2},{.1,.1,1.0},{-.1,.1,1.0}},
			{{-.2,.1,1.0},{.2,.1,1.0},{0,.1,1.2},{0,.1,1.2}},
			{{-.2,-.1,1.0},{.2,-.1,1.0},{0,-.1,1.2},{0,-.1,1.2}}};
for (j=0;j<10;j++)
			{
if (j> 5)
	RGBcolor(200,100,0);
else
	RGBcolor(150,50,0);
bgnpolygon();
			for (i=0;i<4;i++)
			{
			v3f(polydata[j][i]);
			}
endpolygon();
			}
RGBcolor(0,0,0);
for(j=0;j<10;j++)
	{
	for(i=0;i<3;i++)
		{
		move(polydata[j][i][0],polydata[j][i][1],polydata[j][i][2]);
		draw(polydata[j][i+1][0],polydata[j][i+1][1],polydata[j][i+1][2]);
		}
	}
}
closeobj();

makeobj(stretcher);
lmbind(MATERIAL,999);
pushmatrix();
translate(-5.0,0,0);
pushmatrix();
scale(0.25,0.25,20.0);
callobj(cylinder);
popmatrix();
pushmatrix();
translate(0,0,20.0);
rotate(450,'x');

scale(0.25,0.25,0.25);
callobj(sphere);
scale(4.0,4.0,4.0);

scale(0.25,0.25,5.0);
callobj(cylinder);
scale(4.0,4.0,0.2);

translate(0,0,5.0);
rotate(450,'x');

scale(0.25,0.25,0.25);
callobj(sphere);
scale(4.0,4.0,4.0);

scale(0.25,0.25,5.0);
callobj(cylinder);

scale(4.0,4.0,0.2);
translate(0,0,5.0);
rotate(900,'y');

scale(0.25,0.25,0.25);
callobj(sphere);
scale(4.0,4.0,4.0);

popmatrix();
popmatrix();

/* 2nd limb */
pushmatrix();
translate(5.0,0,0);
pushmatrix();
scale(0.25,0.25,20.0);
callobj(cylinder);
popmatrix();
pushmatrix();
translate(0,0,20.0);
rotate(450,'x');

scale(0.25,0.25,0.25);
callobj(sphere);
scale(4.0,4.0,4.0);

scale(0.25,0.25,5.0);
callobj(cylinder);
scale(4.0,4.0,0.2);

translate(0,0,5.0);
rotate(450,'x');

scale(0.25,0.25,0.25);
callobj(sphere);
scale(4.0,4.0,4.0);

scale(0.25,0.25,5.0);
callobj(cylinder);

popmatrix();
popmatrix();

closeobj();

makeobj(fixation);
pushmatrix();
rot(90.0,'x');

lmbind(MATERIAL,999);

pushmatrix();
scale(0.75,0.75,19.0);
callobj(cylinder);
popmatrix();

pushmatrix();
translate(0,0,19.0);
scale(0.6,0.6,11.0);
callobj(cone);
popmatrix();

pushmatrix();
translate(0,0,29.25);
scale(0.5,0.5,0.75);
callobj(cylinder);
popmatrix();
pushmatrix();
translate(0,0,36.65);
rot(90.0,'z');
rot(90.0,'x');
makeannulus(6.75,6.5,0,2*PI,1.15);
{
int i;
for(i=0;i<10;i++)
	{
	rot(36.0,'z');
	pushmatrix();
	translate(6.4,0,0);
	rot(90.0,'z');
	scale(0.7,0.7,0.7);
	callobj(scalpel);
	popmatrix();
	}
popmatrix();
popmatrix();
closeobj();
}
makeobj(guarded_blade);
lmbind(MATERIAL,999);

pushmatrix();
scale(0.75,0.75,25.0);
callobj(cylinder);
popmatrix();
translate(0,0,25.0);
pushmatrix();
scale(0.75,0.75,10.0);
callobj(cone);
popmatrix();
translate(0,0,10.0);
pushmatrix();
translate(0,0,-1.0);
scale(0.6,0.6,3.0);
callobj(cylinder);
popmatrix();
pushmatrix();
translate(0,0,4.0);
scale(0.6,0.6,1.0);
callobj(cone);
popmatrix();
pushmatrix();
translate(0,0,3.0);
makeannulus(0.6,0.59,0,PI,2.0);
popmatrix();
lmbind(MATERIAL,997);
pushmatrix();
scale(0.5,0.5,3.0);
callobj(cylinder);
popmatrix();
lmbind(MATERIAL,994);
rot(90.0,'z');
translate(0,0,3.0);
scale(0.34,0.175,1.5);
callobj(scalpel3);

closeobj();



makeobj(robot_7dof);
pushmatrix();
lmbind(MATERIAL,994);
translate(0,0,8);
scale(0.5,0.5,0.5);
scale(0.34,0.175,1.5);
callobj(scalpel3);
popmatrix();

pushmatrix();
pushmatrix();
pushmatrix();
scale(0.75,0.75,0.75);
callobj(sphere);
popmatrix();
translate(0,0,-8.5);
pushmatrix();
scale(0.75,0.75,0.75);
callobj(sphere);
popmatrix();
popmatrix();
translate(0,0,-10.0);
scale(0.5,0.5,0.5);
callobj(tool);
popmatrix();

lmbind(MATERIAL,999);
pushmatrix();
rot(150.0,'x');
pushmatrix();
scale(0.4,0.4,56.0);
callobj(cylinder);
popmatrix();
pushmatrix();
pushmatrix();
lmbind(MATERIAL,995);
translate(0,0,45.0);
scale(4,4,5.0);
callobj(cylinder);
popmatrix();
translate(0,0,50.5);
scale(4,4,5.0);
callobj(cylinder);
lmbind(MATERIAL,999);
popmatrix();
translate(0,0,7);
rot(45.0,'z');
rot(90.0,'x');
pushmatrix();
translate(0,0,.3);
scale(0.3,0.3,0.3);
callobj(sphere);
popmatrix();
pushmatrix();
scale(0.2,0.2,5.0);
callobj(cylinder);
popmatrix();
translate(0,0,5.0);
pushmatrix();
scale(0.3,0.3,0.3);
callobj(sphere);
popmatrix();
rot(-90.0,'z');
rot(120.0,'x');
scale(0.2,0.2,4.0);
callobj(cylinder);
popmatrix();

pushmatrix();
rot(120.0,'z');
rot(150.0,'x');
pushmatrix();
scale(0.4,0.4,56.0);
callobj(cylinder);
popmatrix();
pushmatrix();
pushmatrix();
lmbind(MATERIAL,995);
translate(0,0,45.0);
scale(4,4,5.0);
callobj(cylinder);
popmatrix();
translate(0,0,50.5);
scale(4,4,5.0);
callobj(cylinder);
lmbind(MATERIAL,999);
popmatrix();
translate(0,0,7);
rot(45.0,'z');
rot(90.0,'x');
pushmatrix();
translate(0,0,.3);
scale(0.3,0.3,0.3);
callobj(sphere);
popmatrix();
pushmatrix();
scale(0.2,0.2,5.0);
callobj(cylinder);
popmatrix();
translate(0,0,5.0);
pushmatrix();
scale(0.3,0.3,0.3);
callobj(sphere);
popmatrix();
rot(-90.0,'z');
rot(120.0,'x');
scale(0.2,0.2,4.0);
callobj(cylinder);
popmatrix();


pushmatrix();
rot(240,'z');
rot(150.0,'x');
pushmatrix();
scale(0.4,0.4,56.0);
callobj(cylinder);
popmatrix();
pushmatrix();
pushmatrix();
lmbind(MATERIAL,995);
translate(0,0,45.0);
scale(4,4,5.0);
callobj(cylinder);
popmatrix();
translate(0,0,50.5);
scale(4,4,5.0);
callobj(cylinder);
lmbind(MATERIAL,999);
popmatrix();
translate(0,0,7);
rot(45.0,'z');
rot(90.0,'x');
pushmatrix();
translate(0,0,.3);
scale(0.3,0.3,0.3);
callobj(sphere);
popmatrix();
pushmatrix();
scale(0.2,0.2,5.0);
callobj(cylinder);
popmatrix();
translate(0,0,5.0);
pushmatrix();
scale(0.3,0.3,0.3);
callobj(sphere);
popmatrix();
rot(-90.0,'z');
rot(120.0,'x');
scale(0.2,0.2,4.0);
callobj(cylinder);
popmatrix();
closeobj();


}
