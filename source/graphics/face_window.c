/*******************************************************************************
FILE : face_window.c

LAST MODIFIED : 13 December 1996

DESCRIPTION :
Management routines for the main command window.
==============================================================================*/
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/List.h>
#include <Xm/Protocols.h>
#include "general/debug.h"
#include "graphics/face_window.h"
#include "graphics/face_window.uidh"
#include "graphics/graphics_library.h"
#include "graphics/graphics_window.h"
#include "graphics/scene.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int face_window_hierarchy_open=0;
static MrmHierarchy face_window_hierarchy;
#endif /* defined (MOTIF) */

/*
Global functions
----------------
*/
void *create_application_window(struct Graphics_window *graphics_window,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
Create the structures and retrieve the command window from the uil file.
==============================================================================*/
{
	struct Face_window *face_window;
	MrmType face_window_class;
	static MrmRegisterArg callback_list[] =
	{
		{ "facebtn1CB", (XtPointer)facebtn1CB },
		{ "close_faceCB", (XtPointer)close_faceCB},
		{ "faceslider1CB", (XtPointer)faceslider1CB},
		{ "faceslider2CB", (XtPointer)faceslider2CB},
		{ "faceslider3CB", (XtPointer)faceslider3CB }
	};
	static MrmRegisterArg identifiers[] =
	{
		{"face_window_pointer",(XtPointer)0}
	};

	ENTER(create_application_window);
	/* check arguments */
	if (user_interface)
	{
		if (MrmOpenHierarchy_base64_string(face_window_uidh,
			&face_window_hierarchy,&face_window_hierarchy_open))
		{
			if (ALLOCATE(face_window, struct Face_window, 1))
			{
				face_window->app_shell = (Widget) NULL;
				face_window->window_shell = (Widget) NULL;
				face_window->main_window = (Widget) NULL;
				face_window->btn1 = (Widget) NULL;
				face_window->btn2 = (Widget) NULL;
				face_window->slider1 = (Widget) NULL;
				face_window->slider2 = (Widget) NULL;
				face_window->slider3 = (Widget) NULL;
				face_window->owner = graphics_window;
				if (face_window->window_shell=XtVaCreatePopupShell("face_window_shell",
					transientShellWidgetClass,user_interface->application_shell,
					XmNmwmDecorations, MWM_DECOR_ALL|MWM_DECOR_MAXIMIZE,
					XmNmwmFunctions, MWM_FUNC_ALL|MWM_FUNC_MAXIMIZE,
					XmNtransient, FALSE,
					NULL))
				{
					/* Add destroy callback */
					XtAddCallback(face_window->window_shell, XmNdestroyCallback,
						destroy_face_window_callback, (XtPointer)face_window);
					XmInternAtom(XtDisplay(face_window->window_shell),
						"WM_DELETE_WINDOW",FALSE);
					/* Register callbacks in UIL */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(face_window_hierarchy,
						callback_list, XtNumber(callback_list)))
					{
						/* pass the face_window_pointer to the uid stuff */
						identifiers[0].value = (XtPointer)face_window;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(face_window_hierarchy,
							identifiers,XtNumber(identifiers)))
						{
							if (MrmSUCCESS==MrmFetchWidget(face_window_hierarchy,
								"face_window",face_window->window_shell,
								&(face_window->main_window),&face_window_class))
							{
								XtManageChild(face_window->main_window);
								XtRealizeWidget(face_window->window_shell);
								XtPopup(face_window->window_shell, XtGrabNone);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_face_window.  Could not retrieve face widget");
								DEALLOCATE(face_window);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_face_window.  Could not register names");
							DEALLOCATE(face_window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_face_window.  Could not register callbacks");
						DEALLOCATE(face_window);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_face_window.  Could not create shell");
					DEALLOCATE(face_window);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_face_window.  Could not allocate face window");
				face_window = (struct Face_window *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_face_window.  Could not open hierarchy");
			face_window = (struct Face_window *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_face_window.  Invalid argument(s)");
		face_window = (struct Face_window *)NULL;
	}
	LEAVE;

	return ((void *)face_window);
} /* create_face_window */

void close_face_window(
	struct Face_window  *the_window    /*Window to close and destroy.  */
	)
/*********************************************************************************
DESCRIPTION:
		Closes and destroys the window pointed to by the window.  After this the
		window must be recreated.  The routine relies on close_help_window_callback
		to actually dispose of the window description structure and call the
		destroy function (if necessary).
================================================================================*/
{
	ENTER(close_face_window);

printf("in close_face_window\n");
	XtPopdown(the_window->window_shell);
	XtDestroyWidget(the_window->window_shell);

	LEAVE;
} /*close_face_window*/


void destroy_face_window_callback(
	Widget    caller,      /*Widget sending the callback.  */
	XtPointer  the_window,    /*Window description structure. */
	XtPointer  caller_data    /*Callback data.    */
	)
/*********************************************************************************
DESCRIPTION:
		Callback for when the window is closed and destroyed.  Calls the destroy_func
		if necessary and disposes of the window description structure.
================================================================================*/
{
	ENTER(destroy_face_window_callback);

	printf("destroying face_window\n");
	DEALLOCATE(the_window);

	LEAVE;
}/*destroy_face_window_callback*/


void close_faceCB(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
{
printf("close_faceCB called\n");
close_face_window((struct Face_window *) client_data);
}

void facebtn1CB(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
{
printf("btn1CB called\n");
}

void faceslider1CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs)
{
struct Face_window *face_window;
struct Graphics_window *graphics_window;

printf("faceslider1CB called\n");
face_window = (struct Face_window *) client_data;
graphics_window = (struct Graphics_window *) face_window->owner;
#if defined (OLD_GFX_WINDOW)
graphics_window->drawing->t
	= ((double) cbs->value)/100.0;
update_window_contents((struct Drawing *)graphics_window->drawing);
drawing_changed((struct Drawing *)graphics_window->drawing);
#endif /* defined (OLD_GFX_WINDOW) */
}

void faceslider2CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs)
{
struct Face_window *face_window;
struct Graphics_window *graphics_window;

printf("faceslider2CB called\n");
face_window = (struct Face_window *) client_data;
graphics_window = (struct Graphics_window *) face_window->owner;
#if defined (OLD_GFX_WINDOW)
graphics_window->drawing->s
	= (((double) cbs->value) - 50.0)/100.0;
remakescene((struct Drawing *)graphics_window->drawing);
drawing_changed((struct Drawing *)graphics_window->drawing);
#endif /* defined (OLD_GFX_WINDOW) */
}

void faceslider3CB(Widget w,XtPointer client_data,XmScrollBarCallbackStruct *cbs)
{
struct Face_window *face_window;
struct Graphics_window *graphics_window;

printf("faceslider3CB called\n");
face_window = (struct Face_window *) client_data;
graphics_window = (struct Graphics_window *) face_window->owner;
#if defined (OLD_GFX_WINDOW)
graphics_window->drawing->u
	= ((double) cbs->value)/100.0;
update_window_contents((struct Drawing *)graphics_window->drawing);
drawing_changed((struct Drawing *)graphics_window->drawing);
#endif /* defined (OLD_GFX_WINDOW) */
}
