/*******************************************************************************
FILE : computed_field_wrappers.h

LAST MODIFIED : 18 October 2000

DESCRIPTION :
Functions for converting fields in a not-so-usable state into more useful
quantities, usually for graphical display or editing. For example, making a
wrapper rectangular Cartesian field out of a prolate coordinate field, making
fibre_axes out of a fibre field.
==============================================================================*/
#if !defined (COMPUTED_FIELD_WRAPPERS_H)
#define COMPUTED_FIELD_WRAPPERS_H

struct Computed_field *Computed_field_begin_wrap_coordinate_field(
	struct Computed_field *coordinate_field);
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Returns a RECTANGULAR_CARTESIAN coordinate field that may be the original
<coordinate field> if it is already in this coordinate system, or a
COMPUTED_FIELD_RC_COORDINATE wrapper for it if it is not.
Notes:
Used to ensure RC coordinate fields are passed to graphics functions.
Must call Computed_field_end_wrap() to clean up the returned field after use.
==============================================================================*/

struct Computed_field *Computed_field_begin_wrap_orientation_scale_field(
	struct Computed_field *orientation_scale_field,
	struct Computed_field *coordinate_field);
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Takes the <orientation_scale_field> and returns a field ready for use in the
rest of the program. This involves making a COMPUTED_FIELD_FIBRE_AXES wrapper
if the field has 3 or fewer components and a FIBRE coordinate system (this
requires the coordinate_field too). If the field has 3 or fewer components and
a non-RECTANGULAR_CARTESIAN coordinate system, a wrapper of type
COMPUTED_FIELD_RC_ORIENTATION_SCALE will be made for it. If the field is deemed
already usable in in its orientation_scale role, it is simply returned. Note
that the function accesses any returned field.
Note:
Must call Computed_field_end_wrap() to clean up the returned field after use.
==============================================================================*/

int Computed_field_end_wrap(struct Computed_field **wrapper_field_address);
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Cleans up a field accessed/created by a Computed_field_begin_wrap*() function.
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_WRAPPERS_H) */
