/*******************************************************************************
FILE : callback.c

LAST MODIFIED : 19 June 1996

DESCRIPTION :
Contains definitions to allow creation of standardised widgets.  An example of
how this should be performed may be found in colour/colour.h and colour/colour.c
???DB.  Is this level of abstraction required ?
==============================================================================*/
#include "general/callback.h"
#include "general/debug.h"
#include "general/list_private.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/
FULL_DECLARE_LIST_TYPE(Callback_data);

struct Callback_data_call_list_struct
{
	Widget calling_widget;
	void *new_data;
};

/*
Module functions
----------------
*/
DECLARE_SIMPLE_LIST_OBJECT_FUNCTIONS(Callback_data)

DECLARE_LIST_FUNCTIONS(Callback_data)

static int callback_data_call_list(struct Callback_data *current_callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Changes a data item of the input_module widget.
==============================================================================*/
{
	int return_code;
	struct Callback_data_call_list_struct *callback_info = user_data;

	ENTER(callback_data_call_list);
	/*???DB.  The callback procedure should have a return_code. */
	return_code=1;
	(current_callback->procedure)(callback_info->calling_widget,
		current_callback->data,callback_info->new_data);
	LEAVE;

	return (return_code);
} /* callback_data_call_list */

/*
Global functions
----------------
*/
void callback_call_list(struct LIST(Callback_data) *callback_list,
	Widget calling_widget,void *new_data)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Changes a data item of the input_module widget.
==============================================================================*/
{
	struct Callback_data_call_list_struct callback_info;

	ENTER(callback_call_list);
	callback_info.calling_widget=calling_widget;
	callback_info.new_data=new_data;
	FOR_EACH_OBJECT_IN_LIST(Callback_data)(callback_data_call_list,&callback_info,
		callback_list);
	LEAVE;
} /* callback_call_list */
