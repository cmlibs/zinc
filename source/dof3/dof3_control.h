/*******************************************************************************
FILE : dof3_control.h

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
all of the widgets.  This is to allow the input_module to pass the correct events
to this 'structure'.

Links are created between the widgets by using the set_data routines.  The
dominant widget (ie the one highest above) should be told who its subwidgets are.
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
#if !defined (DOF3_CONTROL_H)
#define DOF3_CONTROL_H

#include "dof3/dof3.h"
#if defined (EXT_INPUT)
#include "io_devices/input_module.h"
#endif


/*
UIL Identifiers
---------------
*/
#define control_save_button_ID      1
#define control_reset_button_ID      2
#define control_lockval_button_ID    3
#define control_linkres_button_ID    4
#define control_coord_menu_button_ID      5
#define control_coord_item_button_ID      6

/*
Global Types
------------
*/
enum Control_data_type
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Contains the different types of data items that are valid for the control
widget.
==============================================================================*/
{
	CONTROL_DOF3_WIDGET,
	CONTROL_INPUT_WIDGET
}; /* Control_data_type */

struct Control_struct
/*******************************************************************************
LAST MODIFIED : 9 January 1995

DESCRIPTION :
Contains all the information carried by the control panel
==============================================================================*/
{
	enum Dof3_widget_type type;
	struct Dof3_struct *dof3;
	Widget coord[DOF3_NUM_MAX_COORD],coord_menu,linkres,load,lockval,reset,save;
	Widget dof3_widget,input_widget,widget_parent,widget;
}; /* Control_struct */

/*
Global Functions
----------------
*/
Widget create_control_widget(Widget parent,char *description);
/*******************************************************************************
LAST MODIFIED : 9 January 1995

DESCRIPTION :
Creates a widget that will control the behaviour of a 3dof widget.
==============================================================================*/

int control_set_data(Widget control_widget,
	enum Control_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Changes a data item of the control widget.
==============================================================================*/

#if defined (EXT_INPUT)
int control_input_module_CB(void *identifier,Input_module_message message);
/*******************************************************************************
LAST MODIFIED : 7 September 1994

DESCRIPTION :
Finds the id of the buttons on the control dialog box.
==============================================================================*/
#endif


#endif

