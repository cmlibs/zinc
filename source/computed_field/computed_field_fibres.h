/*******************************************************************************
FILE : computed_field_fibres.h

LAST MODIFIED : 18 October 2000

DESCRIPTION :
Computed fields for extracting fibre axes from fibre angles in elements.
==============================================================================*/
#if !defined (COMPUTED_FIELD_FIBRES_H)
#define COMPUTED_FIELD_FIBRES_H

int Computed_field_set_type_fibre_axes(struct Computed_field *field,
	struct Computed_field *fibre_field,struct Computed_field *coordinate_field);
/*******************************************************************************
LAST MODIFIED : 18 October 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_FIBRE_AXES, combining a fibre and
coordinate field to return the 3, 3-component fibre axis vectors:
fibre  = fibre direction,
sheet  = fibre normal in the plane of the sheet,
normal = normal to the fibre sheet.
Sets the number of components to 9.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.

Both the fibre and coordinate fields must have no more than 3 components. The
fibre field is expected to have a FIBRE coordinate_system, although this is not
enforced.
???RC To enforce the fibre field to have a FIBRE coordinate_system, must make
the MANAGER_COPY_NOT_IDENTIFIER fail if it would change the coordinate_system
while the field is in use. Not sure if we want that restriction.
==============================================================================*/

int Computed_field_register_types_fibres(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_FIBRES_H) */
