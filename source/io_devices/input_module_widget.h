/*******************************************************************************
FILE :input_module_widget.h

LAST MODIFIED : 14 May 1998

DESCRIPTION :
This widget allows the user to accept input from certain devices.  Only valid
devices are displayed on the menu.
==============================================================================*/
#if !defined (INPUT_MODULE_WIDGET_H)
#define INPUT_MODULE_WIDGET_H

#include "general/callback.h"
#include "io_devices/input_module.h"

/*
UIL Identifiers
---------------
*/
#define input_module_device_button_ID      1
#define input_module_polhemus_button_ID    2
#define input_module_pulldown_ID    3


/*
Global Types
------------
*/
enum Input_module_widget_data_type
/*******************************************************************************
LAST MODIFIED : 01 April 1995

DESCRIPTION :
The types of data carried by the input_module widget.
==============================================================================*/
{
	INPUT_MODULE_DEVICE_CB,        /* Receives struct Input_module_data *       */
	INPUT_MODULE_POLHEMUS_CB       /* Receives int *    0 = Set                 */
																/*                   1 = Reset               */
}; /* Input_module_widget_data_type */
#define INPUT_MODULE_NUM_CALLBACKS 2

struct Input_module_widget_data
/*******************************************************************************
LAST MODIFIED : 9 April 1997

DESCRIPTION :
Tells the clients which device has changed.
==============================================================================*/
{
	enum Input_module_device device;
	int status;
}; /* Input_module_widget_data */

struct Input_module_widget_struct
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Contains all the information carried by the menu.
==============================================================================*/
{
	int input_device[INPUT_MODULE_NUM_DEVICES];
	struct LIST(Callback_data) *callback_list[INPUT_MODULE_NUM_CALLBACKS];
	Widget input[INPUT_MODULE_NUM_DEVICES];
#if defined (POLHEMUS)
	Widget polhemus_control[2];
#endif
	Widget pulldown;
	Widget *widget_address,widget_parent,widget;
}; /* Input_module_widget_struct */


/*
Global Functions
----------------
*/
Widget create_input_module_widget(Widget *input_module_widget,Widget parent);
/*******************************************************************************
LAST MODIFIED : 9 January 1995

DESCRIPTION :
Creates a widget that will handle the redirection of external input to
other widgets.
==============================================================================*/

int input_module_widget_set_data(Widget input_module_widget,
	enum Input_module_widget_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Changes a data item of the input_module_widget.
==============================================================================*/
#endif
