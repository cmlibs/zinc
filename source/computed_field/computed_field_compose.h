/*******************************************************************************
FILE : computed_field_compose.h

LAST MODIFIED : 23 January 2002

DESCRIPTION :
==============================================================================*/
#if !defined (COMPUTED_FIELD_COMPOSE_H)
#define COMPUTED_FIELD_COMPOSE_H

#include "finite_element/finite_element.h"

int Computed_field_register_types_compose(
	struct Computed_field_package *computed_field_package, 
	struct MANAGER(GROUP(FE_element)) *fe_element_group_manager);
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
==============================================================================*/

int Computed_field_set_type_compose(struct Computed_field *field,
	struct Computed_field *texture_coordinate_field,
	struct Computed_field *find_element_xi_field,
	struct Computed_field *calculate_values_field,
	struct GROUP(FE_element) *search_element_group);
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_COMPOSE, this field allows you to
evaluate one field to find "texture coordinates", use a find_element_xi field
to then calculate a corresponding element/xi and finally calculate values using
this element/xi and a third field.  You can then evaluate values on a "host"
mesh for any points "contained" inside.  The <search_element_group> is the group
from which any returned element_xi will belong.
==============================================================================*/

int Computed_field_get_type_compose(struct Computed_field *field,
	struct Computed_field **texture_coordinate_field,
	struct Computed_field **find_element_xi_field,
	struct Computed_field **calculate_values_field,
	struct GROUP(FE_element) **search_element_group);
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
If the field is of type COMPUTED_FIELD_COMPOSE, the function returns the three
fields which define the field.
Note that the fields are not ACCESSed.
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_COMPOSE_H) */
