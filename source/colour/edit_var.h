/*******************************************************************************
FILE : edit_var.h

LAST MODIFIED : 29 November 1997

DESCRIPTION :
Creates a widget that contains a description of a variable, and means to edit
it via a slider or text field.
==============================================================================*/
#if !defined (EDIT_VAR_H)
#define EDIT_VAR_H

#include <Xm/Xm.h>
#include "general/callback.h"

#define EDIT_VAR_PRECISION double
#define EDIT_VAR_PRECISION_STRING "lf"
#define EDIT_VAR_NUM_FORMAT "%6.4" EDIT_VAR_PRECISION_STRING
#define EDIT_VAR_STRING_SIZE 100
/*
Global Types
------------
*/
enum Edit_var_data_type
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Contains the different types of data stored with the edit_var control widget.
==============================================================================*/
{
	EDIT_VAR_VALUE,
	EDIT_VAR_LOW_LIMIT,
	EDIT_VAR_HIGH_LIMIT
}; /* Edit_var_data_type */

/*
Global Functions
---------------
*/
Widget create_edit_var_widget(Widget parent,char *description,
	EDIT_VAR_PRECISION init_data,EDIT_VAR_PRECISION low_limit,
	EDIT_VAR_PRECISION high_limit);
/*******************************************************************************
LAST MODIFIED : 23 January 1995

DESCRIPTION :
Allows the user to set the value of a variable between the limits low_limit
and high_limit.
==============================================================================*/

int edit_var_get_callback(Widget edit_var_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Returns the update_callback for the edit_var widget.
==============================================================================*/

int edit_var_set_callback(Widget edit_var_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Changes the update_callback for the edit_var widget.
==============================================================================*/

int edit_var_get_data(Widget edit_var_widget,
	enum Edit_var_data_type data_type,EDIT_VAR_PRECISION *data);
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Returns a data item of the edit_var widget.
==============================================================================*/

int edit_var_set_data(Widget edit_var_widget,
	enum Edit_var_data_type data_type,EDIT_VAR_PRECISION data);
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Changes a data item of the edit_var widget.
==============================================================================*/
#endif
