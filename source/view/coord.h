/*******************************************************************************
FILE : coord.h

LAST MODIFIED : 19 June 1996

DESCRIPTION :
This module creates a free coord input device, using two dof3, two control and
one input widget.  The position is given relative to some coordinate system,
and the returned value is a global one.
==============================================================================*/
#if !defined (COORD_H)
#define COORD_H

#include "general/list.h"
#include "view/camera.h"

#define COORD_PRECISION double
#define COORD_PRECISION_STRING "lf"
#define COORD_STRING_SIZE 100
/* make this large so that huge numbers do not cause an overflow */

/* ************************************************************************** */
/* ************************************************************************** */
/*  This is just a hack - it will actually interface with CMGUI coordinates   */
/* ************************************************************************** */
/* ************************************************************************** */

struct Cmgui_coordinate
/*******************************************************************************
LAST MODIFIED : 28 December 1995

DESCRIPTION :
Allows a linked list of coordinate systems to be created.
==============================================================================*/
{
	char *name;
	int access_count;
	struct Camera_data origin;
};
extern struct Cmgui_coordinate *global_coordinate_ptr;

/*
UIL Identifiers
---------------
*/
#define coord_menu_ID          1
#define coord_toggle_ID        2

/*
Global Types
------------
*/
enum Coord_data_type
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Contains the different types of data items that are valid for the coord
widget.
==============================================================================*/
{
	COORD_UPDATE_CB,
	COORD_COORD_DATA
}; /* Coord_data_type */
#define COORD_NUM_CALLBACKS 1

struct Coord_data
/*******************************************************************************
LAST MODIFIED : 11 January 1995

DESCRIPTION :
Allows a linked list of coordinate systems to be created.
==============================================================================*/
{
	int access_count;
	Widget button;
	struct Cmgui_coordinate *coord;
}; /* Coord_data */

struct Coord_widget
/*******************************************************************************
LAST MODIFIED : 11 January 1995

DESCRIPTION :
Keeps a record of all the coord widgets in existence.
==============================================================================*/
{
	int access_count;
	Widget coord_widget;
}; /* Coord_widget */

/* Three types of lists:
	The first is a trash list which will be replaced by the actual cmgui list
	The second contains a list of all Coord widgets to enable global reckoning of
		coord systems
	The third is just a list of toggle widgets and their associated coord systems
*/
PROTOTYPE_OBJECT_FUNCTIONS(Cmgui_coordinate);
DECLARE_LIST_TYPES(Cmgui_coordinate);
DECLARE_LIST_TYPES(Coord_widget);
DECLARE_LIST_TYPES(Coord_data);

struct Cmgui_position
/*******************************************************************************
LAST MODIFIED : 28 December 1995

DESCRIPTION :
Combines a 3 number triple, with the coordinate system that the number is
based on.  The values are always in RC.
==============================================================================*/
{
	struct Dof3_data position;
	struct Cmgui_coordinate *coordinate;
}; /* Cmgui_position */

struct Coord_struct
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Contains all the information carried by the coord widget
==============================================================================*/
{
	struct Cmgui_coordinate *current_value;
	struct LIST(Coord_data) *coord_list;
	struct Callback_data callback_array[COORD_NUM_CALLBACKS];
	Widget menu,widget_parent,widget;
}; /* Coord_struct */

/*
Global Variables
----------------
*/
extern struct LIST(Cmgui_coordinate) *cmgui_coordinate_list;
extern struct LIST(Coord_widget) *coord_widget_list;
extern struct Cmgui_coordinate *global_coordinate_ptr;
extern struct Cmgui_coordinate *poi_coordinate_ptr;

/*
Global Functions
----------------
*/
Widget create_coord_widget(Widget parent);
/*******************************************************************************
LAST MODIFIED : 5 January 1995

DESCRIPTION :
Creates a coord widget that chooses a coordinate origin.
==============================================================================*/

int coord_set_data(Widget coord_widget,
	enum Coord_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Changes a data item of the coord widget.
==============================================================================*/

void *coord_get_data(Widget coord_widget,
	enum Coord_data_type data_type);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Returns a pointer to a data item of the coord widget.
==============================================================================*/

void coord_add_coordinate(Widget coord_widget,
	struct Cmgui_coordinate *new_Cmgui_coordinate);
/*******************************************************************************
LAST MODIFIED : 12 January 1995

DESCRIPTION :
This routine should only be used in special circumstances, as it will only
add the coordinate system to ONE widget.  This function acts as a wrapper
around coord_add_button.
==============================================================================*/

void coord_widget_init(void);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Initialises the list of coord widgets.
==============================================================================*/

void coord_widget_finish(void);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Frees up the list of coordinate systems.
==============================================================================*/

void coord_widget_add_coord(struct Cmgui_coordinate *new_coord);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Adds a coordinate to all coord widgets.
==============================================================================*/

void coord_widget_delete_coord(struct Cmgui_coordinate *old_coord);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Removes a coordinate from all coord widgets.
==============================================================================*/

void coord_widget_add_widget(Widget new_widget);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Adds a widget to the list of all coord_widgets.
==============================================================================*/

void coord_widget_delete_widget(Widget old_widget);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Removes a widget from the list of all coord_widgets.
==============================================================================*/

int set_Cmgui_coordinate(struct Parse_state *state,void *coordinate_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 19 June 1996

DESCRIPTION :
Modifier function to set the coordinate from a command.
==============================================================================*/
#endif
