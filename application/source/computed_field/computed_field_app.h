
#include "command/parser.h"



#include "general/message.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


int define_Computed_field(struct Parse_state *state,void *field_copy_void,
	void *define_field_package_void);
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Modifier entry function for creating and modifying Computed_fields. Format for
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


int list_Computed_field_commands(struct Computed_field *field,
	void *command_prefix_void);
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
Writes the properties of the <field> to the command window.
==============================================================================*/

int list_Computed_field_commands_if_managed_source_fields_in_list(
	struct Computed_field *field, void *list_commands_data_void);
/*******************************************************************************
LAST MODIFIED : 14 December 2001

DESCRIPTION :
Calls list_Computed_field_commands if the field is not already in the list,
has no source fields, or all its source fields are either not managed or
already in the list. If the field is listed, it is added to the list.
Ensures field command list comes out in the order they need to be created.
Note, must be cycled through as many times as it takes till listed_fields -> 0.
Second argument is a struct List_Computed_field_commands_data.
==============================================================================*/


#ifdef __cplusplus
}
#endif /* __cplusplus */
