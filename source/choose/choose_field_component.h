/*******************************************************************************
FILE : choose_field_component.h

LAST MODIFIED : 29 September 1997

DESCRIPTION :
Specialized version of chooser widget that allows component of an FE_field to
be selected from an option menu.
==============================================================================*/
#if !defined (CHOOSE_FIELD_COMPONENT_H)
#define CHOOSE_FIELD_COMPONENT_H

#include <Xm/Xm.h>
#include "finite_element/finite_element.h"
#include "general/callback.h"

/*
Global Types
------------
*/

/*
Global Functions
---------------
*/
Widget create_choose_field_component_widget(Widget parent,
	struct FE_field *field,int component_no);
/*****************************************************************************
LAST MODIFIED : 29 September 1997

DESCRIPTION :
Creates an option menu from which a component of the field may be chosen.
============================================================================*/

int choose_field_component_set_callback(Widget choose_field_component_widget,
	struct Callback_data *new_callback);
/*****************************************************************************
LAST MODIFIED : 29 September 1997

DESCRIPTION :
Changes the callback item of the choose_field_component_widget.
============================================================================*/

int choose_field_component_set_field_component(
	Widget choose_field_component_widget,
	struct FE_field *field,int component_no);
/*****************************************************************************
LAST MODIFIED : 29 September 1997

DESCRIPTION :
Changes the field component in the choose_field_component_widget.
============================================================================*/

struct Callback_data *choose_field_component_get_callback(
	Widget choose_field_component_widget);
/*****************************************************************************
LAST MODIFIED : 29 September 1997

DESCRIPTION :
Returns a pointer to the callback item of the choose_field_component_widget.
============================================================================*/

int choose_field_component_get_field_component(
	Widget choose_field_component_widget,struct FE_field **field,
	int *component_no);
/*****************************************************************************
LAST MODIFIED : 29 September 1997

DESCRIPTION :
Returns the current field and component number in the
choose_field_component_widget.
============================================================================*/
#endif /* !defined (CHOOSE_FIELD_COMPONENT_H) */
