/*******************************************************************************
FILE : poi.h

LAST MODIFIED : 8 April 1995

DESCRIPTION :
This module creates a free poi input device, using two dof3, two control and
one input widget.  The position is given relative to some coordinate system,
and the returned value is a global one.
==============================================================================*/
#if !defined (POI_H)
#define POI_H

#include "dof3/dof3.h"
#include "general/callback_motif.h"

#define POI_PRECISION double
#define POI_PRECISION_STRING "lf"
#define POI_STRING_SIZE 100
/* make this large so that huge numbers do not cause an overflow */

/*
UIL Identifiers
---------------
*/
#define poi_menu_position_ID    1
#define poi_menu_poi_ID          2
#define poi_position_form_ID    3
#define poi_poi_form_ID          4
#define poi_up_vector_form_ID    5
#define poi_coord_position_ID    6
#define poi_coord_poi_ID        7

/*
Global Types
------------
*/
enum Poi_data_type
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Contains the different types of data items that are valid for the poi
widget.
==============================================================================*/
{
	POI_UPDATE_CB,
	POI_DATA
}; /* Poi_data_type */
#define POI_NUM_CALLBACKS 1

struct Poi_data
/*******************************************************************************
LAST MODIFIED : 11 January 1995

DESCRIPTION :
Contains three dof3 data structures - a position and point that we are looking
at and an up vector.
==============================================================================*/
{
	struct Dof3_data position,poi,up_vector;
}; /* Poi_struct */

struct Poi_struct
/*******************************************************************************
LAST MODIFIED : 11 January 1995

DESCRIPTION :
Contains all the information carried by the poi widget
==============================================================================*/
{
	int relative;
	struct Poi_data current_value;
	struct Callback_data callback_array[POI_NUM_CALLBACKS];
	struct Cmgui_coordinate *current_position_coordinate,
		*current_poi_coordinate;
	Widget coord_position_widget,coord_poi_widget,poi_form,position_form,
		up_vector_form,poictrl_widget,positionctrl_widget,poi_widget,
		position_widget,up_vector_widget,input_position_widget,input_poi_widget,
		menu_position,menu_poi,widget_parent,widget,position_coord_form,
		poi_coord_form,*widget_address;
}; /* Poi_struct */

/*
Global Functions
----------------
*/
Widget create_poi_widget(Widget *poi_widget,Widget parent,struct Poi_data *init_data);
/*******************************************************************************
LAST MODIFIED : 5 January 1995

DESCRIPTION :
Creates a poi widget that gets a camera position, point of interest and an
up vector from the user.
==============================================================================*/

int poi_set_data(Widget poi_widget,
	enum Poi_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Changes a data item of the poi widget.
==============================================================================*/

void *poi_get_data(Widget poi_widget,
	enum Poi_data_type data_type);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Returns a pointer to a data item of the poi widget.
==============================================================================*/


#endif

