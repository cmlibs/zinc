/*******************************************************************************
FILE : view.h

LAST MODIFIED : 21 January 1995

DESCRIPTION :
This module creates a view widget that will return a global camera position
and orientation.  This widget contains a toggle to switch between camera and
point of interest modes.  For each mode, the widget contains a separate
widget which gets input from the user according to where the user wishes
to look.
The user may input their viewing position relative to the origin of a model
component (ie heart etc), and the view widget will interact with the models
coordinate system to return global values.
Output from this widget is used in the following way by OPENGL and GL to create
a viewing matrix.
if defined (GL_API)
		mmode(MVIEWING);
		start from scratch
		loadmatrix(idmat);
		This is the final (cosmetic) transformation that makes the z axis be
			'up', and look down the z direction
		rotate(-900,'z');
		rotate(900,'y');
		These are the orientation transformations
		rotate(-angle[2],'x');
		rotate(-angle[1],'y');
		rotate(-angle[0],'z');
		and translate back
		translate(-position[0],-position[1],-position[2]);
endif
if defined (OPENGL_API)
		glMatrixMode(GL_MODELVIEW);
		start from scratch
		glLoadIdentity();
		This is the final (cosmetic) transformation that makes the z axis be
			'up', and look down the z direction
		glRotated(-90,0,0,1);
		glRotated(90,0,1,0);
		These are the orientation transformations
		glRotated(-angle[2],1,0,0);
		glRotated(-angle[1],0,1,0);
		glRotated(-angle[0],0,0,1);
		and translate back
		glTranslated(-position[0],-position[1],-position[2]);
endif
==============================================================================*/
#if !defined (VIEW_H)
#define VIEW_H

#include "dof3/dof3.h"
#include "general/callback_motif.h"
#include "view/camera.h"
#include "view/poi.h"

#define VIEW_PRECISION double
#define VIEW_PRECISION_STRING "lf"
#define VIEW_STRING_SIZE 100
/* make this large so that huge numbers do not cause an overflow */

/*
UIL Identifiers
---------------
*/
#define view_toggle_camera_button_ID    1
#define view_toggle_poi_button_ID        2
#define view_toggle_relative_button_ID  3
#define view_cam_form_ID                4

/*
Global Types
------------
*/
enum View_mode
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
The two states of the view widget - camera or poi.
==============================================================================*/
{
	VIEW_CAMERA_MODE,
	VIEW_POI_MODE,
	VIEW_RELATIVE_MODE
}; /* View_mode */
#define VIEW_NUM_MODES 3

enum View_data_type
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Contains the different types of data items that are valid for the view
widget.
==============================================================================*/
{
	VIEW_UPDATE_CB,
	VIEW_CAMERA_DATA,
	VIEW_POI_DATA
}; /* View_data_type */
#define VIEW_NUM_CALLBACKS 1

struct View_struct
/*******************************************************************************
LAST MODIFIED : 11 January 1995

DESCRIPTION :
Contains all the information carried by the view widget
==============================================================================*/
{
	enum View_mode mode;
	struct Camera_data current_value;
	struct Callback_data callback_array[VIEW_NUM_CALLBACKS];
	Widget cam_form,sub_widget[VIEW_NUM_MODES],toggle[VIEW_NUM_MODES],
		widget_parent,widget;
}; /* View_struct */

/*
Global Functions
----------------
*/
Widget create_view_widget(Widget parent,enum View_mode mode);
/*******************************************************************************
LAST MODIFIED : 5 January 1995

DESCRIPTION :
Creates a view widget that will either be in camera or poi mode, and will
return a global position and orientation of the camera.
==============================================================================*/

int view_set_data(Widget view_widget,
	enum View_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Changes a data item of the view widget.
==============================================================================*/

void *view_get_data(Widget view_widget,
	enum View_data_type data_type);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Returns a pointer to a data item of the view widget.
==============================================================================*/


#endif

