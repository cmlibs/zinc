/*******************************************************************************
FILE : comp_new_field_commands.h

LAST MODIFIED : 19 January 2003

DESCRIPTION :
Functions to connect Comp_new_field's with cmgui commands.

???DB.  Split off because want to be able to use Comp_new_field's with any
	program/interpreter and so want to be able to exclude these the cmgui
	command interface.
==============================================================================*/
#if !defined (COMP_NEW_FIELD_COMMANDS_H)
#define COMP_NEW_FIELD_COMMANDS_H

#include "command/parser.h"
#include "comp_new_field/comp_new_field.h"
	/*???DB.  May have to be _private */

#if defined (OLD_CODE)
#include "finite_element/finite_element.h"
#include "general/geometry.h"
#include "user_interface/message.h"
#endif /* defined (OLD_CODE) */

/*
Global types
------------
*/
struct List_Comp_new_field_commands_data
/*******************************************************************************
LAST MODIFIED : 2 January 2003

DESCRIPTION :
==============================================================================*/
{
	char *command_prefix;
	int listed_fields;
	struct LIST(Comp_new_field) *comp_new_field_list;
	struct MANAGER(Comp_new_field) *comp_new_field_manager;
}; /* struct List_Comp_new_field_commands_data */

/*
Global functions
----------------
*/
int define_Comp_new_field(struct Parse_state *state,void *field_copy_void,
	void *define_field_package_void);
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Modifier entry function for creating and modifying Comp_new_fields. Format for
parameters from the parse state are:
	FIELD_NAME|NEW_FIELD_NAME
		components #
			rectangular_cartesian/cylindrical_polar/spherical_polar/prolate_sph...
				component
					FIELD_NAME.COMPONENT_NAME
				composite
					scalars FIELD_NAME FIELD_NAME... FIELD_NAME{number_of_components}
				gradient
					scalar FIELD_NAME
					coordinate FIELD_NAME
				rc_coordinate
					coordinate FIELD_NAME
				scale
					field FIELD_NAME
					values # # ... #{number_of_components}
				... (more as more types added)
Note that the above layout is used because:
1. The number_of_components is often prerequisite information for setting up
the modifier functions for certain types of computed field, eg. "composite"
requires as many scalar fields as there are components, while scale has as many
floats.
2. The number_of_components and coordinate system are options for all types of
computed field so it makes sense that they are set before splitting up into the
options for the various types.
The <field_copy_void> parameter, if set, points to the field we are to modify
and should not itself be managed.
==============================================================================*/

int list_Comp_new_field(struct Comp_new_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
Writes the properties of the <field> to the command window.
==============================================================================*/

int list_Comp_new_field_commands(struct Comp_new_field *field,
	void *command_prefix_void);
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
Writes the properties of the <field> to the command window.
==============================================================================*/

int list_Comp_new_field_commands_if_managed_source_fields_in_list(
	struct Comp_new_field *field, void *list_commands_data_void);
/*******************************************************************************
LAST MODIFIED : 14 December 2001

DESCRIPTION :
Calls list_Comp_new_field_commands if the field is not already in the list,
has no source fields, or all its source fields are either not managed or
already in the list. If the field is listed, it is added to the list.
Ensures field command list comes out in the order they need to be created.
Note, must be cycled through as many times as it takes till listed_fields -> 0.
Second argument is a struct List_Comp_new_field_commands_data.
==============================================================================*/

int list_Comp_new_field_name(struct Comp_new_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 4 February 1999

DESCRIPTION :
Lists a single line about a computed field including just name, number of
components, coordinate system and type.
==============================================================================*/
#endif /* !defined (COMP_NEW_FIELD_COMMANDS_H) */
