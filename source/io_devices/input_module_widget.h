/*******************************************************************************
FILE :input_module_widget.h

LAST MODIFIED : 23 March 2000

DESCRIPTION :
This widget allows the user to accept input from certain devices.  Only valid
devices are displayed on the menu.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
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

DECLARE_CMISS_CALLBACK_TYPES(Input_module_device_change,Widget, \
	struct Input_module_widget_data *);
DECLARE_CMISS_CALLBACK_TYPES(Input_module_polhemus_change,Widget,int/*button_num*/);

struct Input_module_widget_struct
/*******************************************************************************
LAST MODIFIED : 23 March 2000

DESCRIPTION :
Contains all the information carried by the menu.
==============================================================================*/
{
	int input_device[INPUT_MODULE_NUM_DEVICES];
	struct LIST(CMISS_CALLBACK_ITEM(Input_module_device_change))
		*device_change_callback_list;
	struct LIST(CMISS_CALLBACK_ITEM(Input_module_polhemus_change))
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
	CMISS_CALLBACK_FUNCTION(Input_module_device_change) *function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Asks for callbacks to <function> with <user_data> when device change events
occur in the <input_module_widget>.
==============================================================================*/

int Input_module_remove_device_change_callback(Widget input_module_widget,
	CMISS_CALLBACK_FUNCTION(Input_module_device_change) *function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Stops getting callbacks to <function> with <user_data> when device change events
occur in the <input_module_widget>.
==============================================================================*/

int Input_module_add_polhemus_change_callback(Widget input_module_widget,
	CMISS_CALLBACK_FUNCTION(Input_module_polhemus_change) *function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Asks for callbacks to <function> with <user_data> when polhemus change events
occur in the <input_module_widget>.
==============================================================================*/

int Input_module_remove_polhemus_change_callback(Widget input_module_widget,
	CMISS_CALLBACK_FUNCTION(Input_module_polhemus_change) *function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Stops getting callbacks to <function> with <user_data> when polhemus change
events occur in the <input_module_widget>.
==============================================================================*/
#endif
