/*******************************************************************************
FILE : vector.h

LAST MODIFIED : 11 January 1995

DESCRIPTION :
This module creates a free vector input device, using two dof3, two control and
one input widget.  The position is given relative to some vectorinate system,
and the returned value is a global one.
==============================================================================*/
#if !defined (VECTOR_H)
#define VECTOR_H

#include "dof3/dof3.h"

#define VECTOR_PRECISION double
#define VECTOR_PRECISION_STRING "lf"
#define VECTOR_STRING_SIZE 100
#define VECTOR_NUM_CHOICES 6
/* make this large so that huge numbers do not cause an overflow */

/*
UIL Identifiers
---------------
*/
#define vector_menu_ID          1
#define vector_toggle_ID        2

/*
Global Types
------------
*/
enum Vector_data_type
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Contains the different types of data items that are valid for the vector
widget.
==============================================================================*/
{
	VECTOR_UPDATE_CB,
	VECTOR_DATA
}; /* Vector_data_type */
#define VECTOR_NUM_CALLBACKS 1

struct Vector_struct
/*******************************************************************************
LAST MODIFIED : 11 January 1995

DESCRIPTION :
Contains all the information carried by the vector widget
==============================================================================*/
{
	struct Dof3_data current_value;
	struct Callback_data callback_array[VECTOR_NUM_CALLBACKS];
	Widget menu,toggle[VECTOR_NUM_CHOICES],widget_parent,widget;
}; /* Vector_struct */

/*
Global Functions
----------------
*/
Widget create_vector_widget(Widget parent);
/*******************************************************************************
LAST MODIFIED : 5 January 1995

DESCRIPTION :
Creates a vector widget that returns a vector in one of six directions
- +/-xyz
==============================================================================*/

int vector_set_data(Widget vector_widget,
	enum Vector_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Changes a data item of the vector widget.
==============================================================================*/

void *vector_get_data(Widget vector_widget,
	enum Vector_data_type data_type);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Returns a pointer to a data item of the vector widget.
==============================================================================*/



#endif

