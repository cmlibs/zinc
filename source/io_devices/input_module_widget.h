/*******************************************************************************
FILE :input_module_widget.h

LAST MODIFIED : 20 March 2000

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

struct Input_module_widget_data
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Call data for Input_module_device_change callbacks, telling the client which
device has changed.
==============================================================================*/
{
	enum Input_module_device device;
	int status;
}; /* Input_module_widget_data */

DECLARE_CALLBACK_TYPES(Input_module_device_change,Widget, \
	struct Input_module_widget_data *);
DECLARE_CALLBACK_TYPES(Input_module_polhemus_change,Widget,int/*button_num*/);

struct Input_module_widget_struct
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Contains all the information carried by the menu.
==============================================================================*/
{
	int input_device[INPUT_MODULE_NUM_DEVICES];
	struct LIST(CALLBACK(Input_module_device_change))
		*device_change_callback_list;
	struct LIST(CALLBACK(Input_module_polhemus_change))
		*polhemus_change_callback_list;
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

int Input_module_add_device_change_callback(Widget input_module_widget,
	CALLBACK_FUNCTION(Input_module_device_change) *function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Asks for callbacks to <function> with <user_data> when device change events
occur in the <input_module_widget>.
==============================================================================*/

int Input_module_remove_device_change_callback(Widget input_module_widget,
	CALLBACK_FUNCTION(Input_module_device_change) *function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Stops getting callbacks to <function> with <user_data> when device change events
occur in the <input_module_widget>.
==============================================================================*/

int Input_module_add_polhemus_change_callback(Widget input_module_widget,
	CALLBACK_FUNCTION(Input_module_polhemus_change) *function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Asks for callbacks to <function> with <user_data> when polhemus change events
occur in the <input_module_widget>.
==============================================================================*/

int Input_module_remove_polhemus_change_callback(Widget input_module_widget,
	CALLBACK_FUNCTION(Input_module_polhemus_change) *function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Stops getting callbacks to <function> with <user_data> when polhemus change
events occur in the <input_module_widget>.
==============================================================================*/
#endif
