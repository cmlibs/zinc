/*******************************************************************************
FILE : computed_field_lookup.h

LAST MODIFIED : 10 October 2003

DESCRIPTION :
Implements computed fields for lookups.
==============================================================================*/
#if !defined (COMPUTED_FIELD_LOOKUP_H)
#define COMPUTED_FIELD_LOOKUP_H

#include "region/cmiss_region.h"

int Computed_field_register_types_lookup(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region);
/*******************************************************************************
LAST MODIFIED : 01 October 2003

DESCRIPTION :
==============================================================================*/

int Computed_field_set_type_nodal_lookup(struct Computed_field *field,
	struct Computed_field *source_field,struct Cmiss_region *region,
  int lookup_node_identifier);
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_LOOKUP with the supplied
fields, <source_field> is the field the values are returned from but rather
than using the current node the <lookup_node_identifier> node is used.
==============================================================================*/

int Computed_field_get_type_nodal_lookup(struct Computed_field *field,
  struct Computed_field **lookup_field,struct Cmiss_region **lookup_region,
  int *lookup_node_identifier);
/*******************************************************************************
LAST MODIFIED : 10 October 2003

DESCRIPTION :
If the field is of type COMPUTED_FIELD_LOOKUP, the function returns the source
<lookup_field>, <lookup_region>, and the <lookup_node_identifier>.
Note that nothing returned has been ACCESSed.
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_LOOKUP_H) */
