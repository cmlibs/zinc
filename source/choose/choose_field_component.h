/*******************************************************************************
FILE : choose_field_component.h

LAST MODIFIED : 21 November 2001

DESCRIPTION :
Specialized chooser widget that allows a component of an FE_field to be
selected from an option menu.
==============================================================================*/
#if !defined (CHOOSE_FIELD_COMPONENT_H)
#define CHOOSE_FIELD_COMPONENT_H

#include <Xm/Xm.h>
#include "finite_element/finite_element.h"
#include "general/callback.h"
#include "user_interface/user_interface.h"

/*
Global Types
------------
*/

/*
Global Functions
---------------
*/

Widget create_choose_field_component_widget(Widget parent,
	struct FE_field *field, int component_no,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Creates an option menu from which a component of the <field> may be chosen,
initially with the given <component_no>.
Note: Choose_field_component will be automatically DESTROYed with its widgets.
<user_interface> supplies fonts.
==============================================================================*/

struct Callback_data *choose_field_component_get_callback(
	Widget choose_field_component_widget);
/*****************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Returns a pointer to the callback item of the choose_field_component_widget.
============================================================================*/

int choose_field_component_set_callback(Widget choose_field_component_widget,
	struct Callback_data *new_callback);
/*****************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Changes the callback item of the choose_field_component_widget.
============================================================================*/

int choose_field_component_get_field_component(
	Widget choose_field_component_widget,struct FE_field **field,
	int *component_no);
/*****************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Returns the current field and component number in the
choose_field_component_widget.
============================================================================*/

int choose_field_component_set_field_component(
	Widget choose_field_component_widget,
	struct FE_field *field,int component_no);
/*****************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Changes the field component in the choose_field_component_widget.
============================================================================*/
#endif /* !defined (CHOOSE_FIELD_COMPONENT_H) */
