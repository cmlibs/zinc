/*******************************************************************************
FILE : dof3.h

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
#if !defined (DOF3_H)
#define DOF3_H

#include "general/callback_motif.h"
/* contains struct Dof3_data as it is used for conversions */
#include "io_devices/conversion.h"
#include "io_devices/matrix.h"
#if defined (EXT_INPUT)
#include "io_devices/input_module.h"
#endif

#define DOF3_STRING_SIZE 100
/* make this large so that huge numbers do not cause an overflow */
#define DOF3_NUM_POSITION_COORD 3
#define DOF3_NUM_DIRECTION_COORD 1
#define DOF3_NUM_VECTOR_COORD 2
#define DOF3_NUM_COORD 3
#define DOF3_NUM_MAX_COORD 3
/* maximum number of coordinate systems - either position or direction */

/*
UIL Identifiers
---------------
*/
#define dof3_slider_ID        1
#define dof3_combo_label_ID    2
#define dof3_combo_data_ID    3
#define dof3_combo_arrneg_ID  4
#define dof3_combo_dataexp_ID  5
#define dof3_combo_arrpos_ID  6
#define dof3_combo_toggle_ID  7

/*
Global Types
------------
*/
extern char dof3_coord_names[DOF3_NUM_COORD][DOF3_NUM_MAX_COORD][22];

enum Dof3_widget_type
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Whether it is a position or a direction widget.
==============================================================================*/
{
	DOF3_POSITION,
	DOF3_DIRECTION,
	DOF3_VECTOR
}; /* Dof3_widget_type */

enum Dof3_widget_mode
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Whether it is a absolute or relative to its current orientation.
==============================================================================*/
{
	DOF3_RELATIVE,
	DOF3_ABSOLUTE
}; /* Dof3_widget_mode */

enum Dof3_data_type
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Contains the different types of data items that are valid for the 3dof
widget.
==============================================================================*/
{
	DOF3_UPDATE_CB,
	DOF3_INPUT_WIDGET,
	DOF3_CONTROL_WIDGET,
	DOF3_DATA,
	DOF3_POSITION_WIDGET
}; /* Dof3_data_type */
#define DOF3_NUM_CALLBACKS 1

struct Dof3_ext_origin_struct
/*******************************************************************************
LAST MODIFIED : 18 March 1994

DESCRIPTION :
Contains all the information needed to support a definable origin for external
devices.  Origin holds the direction cosine matrix for the origin (transposed),
and last holds the last matrix sent by the external device.
Position holds the last position data sent by the polhemus.
==============================================================================*/
{
	Gmatrix origin,last;
	struct Dof3_data position;
}; /* Dof3_ext_origin_struct */

struct Dof3_combo_struct
/*******************************************************************************
LAST MODIFIED : 18 March 1994

DESCRIPTION :
Contains the information needed for a combo panel on a dof3 dialog.
==============================================================================*/
{
	Widget arrow_pos;
	Widget arrow_neg;
	Widget data;
	Widget dataexp;
	Widget label;
	Widget labelexp;
	Widget toggle;
}; /* Dof3_combo_struct */

struct Dof3_struct
/*******************************************************************************
LAST MODIFIED : 9 April 1997

DESCRIPTION :
Contains all the information carried by the control panel
==============================================================================*/
{
	struct Dof3_data current_value,save_value;
	struct Dof3_struct *position;
	int current_coord,return_coord;
	enum Dof3_widget_type type;
	enum Dof3_widget_mode mode;
	Gmatrix orientation; /* this is only used if the widget is a relative widget */
	int current_axis;
	int editing[3];   /* lets us decide not to accept external input if the cursor is in the input bos */
	int lock_data,link_res;
	int resolution[3],slider_ofs;
	struct Callback_data callback_array[DOF3_NUM_CALLBACKS];
	struct Dof3_combo_struct axis[3];
#if defined (EXT_INPUT)
	struct Dof3_ext_origin_struct ext_origin;
#endif
	Widget control_widget,input_widget,position_widget;
	Widget slider,widget_parent,widget,*widget_address;
}; /* Dof3_struct */


/*
Global Functions
----------------
*/
Widget create_dof3_widget(Widget *dof3_widget,Widget parent,enum Dof3_widget_type type,
	enum Dof3_widget_mode mode,int coord_sys,struct Dof3_data *init_data);
/*******************************************************************************
LAST MODIFIED : 5 January 1995

DESCRIPTION :
Creates a widget that will allow for the acquisition of three degrees of freedom.
This may either be in position form or direction form.
==============================================================================*/

int dof3_set_data(Widget dof3_widget,
	enum Dof3_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Changes a data item of the dof3 widget.
==============================================================================*/

void *dof3_get_data(Widget dof3_widget,
	enum Dof3_data_type data_type);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Returns a pointer to a data item of the dof3 widget.
==============================================================================*/

void dof3_init(struct Dof3_struct *temp_dof3,int old_coord);
/*******************************************************************************
LAST MODIFIED : 8 September 1994

DESCRIPTION :
Changes the icons and labels to reflect the new coordinate system.
==============================================================================*/

#if defined (EXT_INPUT)
int dof3_input_module_CB(void *identifier,Input_module_message message);
/*******************************************************************************
LAST MODIFIED : 7 September 1994

DESCRIPTION :
Finds the id of the buttons on the dof3ment dialog box.
==============================================================================*/
#endif

void dof3_reset(struct Dof3_struct *temp_dof3);
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Set the values to whatever has been saved.
==============================================================================*/

void dof3_save(struct Dof3_struct *temp_dof3);
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Saves the current values.
==============================================================================*/

void dof3_change_lock(struct Dof3_struct *temp_dof3,int state);
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Changes the lock_data to its state.
==============================================================================*/

int dof3_get_lock(struct Dof3_struct *temp_dof3);
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
returns the value of lock_data.
==============================================================================*/

void dof3_change_link(struct Dof3_struct *temp_dof3,int state);
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Changes the link_res to state.
==============================================================================*/

int dof3_get_link(struct Dof3_struct *temp_dof3);
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
returns the value of link_res.
==============================================================================*/






#endif

