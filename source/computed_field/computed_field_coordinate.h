/*******************************************************************************
FILE : computed_field_coordinate.h

LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
#if !defined (COMPUTED_FIELD_COORDINATE_H)
#define COMPUTED_FIELD_COORDINATE_H

int Computed_field_evaluate_rc_coordinate(struct Computed_field *field,
  int element_dimension,int calculate_derivatives);
/*******************************************************************************
LAST MODIFIED : 25 June 1999

DESCRIPTION :
Function called by Computed_field_evaluate_cache_in_element/at_node to compute
rectangular cartesian coordinates from the source_field values in an arbitrary
coordinate system.
NOTE: Assumes that values and derivatives arrays are already allocated in
<field>, and that its source_fields are already computed (incl. derivatives if
calculate_derivatives set) for the same element, with the given
<element_dimension> = number of Xi coords.
Note: both COMPUTED_FIELD_DEFAULT_COORDINATE and COMPUTED_FIELD_RC_COORDINATE
are computed with this function.
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_COORDINATE_H) */
