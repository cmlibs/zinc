/*******************************************************************************
FILE : transformation_editor.h

LAST MODIFIED : 09 April 1995

DESCRIPTION :
This module creates a widget that will allow the user to position a model in
three dimensional space, relative to some 'parent' coordinate system.
==============================================================================*/
#if !defined (TRANSFORMATION_EDITOR_H)
#define TRANSFORMATION_EDITOR_H

#include "general/callback.h"
#include "dof3/dof3.h"
#include "view/coord.h" /* only for Cmgui_coordinate */
#define TRANSFORMATION_EDITOR_PRECISION double
#define TRANSFORMATION_EDITOR_PRECISION_STRING "lf"
#define TRANSFORMATION_EDITOR_STRING_SIZE 100
/* make this large so that huge numbers do not cause an overflow */

/*
UIL Identifiers
---------------
*/
#define transformation_editor_menu_ID          1
#define transformation_editor_pos_form_ID      2
#define transformation_editor_dir_form_ID      3
#define transformation_editor_coord_ID          4
#define transformation_editor_pos_menu_ID      5
#define transformation_editor_dir_menu_ID      6
#define transformation_editor_name_ID          7

/*
Global Types
------------
*/
enum Transformation_editor_data_type
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Contains the different types of data items that are valid for the transformation_editor
widget.
==============================================================================*/
{
	TRANSFORMATION_EDITOR_UPDATE_CB,
	TRANSFORMATION_EDITOR_DATA
}; /* Transformation_editor_data_type */
#define TRANSFORMATION_EDITOR_NUM_CALLBACKS 1

struct Transformation_editor_struct
/*******************************************************************************
LAST MODIFIED : 11 January 1995

DESCRIPTION :
Contains all the information carried by the transformation_editor widget
==============================================================================*/
{
	struct Cmgui_coordinate current_value;
	struct Callback_data callback_array[TRANSFORMATION_EDITOR_NUM_CALLBACKS];
	struct Cmgui_coordinate *parent_coordinate;
	Widget coord_widget,dir_form,dirctrl_widget,direction_widget,input_widget,
		menu,pos_form,posctrl_widget,position_widget,widget_parent,widget,
		coord_form,pos_menu,dir_menu,name,*widget_address;
}; /* Transformation_editor_struct */

/*
Global Functions
----------------
*/
Widget create_transformation_editor_widget(Widget *transformation_editor_widget,
	Widget parent,struct Cmgui_coordinate *init_data);
/*******************************************************************************
LAST MODIFIED : 5 January 1995

DESCRIPTION :
Creates a transformation_editor widget that gets a transformation_editor and orientation from the user.
==============================================================================*/

int transformation_editor_set_data(Widget transformation_editor_widget,
	enum Transformation_editor_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Changes a data item of the transformation_editor widget.
==============================================================================*/

void *transformation_editor_get_data(Widget transformation_editor_widget,
	enum Transformation_editor_data_type data_type);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Returns a pointer to a data item of the transformation_editor widget.
==============================================================================*/
#endif
