/*******************************************************************************
FILE : view_control.c

LAST MODIFIED : 23 November 2001

DESCRIPTION :
Interfaces the view widget to the 3-D graphics window.  At present it creates a
separate shell for the view widget.
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
#include <stddef.h>
#include <math.h>
#include "general/debug.h"
#include "graphics/graphics_library.h"
#include "graphics/graphics_window.h"
#include "io_devices/conversion.h"
#include "user_interface/message.h"
#include "view/view.h"
#include "view/view_control.h"

/*
Module variables
----------------
*/
#if defined (GL_API)
static Matrix idmat =
{
	1.0,0.0,0.0,0.0,
	0.0,1.0,0.0,0.0,
	0.0,0.0,1.0,0.0,
	0.0,0.0,0.0,1.0
};
#endif

/*
Module functions
----------------
*/

static void change_view(Widget widget, void *user_data, void *call_data)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
Receives an update from the view widget, and so changes the location and
direction of the viewer.
==============================================================================*/
{
	struct Camera_data *camera_data;
	struct Dof3_data dof3_data;
	int i;
#if defined (GL_API)
	int angle[3];
#endif
#if defined (OPENGL_API)
	double angle[3];
#endif

	ENTER(change_view);
	USE_PARAMETER(widget);
	USE_PARAMETER(user_data);
	if (camera_data=(struct Camera_data *)call_data)
	{
		/*???GH do we really need to keep the camera position in two coords? */
		(*(conversion_position[CONV_RECTANGULAR_CARTESIAN][CONV_SPHERICAL_POLAR]))
			(&(camera_data->position),&dof3_data);
		/* calculate absolute position of viewer for reference if needed */
		for (i=0;i<3;i++)
		{
#if defined (GL_API)
			angle[i]=camera_data->direction.data[i]*10;
#endif
#if defined (OPENGL_API)
			angle[i]=camera_data->direction.data[i];
#endif
		}
		/*???DB.  These transformation commands are already done elsewhere ?  Don't
			keep drawing up to date ?  Check with set view_point */
#if defined (GL_API)
		mmode(MVIEWING);
		loadmatrix(idmat);
		/* these commands are in the reverse order to the actual
			multiplication due to GL pre-multiplying.  These take
			euler angles and convert to viewing direction and
			twist. It is set up so the camera (-z dir in GL) is looking
			down the x axis of the receiver. */
		rotate(-900,'z');
		rotate(900,'y');
		rotate(-angle[2],'x');
		rotate(-angle[1],'y');
		rotate(-angle[0],'z');
		translate(-(camera_data->position.data[0]),-(camera_data->position.data[1]),
			-(camera_data->position.data[2]));
#endif
#if defined (OPENGL_API)
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		/* these commands are in the reverse order to the actual
			multiplication due to GL pre-multiplying.  These take
			euler angles and convert to viewing direction and
			twist. It is set up so the camera (-z dir in GL) is looking
			down the x axis of the receiver. */
		glRotated(-90,0,0,1);
		glRotated(90,0,1,0);
		glRotated(-angle[2],1,0,0);
		glRotated(-angle[1],0,1,0);
		glRotated(-angle[0],0,0,1);
		glTranslated(-(camera_data->position.data[0]),
			-(camera_data->position.data[1]),-(camera_data->position.data[2]));
#endif
	}
	else
	{
		display_message(ERROR_MESSAGE,"change_view.  Invalid argument(s)");
	}
	LEAVE;
} /* change_view */

/*
Global functions
----------------
*/
struct View_control *create_View_control(Widget parent,
	struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Creates a view widget, a shell around the view widget (with the specified
<parent>) and sets the callbacks for the view widget to interface with the
<graphics_window>.
==============================================================================*/
{
	struct View_control *view_control = NULL;
	struct Callback_data callback;

	ENTER(create_View_control);
	/* check arguments */
	if (graphics_window)
	{
		/* allocate memory */
		if (ALLOCATE(view_control,struct View_control,1))
		{
			/* initialise the structure */
			view_control->widget_parent = parent;
			view_control->widget = (Widget)NULL;
			view_control->view_widget = (Widget)NULL;
			view_control->graphics_window = graphics_window;
			/* give us a totally new shell for this dialog - use top level
				shell so it can be iconised */
			if (view_control->widget = XtVaCreatePopupShell("View Control",
				topLevelShellWidgetClass,parent,XmNallowShellResize,TRUE,NULL))
			{
				/* fetch view widget */
				if (view_control->view_widget = create_view_widget(view_control->widget,
					VIEW_CAMERA_MODE))
				{
					callback.procedure = change_view;
					callback.data = view_control;
					view_set_data(view_control->view_widget,VIEW_UPDATE_CB,&callback);
					XtManageChild(view_control->widget);
					XtRealizeWidget(view_control->widget);
					XtPopup(view_control->widget, XtGrabNone);
#if defined (EXT_INPUT)
					/* ensure that we can receive events in this window */
					display_message(INFORMATION_MESSAGE,"create_View_control: old graphics window functionality temporarily disabled\n");
#if defined (OLD_GFX_WINDOW)
					input_module_add_input_window(view_control->widget,
						graphics_window->user_interface);
#endif /* defined (OLD_GFX_WINDOW) */
#endif
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_View_control.  Could not create view widget");
					DEALLOCATE(view_control);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_View_control.  Could not create shell");
				DEALLOCATE(view_control);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_View_control.  Could not allocate control window structure");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_View_control.  Invalid argument(s)");
		view_control=(struct View_control *)NULL;
	}
	LEAVE;

	return (view_control);
} /* create_View_control */
