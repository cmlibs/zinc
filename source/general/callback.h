/*******************************************************************************
FILE : callback.h

LAST MODIFIED : 26 September 1995

DESCRIPTION :
Contains definitions to allow creation of standardised widgets.  An example of
how this should be performed may be found in colour/colour.h and colour/colour.c
???DB.  Is this level of abstraction required ?
==============================================================================*/
#if !defined (CALLBACK_H)
#define CALLBACK_H
#include <Xm/Xm.h>
#include "general/simple_list.h"

/*
Global Types
------------
*/
/*???DB.  Should have a return_code ? */
typedef void Callback_procedure(Widget,void *,void *);

struct Callback_data
/*******************************************************************************
LAST MODIFIED : 4 January 1995

DESCRIPTION :
Contains all information necessary for a callback.
==============================================================================*/
{
	Callback_procedure *procedure;
	void *data;
}; /* struct Callback_data */

DECLARE_LIST_TYPES(Callback_data);
/* DECLARE_LIST_CONDITIONAL_FUNCTION(Callback_data); */
/* DECLARE_LIST_ITERATOR_FUNCTION(Callback_data); */
PROTOTYPE_SIMPLE_LIST_OBJECT_FUNCTIONS(Callback_data);
PROTOTYPE_LIST_FUNCTIONS(Callback_data);

void callback_call_list(struct LIST(Callback_data) *callback_list,
	Widget calling_widget,void *new_data);
/*******************************************************************************
LAST MODIFIED : 24 September 1995

DESCRIPTION :
Changes a data item of the input_module widget.
==============================================================================*/
#endif
