/*******************************************************************************
FILE : dof3_input.h

LAST MODIFIED : 6 January 1998

DESCRIPTION :
This module allows for the creation of three types of widgets -
	dof3 (three degree of freedom)
	input (input router to dof3 and control widgets)
	control (controlling widget for each of the dof3 widgets)

The rough 'control structure' is as follows
											---------
											| INPUT |
											---------
													|
	------------------------------------------------
	|              |button messages|                | movement messages
	|         -----------     -----------          |
	|         | CONTROL1|     | CONTROL2|          |
	|         -----------     -----------          |
	|              |               |                |
	------------   |               |   -------------
							|   |               |   |
						----------         ----------
						|  DOF31 |         |  DOF32 |
						----------         ----------
This is for the full case (6DOF), where two dof3 widgets each have a controller,
and input to the whole set is controlled by the input widget.  Note that in
this case, the <bounding> widget passed to create_input_widget would encompass
all of the widgets.  This is to allow the input_module to pass the correct
events to this 'structure'.

Links are created between the widgets by using the set_data routines.  The
dominant widget (ie the one highest above) should be told who its subwidgets
are.
ie input_set data would be called with
	INPUT_POSITION_WIDGET      DOF31
	INPUT_DIRECTION_WIDGET      DOF32
	INPUT_POSCTRL_WIDGET        CONTROL1
	INPUT_DIRCTRL_WIDGET        CONTROL2
input_set_data will ensure that the control and dof3 widgets are informed as
to who is providing their input.

When any of the widgets are destroyed, they ensure that all links with other
widgets are terminated, and so the whole structure does not need to be destroyed
- widgets may be mixed and matched as the situation changes.


The update callback for the dof3 widget returns -
	Widget dof3_widget
	void *user data    (sent in Callback_data record)
	struct Dof3_data *dof3_data (the current values of the dof3 widget, in the
			coordinate system that the widget was created with)
==============================================================================*/
#if !defined (DOF3_INPUT_H)
#define DOF3_INPUT_H

#include "io_devices/input_module.h"

/*
UIL Identifiers
---------------
*/
#define input_device_button_ID      1
#define input_polhemus_button_ID    2

/*
Global Types
------------
*/
enum Input_data_type
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Contains the different types of data items that are valid for the 3dof
widget.
==============================================================================*/
{
	INPUT_POSITION_WIDGET,
	INPUT_DIRECTION_WIDGET,
	INPUT_POSCTRL_WIDGET,
	INPUT_DIRCTRL_WIDGET
}; /* Input_data_type */

struct Input_struct
/*******************************************************************************
LAST MODIFIED : 9 January 1995

DESCRIPTION :
Contains all the information carried by the control panel
==============================================================================*/
{
#if defined (EXT_INPUT)
	int input_device[INPUT_MODULE_NUM_DEVICES];
#endif
	struct Dof3_struct *position,*direction;
	struct Control_struct *posctrl,*dirctrl;
#if defined (EXT_INPUT)
	Widget input[INPUT_MODULE_NUM_DEVICES];
#endif
	Widget dirctrl_widget,direction_widget,polhemus_control[2],posctrl_widget,
		position_widget;
	Widget bounding,widget_parent,widget;
}; /* Input_struct */


/*
Global Functions
----------------
*/
Widget create_input_widget(Widget parent,Widget bounding);
/*******************************************************************************
LAST MODIFIED : 9 January 1995

DESCRIPTION :
Creates a widget that will handle the redirection of external input to dof3
widgets.  <position/direction> may be NULL.  <bounding> is the widget that
encloses all dof3 and input and control widgets that wis to be regarded as one
item.
==============================================================================*/

int input_set_data(Widget input_widget,
	enum Input_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Changes a data item of the input widget.
==============================================================================*/
#endif
