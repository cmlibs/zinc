/*******************************************************************************
FILE : animation_window.c

LAST MODIFIED : 23 November 2001

DESCRIPTION :
Management routines for the animation command window.
==============================================================================*/
#include <stdlib.h>
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/List.h>
#include <Xm/Protocols.h>
#include <Xm/TextF.h>

#include "general/debug.h"
#include "graphics/animation_window.h"
#include "graphics/animation_window.uidh"
#include "graphics/graphics_window.h"
#include "graphics/graphics_library.h"
#include "graphics/scene.h"
#include "graphics/splines.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int animation_window_hierarchy_open=0;
static MrmHierarchy animation_window_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static void animation_list_points(struct Animation_window *animation_window)
/*******************************************************************************
LAST MODIFIED : 29 January 1996

DESCRIPTION :
Goes through list printing each points values
==============================================================================*/
{
struct Animation_Point_List_Item *ptr;

ptr = animation_window->animation_point_list;
while(ptr != NULL)
	{
	printf("Point# %.2f    eye:(%.3f,%.3f,%.3f)  poi:(%.3f,%.3f,%.3f)  fov:(%.3f) time:(%.3f) \n",
		ptr->point_number,
		ptr->eye[0],ptr->eye[1],ptr->eye[2],
		ptr->poi[0],ptr->poi[1],ptr->poi[2],
		ptr->fov,ptr->time
		);

	ptr = ptr->next;
	}
}

static void animation_add_point_to_list(
	struct Animation_window *animation_window)
/*******************************************************************************
LAST MODIFIED : 29 January 1996

DESCRIPTION :
Looks through animation point list and inserts or appends latest point.  If
point index already exists, the existing point values are replaced.
==============================================================================*/
{
char *string_ptr;
int i,insert_flag=0,replace_flag=0;
struct Animation_Point_List_Item *new_point,*ptr;

new_point = NULL;
XtVaGetValues(animation_window->animation_text_fields[6],XmNvalue,&string_ptr,NULL);
animation_window->point_index =  atof(string_ptr);

/* if point index matches, replace existing point values */
ptr = animation_window->animation_point_list;
while (ptr != NULL)
	{
	if (ptr->point_number == animation_window->point_index)
		{
		new_point = ptr;
		replace_flag = 1;
		printf("replacing point\n");
		break;
		}
	ptr = ptr->next;
	}

/* if not replaced, create new point */
if (!replace_flag)
	{
	/*???DB.  Check */
	ALLOCATE(new_point,struct Animation_Point_List_Item,1);
	}
for (i=0;i<3;i++)
	{
	XtVaGetValues(animation_window->animation_text_fields[i],XmNvalue,&string_ptr,NULL);
	new_point->eye[i] = (double) atof(string_ptr);
	XtVaGetValues(animation_window->animation_text_fields[i+3],XmNvalue,&string_ptr,NULL);
	new_point->poi[i] = (double) atof(string_ptr);
	}
XtVaGetValues(animation_window->animation_text_fields[7],XmNvalue,&string_ptr,NULL);
new_point->fov = (double) atof(string_ptr);
XtVaGetValues(animation_window->animation_text_fields[8],XmNvalue,&string_ptr,NULL);
new_point->time = (double) atof(string_ptr);
new_point->point_number = animation_window->point_index;
if (!replace_flag)
	{
	new_point->next = NULL;
	}

/* if we have replaced point ,leave routine now */
if (replace_flag)
	{
	return;
	}

/* go through list and insert or add point */

if (animation_window->animation_point_list == NULL)
	{
	animation_window->animation_point_list = new_point;
	}
else
	{
	insert_flag = 0;
	ptr = animation_window->animation_point_list;

	if (ptr->point_number > new_point->point_number)
		{
		/* insert before start of list */
		new_point->next = ptr;
		animation_window->animation_point_list = new_point;
		insert_flag = 1;
		}
	while(ptr->next != NULL && !insert_flag)
		{
		if ((ptr->next->point_number) > new_point->point_number)
			{
			/*insert point if next pt has higher number*/
			new_point->next = ptr->next;
			ptr->next = new_point;
			insert_flag = 1;
			break;
			}
		ptr=ptr->next;
		}
	if (!insert_flag)
		{
		/* must be at end of list */
		ptr->next = new_point;
		}
	}
}

static void animation_remove_point_from_list(
	struct Animation_window *animation_window)
/*******************************************************************************
LAST MODIFIED : 29 January 1996

DESCRIPTION :
Looks through animation point list & removes current point then resets current
point to previous point
==============================================================================*/
{
char string_number[10];
struct Animation_Point_List_Item *ptr,*ptrlast;
int i;
ptrlast = ptr = animation_window->animation_point_list;
if (ptr != NULL)
	{
	if (ptr->point_number == animation_window->point_index)
		{
		/* point to remove is the head of the list */
		animation_window->animation_point_list  = ptr->next;
		DEALLOCATE(ptr);
		/* set current point index to zero */
		animation_window->point_index = 0.0;
		}
	else
		{
		while(ptr != NULL)
			{
			if (ptr->point_number == animation_window->point_index)
				{
				/* remove this point */
				ptrlast->next = ptr->next;

				/* move back one point & update values */
				if (ptrlast != NULL)
					{
					animation_window->point_index = ptrlast->point_number;
					for (i=0;i<3;i++)
						{
						sprintf(string_number,"%.3f",ptrlast->eye[i]);
						XtVaSetValues(animation_window->animation_text_fields[i],XmNvalue,string_number,NULL);
						sprintf(string_number,"%.3f",ptrlast->poi[i]);
						XtVaSetValues(animation_window->animation_text_fields[i+3],XmNvalue,string_number,NULL);
						}
					sprintf(string_number,"%.3f",ptrlast->fov);
					XtVaSetValues(animation_window->animation_text_fields[7],
							XmNvalue,string_number,NULL);

					sprintf(string_number,"%.2f",animation_window->point_index);
					XtVaSetValues(animation_window->animation_text_fields[6],
						XmNvalue,string_number,NULL);


					sprintf(string_number,"%.2f",ptrlast->time);
					XtVaSetValues(animation_window->animation_text_fields[8],
						XmNvalue,string_number,NULL);

					}

				DEALLOCATE(ptr);
				break;
				}
			else
				{
				ptrlast = ptr;
				ptr = ptr->next;
				}
			}
		}
	}
}

static void animation_produce_spline_path(
	struct Animation_window *animation_window)
/*******************************************************************************
LAST MODIFIED : 29 January 1996

DESCRIPTION :
Fills or updates camera eye & poi spline curves stored in animation window (the
curves generated have n_points)
==============================================================================*/
{
int i,count = 0;
struct Animation_Point_List_Item *ptr;
double *px,*py,*pz,*pu,*pfov;
int n_points;

n_points = animation_window->n_frames;

printf("in animation produce spline path\n");
if (animation_window->eye_spline_x)
	DEALLOCATE(animation_window->eye_spline_x);
if (animation_window->eye_spline_y)
	DEALLOCATE(animation_window->eye_spline_y);
if (animation_window->eye_spline_z)
	DEALLOCATE(animation_window->eye_spline_z);
if (animation_window->poi_spline_x)
	DEALLOCATE(animation_window->poi_spline_x);
if (animation_window->poi_spline_y)
	DEALLOCATE(animation_window->poi_spline_y);
if (animation_window->poi_spline_z)
	DEALLOCATE(animation_window->poi_spline_z);
if (animation_window->spline_knots)
	DEALLOCATE(animation_window->spline_knots);
if (animation_window->fov_spline)
	DEALLOCATE(animation_window->fov_spline);

ptr = animation_window->animation_point_list;
while (ptr != NULL)
	{
	count++;
	ptr= ptr->next;
	}
/*???DB.  Check */
ALLOCATE(px,double,count+2);
ALLOCATE(py,double,count+2);
ALLOCATE(pz,double,count+2);
ALLOCATE(pu,double,count+4);
ALLOCATE(pfov,double,count+4);
ALLOCATE(animation_window->eye_spline_x,double,n_points+2);
ALLOCATE(animation_window->eye_spline_y,double,n_points+2);
ALLOCATE(animation_window->eye_spline_z,double,n_points+2);
ALLOCATE(animation_window->poi_spline_x,double,n_points+2);
ALLOCATE(animation_window->poi_spline_y,double,n_points+2);
ALLOCATE(animation_window->poi_spline_z,double,n_points+2);
ALLOCATE(animation_window->fov_spline,double,n_points+2);
ALLOCATE(animation_window->spline_knots,double,count+4);

animation_window->n_spline_points = n_points;
if (count)
	{
	i =0;
	ptr = animation_window->animation_point_list;
	while (ptr != NULL)
		{
		px[i] = ptr->eye[0];
		py[i] = ptr->eye[1];
		pz[i] = ptr->eye[2];
		pfov[i] = ptr->fov;
		animation_window->spline_knots[i] = pu[i] = ptr->time;
		ptr = ptr->next;
		i++;
		}
	printf("apsp b4 interpolate spline\n");

	interpolate_spline(count,px,pu,n_points,
		animation_window->eye_spline_x);
	interpolate_spline(count,py,pu,n_points,
		animation_window->eye_spline_y);
	interpolate_spline(count,pz,pu,n_points,
		animation_window->eye_spline_z);

	printf("apsp after interpolate spline\n");

	i =0;
	ptr = animation_window->animation_point_list;
	while (ptr != NULL)
		{
		px[i] = ptr->poi[0];
		py[i] = ptr->poi[1];
		pz[i] = ptr->poi[2];
		pu[i] = ptr->time;
		ptr = ptr->next;
		i++;
		}

	interpolate_spline(count,px,pu,n_points,
		animation_window->poi_spline_x);
	interpolate_spline(count,py,pu,n_points,
		animation_window->poi_spline_y);
	interpolate_spline(count,pz,pu,n_points,
		animation_window->poi_spline_z);

	interpolate_spline(count,pfov,pu,n_points,
		animation_window->fov_spline);

	DEALLOCATE(px);
	DEALLOCATE(py);
	DEALLOCATE(pz);
	DEALLOCATE(pu);
	DEALLOCATE(pfov);
	}
	printf("apsp leaving\n");

}

/*
Global functions
----------------
*/
struct Animation_window *create_animation_window(
	struct Graphics_window *graphics_window,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
Create the structures and retrieve the command window from the uil file.
==============================================================================*/
{
	int i;
	struct Animation_window *animation_window;
	MrmType animation_window_class;
	static MrmRegisterArg callback_list[] = {
		{ "animation_inc_point", (XtPointer)animation_inc_point },
		{ "animation_dec_point", (XtPointer)animation_dec_point },
		{ "animation_close", (XtPointer)animation_close},
		{ "animation_accept", (XtPointer)animation_accept},
		{ "animation_create", (XtPointer)animation_create},
		{ "animation_clear", (XtPointer)animation_clear},
		{ "animation_load", (XtPointer)animation_load},
		{ "animation_text_field_created", (XtPointer)animation_text_field_created},
		{ "animation_text_value_changed", (XtPointer)animation_text_value_changed}
	};
	static MrmRegisterArg identifiers[] =
	{
		{"animation_window_pointer",(XtPointer)0}
	};

	ENTER(create_animation_window);
	/* check arguments */
	if (user_interface)
	{
		if (MrmOpenHierarchy_base64_string(animation_window_uidh,
			&animation_window_hierarchy,&animation_window_hierarchy_open))
		{
			if (ALLOCATE(animation_window, struct Animation_window, 1))
			{
				animation_window->app_shell = (Widget) NULL;
				animation_window->window_shell = (Widget) NULL;
				animation_window->main_window = (Widget) NULL;
				animation_window->owner = graphics_window;
				animation_window->animation_point_list = NULL;
				animation_window->point_index = 0;
				animation_window->eye_spline_x = NULL;
				animation_window->eye_spline_y = NULL;
				animation_window->eye_spline_z = NULL;
				animation_window->poi_spline_x = NULL;
				animation_window->poi_spline_y = NULL;
				animation_window->poi_spline_z = NULL;
				animation_window->spline_knots = NULL;
				animation_window->fov_spline = NULL;
				animation_window->n_spline_points = 0;
				animation_window->n_frames = 0;
				for (i = 0;i< N_TEXT_FIELDS;i++)
				{
					animation_window->animation_text_fields[i] = NULL;
				}
				if (animation_window->window_shell=XtVaCreatePopupShell(
					"animation_window_shell",transientShellWidgetClass,
					user_interface->application_shell,
					XmNmwmDecorations, MWM_DECOR_ALL|MWM_DECOR_MAXIMIZE,
					XmNmwmFunctions, MWM_FUNC_ALL|MWM_FUNC_MAXIMIZE,
					XmNtransient, FALSE,
					NULL))
				{
					/* Add destroy callback */
					XtAddCallback(animation_window->window_shell, XmNdestroyCallback,
						destroy_animation_window_callback, (XtPointer)animation_window);
					XmInternAtom(XtDisplay(animation_window->window_shell),
						"WM_DELETE_WINDOW",FALSE);
					/* pass the animation_window_pointer to the uid stuff */
					/* Register callbacks in UIL */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						animation_window_hierarchy,callback_list,XtNumber(callback_list)))
					{
						/* pass the animation_window_pointer to the uid stuff */
						identifiers[0].value = (XtPointer)animation_window;
						if (MrmSUCCESS!=MrmRegisterNamesInHierarchy(
							animation_window_hierarchy,identifiers,XtNumber(identifiers)))
						{
							display_message(ERROR_MESSAGE,"Could not register names");
							DEALLOCATE(animation_window);
						}
						else
						{
							if (MrmSUCCESS==MrmFetchWidget(animation_window_hierarchy,
								"animation_window",animation_window->window_shell,
								&(animation_window->main_window), &animation_window_class))
							{
								XtManageChild(animation_window->main_window);
								XtRealizeWidget(animation_window->window_shell);
								XtPopup(animation_window->window_shell, XtGrabNone);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Could not retrieve animation widget");
								DEALLOCATE(animation_window);
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE, "Could not register callbacks");
						DEALLOCATE(animation_window);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "Could not create shell");
					DEALLOCATE(animation_window);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"create_animation_window.  Insufficient memory");
				animation_window = (struct Animation_window *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Could not open hierarchy");
			animation_window = (struct Animation_window *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_animation_window.  Invalid argument(s)");
		animation_window = (struct Animation_window *)NULL;
	}
	LEAVE;

	return ((void *)animation_window);
} /* create_animation_window */

void animation_close(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
Closes and destroys the window pointed to by the window.  After this the window
must be recreated.  The routine relies on close_help_window_callback to actually
dispose of the window description structure and call the destroy function (if
necessary).
==============================================================================*/
{
	struct Animation_window *animation_window;
	struct Animation_Point_List_Item *ptr,*freeptr;

	ENTER(animation_close);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	animation_window = (struct Animation_window *) client_data;
	printf("in close_animation_window\n");
	XtPopdown(animation_window->window_shell);
	XtDestroyWidget(animation_window->window_shell);
#if defined (OLD_GFX_WINDOW)
	((struct Graphics_window *)animation_window->owner)->animation_window = NULL;
#endif /* defined (OLD_GFX_WINDOW) */

	ptr = animation_window->animation_point_list;
	while (ptr!=NULL)
	{
		freeptr = ptr;
		ptr = ptr->next;
		DEALLOCATE(freeptr);
	}
	DEALLOCATE(animation_window);
	LEAVE;
} /* animation_close */


void destroy_animation_window_callback(
	Widget    caller,      /*Widget sending the callback.  */
	XtPointer  the_window,    /*Window description structure. */
	XtPointer  caller_data    /*Callback data.    */
	)
/*******************************************************************************
LAST MODIFIED : 6 October 1993

DESCRIPTION :
Callback for when the window is closed and destroyed.  Calls the destroy_func
if necessary and disposes of the window description structure.
==============================================================================*/
{
	ENTER(destroy_animation_window_callback);
	USE_PARAMETER(caller);
	USE_PARAMETER(caller_data);
	printf("destroying seat_window\n");
	DEALLOCATE(the_window);

	LEAVE;
}/* destroy_animation_window_callback */


void animation_dec_point(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Decrements current point index (steps back one list item)
==============================================================================*/
{
	struct Animation_window *animation_window;
	int i;
	char string_number[10];
	struct Animation_Point_List_Item *ptr,*ptrlast;

	USE_PARAMETER(w);
	USE_PARAMETER(cbs);

	animation_window = (struct Animation_window *)client_data;

	ptrlast = ptr = animation_window->animation_point_list;
	while(ptr != NULL)
	{

		if (ptr->point_number >= animation_window->point_index)
		{
			animation_window->point_index = ptrlast->point_number;
			break;
		}
		else
		{
			ptrlast = ptr;
			ptr = ptr->next;
		}
	}
	if (ptrlast != NULL)
	{
		for (i=0;i<3;i++)
		{
			sprintf(string_number,"%.3f",ptrlast->eye[i]);
			XtVaSetValues(animation_window->animation_text_fields[i],XmNvalue,string_number,NULL);
			sprintf(string_number,"%.3f",ptrlast->poi[i]);
			XtVaSetValues(animation_window->animation_text_fields[i+3],XmNvalue,string_number,NULL);
			sprintf(string_number,"%.3f",ptrlast->time);
			XtVaSetValues(animation_window->animation_text_fields[8],XmNvalue,string_number,NULL);
		}
		sprintf(string_number,"%.3f",ptrlast->fov);
		XtVaSetValues(animation_window->animation_text_fields[7],XmNvalue,string_number,NULL);
	}
	sprintf(string_number,"%.2f",animation_window->point_index);
	XtVaSetValues(animation_window->animation_text_fields[6],XmNvalue,string_number,NULL);

	printf("point_index = %f\n",animation_window->point_index);
#if defined (OLD_GFX_WINDOW)
	drawing_changed((struct Drawing *) ((struct Graphics_window *)animation_window->owner)->drawing);
#endif /* defined (OLD_GFX_WINDOW) */
}

void animation_inc_point(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 3 November 1995

DESCRIPTION :
Increments current point index (steps forward one list item)
==============================================================================*/
{
	char string_number[10];
	struct Animation_Point_List_Item *ptr;
	int i;
	struct Animation_window *animation_window;

	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	animation_window = (struct Animation_window *)client_data;
	ptr = animation_window->animation_point_list;
	while(ptr != NULL)
	{
		if (ptr->point_number > animation_window->point_index)
		{
			animation_window->point_index = ptr->point_number;
			break;
		}
		ptr = ptr->next;
	}
	if (ptr != NULL)
	{
		/* update text fields */
		for (i=0;i<3;i++)
		{
			sprintf(string_number,"%.3f",ptr->eye[i]);
			XtVaSetValues(animation_window->animation_text_fields[i],XmNvalue,string_number,NULL);
			sprintf(string_number,"%.3f",ptr->poi[i]);
			XtVaSetValues(animation_window->animation_text_fields[i+3],XmNvalue,string_number,NULL);
		}
		sprintf(string_number,"%.3f",ptr->fov);
		XtVaSetValues(animation_window->animation_text_fields[7],XmNvalue,string_number,NULL);
		sprintf(string_number,"%.3f",ptr->time);
		XtVaSetValues(animation_window->animation_text_fields[8],XmNvalue,string_number,NULL);

		sprintf(string_number,"%.2f",animation_window->point_index);
		XtVaSetValues(animation_window->animation_text_fields[6],XmNvalue,string_number,NULL);
	}
	printf("point_index = %f\n",animation_window->point_index);
#if defined (OLD_GFX_WINDOW)
	drawing_changed((struct Drawing *) ((struct Graphics_window *)animation_window->owner)->drawing);
#endif /* defined (OLD_GFX_WINDOW) */
}


void animation_text_field_created(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 7 October 1993

DESCRIPTION :
Identifies text fields
==============================================================================*/
{
	char string_number[10];
	struct Animation_window *animation_window;
	int index;

	USE_PARAMETER(cbs);
	animation_window = (struct Animation_window *)client_data;
	XtVaGetValues(w,XmNuserData,&index,NULL);
	animation_window->animation_text_fields[index] = w;
	sprintf(string_number,"%d",0);
	XtVaSetValues(w,XmNvalue,string_number,NULL);

}

void animation_text_value_changed(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Updates field data for point
==============================================================================*/
{
	struct Animation_window *animation_window;
	int index,i;
	float point_index;
	char *string_ptr,string_number[10];
	struct Animation_Point_List_Item *ptr,*found_ptr = NULL;

	USE_PARAMETER(cbs);

	animation_window = (struct Animation_window *)client_data;
	XtVaGetValues(w,XmNuserData,&index,NULL);

	XtVaGetValues(w,XmNvalue,&string_ptr,NULL);
	printf("field(%d) = %f\n",index,atof(string_ptr));

	ptr = animation_window->animation_point_list;
	if (index == 6)
	{
		/* specific point index entered */
		point_index = atof(string_ptr);
		animation_window->point_index = point_index;
		while (ptr != NULL)
		{
			if (ptr->point_number == point_index)
			{
				found_ptr = ptr;
				break;
			}
			ptr = ptr->next;
		}

		if (found_ptr != NULL)
		{
			for (i=0;i<3;i++)
			{
				sprintf(string_number,"%.3f",found_ptr->eye[i]);
				XtVaSetValues(animation_window->animation_text_fields[i],XmNvalue,string_number,NULL);
				sprintf(string_number,"%.3f",found_ptr->poi[i]);
				XtVaSetValues(animation_window->animation_text_fields[i+3],XmNvalue,string_number,NULL);
			}
			sprintf(string_number,"%.3f",found_ptr->fov);
			XtVaSetValues(animation_window->animation_text_fields[7],XmNvalue,string_number,NULL);
			sprintf(string_number,"%.3f",found_ptr->time);
			XtVaSetValues(animation_window->animation_text_fields[8],XmNvalue,string_number,NULL);

		}
	}
#if defined (OLD_GFX_WINDOW)
	drawing_changed((struct Drawing *) ((struct Graphics_window *)animation_window->owner)->drawing);
#endif /* defined (OLD_GFX_WINDOW) */
}

void animation_accept(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Accepts current point into list
==============================================================================*/
{
	struct Animation_window *animation_window;

	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	animation_window = (struct Animation_window *)client_data;
	printf("accept called\n");
	animation_add_point_to_list(animation_window);
#if defined (OLD_GFX_WINDOW)
	drawing_changed((struct Drawing *) ((struct Graphics_window *)animation_window->owner)->drawing);
#endif /* defined (OLD_GFX_WINDOW) */

}

void animation_clear(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 2 November 1993

DESCRIPTION :
Removes current point
==============================================================================*/
{
	struct Animation_window *animation_window;

	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	animation_window = (struct Animation_window *)client_data;
	/* remove point and set index back one */
	animation_remove_point_from_list(animation_window);
	printf("clear called\n");
#if defined (OLD_GFX_WINDOW)
	drawing_changed((struct Drawing *) ((struct Graphics_window *)animation_window->owner)->drawing);
#endif /* defined (OLD_GFX_WINDOW) */
}

void animation_create(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Writes animation file
==============================================================================*/
{
	struct Animation_window *animation_window;
	struct Animation_Point_List_Item *ptr;
	int i,cnt=0;
	FILE *fp,*fq;
	double maxtime=0;

	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	animation_window = (struct Animation_window *)client_data;

	printf("create called\n");
	fq = fopen("animation.points","w");

	ptr = animation_window->animation_point_list;
	while (ptr !=0)
	{
		cnt++;
		if (ptr->time > maxtime)
			maxtime = ptr->time;
		printf("1: ptr->point_number = %f, eye_point = %f,%f,%f poi = %f,%f,%f, time = %f\n",
			ptr->point_number,ptr->eye[0],ptr->eye[1],ptr->eye[2],
			ptr->poi[0],ptr->poi[1],ptr->poi[2],ptr->time);
		fprintf(fq,"%lf\n%lf %lf %lf %lf %lf %lf\n%lf %lf\n",ptr->point_number,
			ptr->eye[0],ptr->eye[1],ptr->eye[2],ptr->poi[0],ptr->poi[1],ptr->poi[2],
			ptr->fov,ptr->time);
		ptr=ptr->next;
	}

	fclose(fq);

	/* work out number of frames each 1/25 of a second to fit the whole time  */
	animation_window->n_frames = (int) (maxtime * 25.0);
	printf("# of frames to be generated: %d frames for %f seconds\n",animation_window->n_frames,maxtime);

	fp = fopen("animation.com","w");
	fprintf(fp,"gfx set video\n");

	animation_list_points(animation_window);
	if (cnt > 1)
	{
		animation_produce_spline_path(animation_window);

		for (i=0;i<=animation_window->n_frames;i++)
		{
			printf("spline pt[%d] : eye(%.2f,%.2f,%.2f)  poi(%.2f,%.2f,%.2f) fov(%f)\n",i,
				animation_window->eye_spline_x[i],animation_window->eye_spline_y[i],animation_window->eye_spline_z[i],
				animation_window->poi_spline_x[i],animation_window->poi_spline_y[i],animation_window->poi_spline_z[i],
				animation_window->fov_spline[i]);
			fprintf(fp,"gfx set view_point %lf %lf %lf\n",
				animation_window->eye_spline_x[i],
				animation_window->eye_spline_y[i],
				animation_window->eye_spline_z[i]);
			fprintf(fp,"gfx set interest_point %lf %lf %lf\n",
				animation_window->poi_spline_x[i],
				animation_window->poi_spline_y[i],
				animation_window->poi_spline_z[i]);
			fprintf(fp,"gfx set view_angle %lf\n",animation_window->fov_spline[i]);
			fprintf(fp,"gfx grab_frame\n");
		}
	}
	else
	{
		animation_window->n_spline_points = 0;
	}

	fprintf(fp,"gfx set video\n");
	fclose(fp);

	ptr = animation_window->animation_point_list;
	while (ptr !=0)
	{

		printf("2: ptr->point_number = %f, eye_point = %f,%f,%f poi = %f,%f,%f, time = %f\n",
			ptr->point_number,ptr->eye[0],ptr->eye[1],ptr->eye[2],
			ptr->poi[0],ptr->poi[1],ptr->poi[2],ptr->time);
		ptr=ptr->next;
	}

#if defined (OLD_GFX_WINDOW)
	drawing_changed((struct Drawing *) ((struct Graphics_window *)animation_window->owner)->drawing);
#endif /* defined (OLD_GFX_WINDOW) */
}

void animation_load(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Loads animation.points file
==============================================================================*/
{
	struct Animation_window *animation_window;
	struct Animation_Point_List_Item *ptr;
	double number,end;
	char string_number[10];
	FILE *fp;

	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	animation_window = (struct Animation_window *)client_data;

	printf("loading file animation.points\n");
	/* clear existing data */

	ptr = animation_window->animation_point_list;
	while (ptr != NULL)
	{
		end = ptr->point_number;
		ptr = ptr->next;
	}
	animation_window->point_index = end;
	while (animation_window->animation_point_list)
	{
		animation_remove_point_from_list(animation_window);
	}

	if (fp = fopen("animation.points","r"))
	{
		animation_window->point_index = 0;
		while (fscanf(fp,"%lf",&number) != EOF)
		{
			sprintf(string_number,"%.3f",number);
			XtVaSetValues(animation_window->animation_text_fields[6],XmNvalue,string_number,NULL);
			printf("adding point %f to list\n",number);

			fscanf(fp,"%lf",&number);
			sprintf(string_number,"%.3f",number);
			XtVaSetValues(animation_window->animation_text_fields[0],XmNvalue,string_number,NULL);

			fscanf(fp,"%lf",&number);
			sprintf(string_number,"%.3f",number);
			XtVaSetValues(animation_window->animation_text_fields[1],XmNvalue,string_number,NULL);

			fscanf(fp,"%lf",&number);
			sprintf(string_number,"%.3f",number);
			XtVaSetValues(animation_window->animation_text_fields[2],XmNvalue,string_number,NULL);

			fscanf(fp,"%lf",&number);
			sprintf(string_number,"%.3f",number);
			XtVaSetValues(animation_window->animation_text_fields[3],XmNvalue,string_number,NULL);

			fscanf(fp,"%lf",&number);
			sprintf(string_number,"%.3f",number);
			XtVaSetValues(animation_window->animation_text_fields[4],XmNvalue,string_number,NULL);

			fscanf(fp,"%lf",&number);
			sprintf(string_number,"%.3f",number);
			XtVaSetValues(animation_window->animation_text_fields[5],XmNvalue,string_number,NULL);

			fscanf(fp,"%lf",&number);
			sprintf(string_number,"%.3f",number);
			XtVaSetValues(animation_window->animation_text_fields[7],XmNvalue,string_number,NULL);

			fscanf(fp,"%lf",&number);
			sprintf(string_number,"%.3f",number);
			XtVaSetValues(animation_window->animation_text_fields[8],XmNvalue,string_number,NULL);

			animation_add_point_to_list(animation_window);

		}

	}
	else
	{
		printf("ERROR : Couldn't open animation.points\n");
	}
	fclose(fp);
#if defined (OLD_GFX_WINDOW)
	drawing_changed((struct Drawing *) ((struct Graphics_window *)animation_window->owner)->drawing);
#endif /* defined (OLD_GFX_WINDOW) */
}
