/*******************************************************************************
FILE : relative.h

LAST MODIFIED : 11 January 1995

DESCRIPTION :
This module creates a free relative input device, using two dof3, two control and
one input widget.  The position is given relative to some coordinate system,
and the returned value is a global one.
==============================================================================*/
#if !defined (RELATIVE_H)
#define RELATIVE_H

#include "general/callback_motif.h"
#include "dof3/dof3.h"

#define RELATIVE_PRECISION double
#define RELATIVE_PRECISION_STRING "lf"
#define RELATIVE_STRING_SIZE 100
/* make this large so that huge numbers do not cause an overflow */

/*
UIL Identifiers
---------------
*/
#define relative_menu_ID          1
#define relative_pos_form_ID      2
#define relative_dir_form_ID      3

/*
Global Types
------------
*/
enum Relative_data_type
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Contains the different types of data items that are valid for the relative
widget.
==============================================================================*/
{
	RELATIVE_UPDATE_CB
}; /* Relative_data_type */
#define RELATIVE_NUM_CALLBACKS 1

struct Relative_data
/*******************************************************************************
LAST MODIFIED : 11 January 1995

DESCRIPTION :
Contains two dof3 data structres - a position and orientation.
==============================================================================*/
{
	struct Dof3_data position,direction;
}; /* Relative_struct */

struct Relative_struct
/*******************************************************************************
LAST MODIFIED : 11 January 1995

DESCRIPTION :
Contains all the information carried by the relative widget
==============================================================================*/
{
	struct Relative_data current_value;
	struct Callback_data callback_array[RELATIVE_NUM_CALLBACKS];
	struct Cmgui_coordinate *current_coordinate;
	Widget coord_widget,dir_form,dirctrl_widget,direction_widget,input_widget,
		menu,pos_form,posctrl_widget,position_widget,widget_parent,widget;
}; /* Relative_struct */

/*
Global Functions
----------------
*/
Widget create_relative_widget(Widget parent,struct Relative_data *init_data);
/*******************************************************************************
LAST MODIFIED : 5 January 1995

DESCRIPTION :
Creates a relative widget that gets a position and orientation from the user.
==============================================================================*/

int relative_set_data(Widget relative_widget,
	enum Relative_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Changes a data item of the relative widget.
==============================================================================*/

void *relative_get_data(Widget relative_widget,
	enum Relative_data_type data_type);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Returns a pointer to a data item of the relative widget.
==============================================================================*/


#endif

