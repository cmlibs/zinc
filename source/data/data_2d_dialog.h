/*******************************************************************************
FILE : data_2d_dialog.h

LAST MODIFIED : 29 January 1999

DESCRIPTION :
This module creates a free data_2d_dialog input device, using two dof3,
two control and one input widget.  The position is given relative to some
coordinate system, and the returned value is a global one.
==============================================================================*/
#if !defined (DATA_2D_DIALOG_H)
#define DATA_2D_DIALOG_H

#include "finite_element/finite_element.h"
#include "data/data_2d.h"

/*
Global types
------------
*/
enum data_2d_dialog_data_type
/*******************************************************************************
LAST MODIFIED : 22 February 1995

DESCRIPTION :
Contains the different types of data items that are valid for the
data_2d widget.
==============================================================================*/
{
	DATA_2D_DIALOG_UPDATE_CB,
	DATA_2D_DIALOG_SELECT_CB,
	DATA_2D_DIALOG_DESTROY_CB,
	DATA_2D_DIALOG_DATA,
	DATA_2D_DIALOG_SELECT_RADIUS
}; /* data_2d_dialog_data_type */

#define DATA_2D_DIALOG_NUM_CALLBACKS 3

/*
Global functions
----------------
*/
Widget create_data_2d_dialog(Widget *data_2d_dialog_widget,
	Widget parent,int dimension,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *group_manager,
	struct GROUP(FE_node) *init_data,DATA_2D_PRECISION select_radius);
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Creates a dialog widget that allows the user to edit the properties of any of
the materials contained in the global list.
==============================================================================*/

int data_2d_dialog_set_data(Widget data_2d_dialog_widget,
	enum data_2d_dialog_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 2 April 1995

DESCRIPTION :
If <data_2d_dialog_widget> is not NULL, then change the data item on
<data_2d_dialog widget>.  Otherwise, change the data item on
<data_2d_dialog>.
==============================================================================*/

void *data_2d_dialog_get_data(Widget data_2d_dialog_widget,
	enum data_2d_dialog_data_type data_type);
/*******************************************************************************
LAST MODIFIED : 2 April 1995

DESCRIPTION :
If <data_2d_dialog_widget> is not NULL, then get the data item from
<data_2d_dialog widget>.  Otherwise, get the data item from
<data_2d_dialog>.
==============================================================================*/

int data_2d_dialog_selection(Widget data_2d_dialog_widget,DATA_2D_PRECISION *data,
	int button_down,int single);
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Notifies the grabber about the selection process.
==============================================================================*/

#endif
