/*******************************************************************************
FILE : computed_field_coordinate.h

LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
#if !defined (COMPUTED_FIELD_COORDINATE_H)
#define COMPUTED_FIELD_COORDINATE_H

int Computed_field_extract_rc(struct Computed_field *field,
	int element_dimension,FE_value *rc_coordinates,FE_value *rc_derivatives);
/*******************************************************************************
LAST MODIFIED : 9 February 1999

DESCRIPTION :
Takes the values in <field> and converts them from their current coordinate
system into rectangular cartesian, returning them in the 3 component
<rc_coordinates> array. If <rc_derivatives> is not NULL, the derivatives are
also converted to rc and returned in that 9-component FE_value array.
Note that odd coordinate systems, such as FIBRE are treated as if they are
RECTANGULAR_CARTESIAN, which just causes a copy of values.
If <element_dimension> or the number of components in <field> are less than 3,
the missing places in the <rc_coordinates> and <rc_derivatives> arrays are
cleared to zero.
???RC Uses type float for in-between values x,y,z and jacobian for future
compatibility with coordinate system transformation functions in geometry.c.
This causes a slight drop in performance.

Note the order of derivatives:
1. All the <element_dimension> derivatives of component 1.
2. All the <element_dimension> derivatives of component 2.
3. All the <element_dimension> derivatives of component 3.
==============================================================================*/

int Computed_field_set_type_coordinate_transformation(struct Computed_field *field,
	struct Computed_field *source_field);
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_COORDINATE_TRANSFORMATION with the supplied
<source_field>.  Sets the number of components equal to the <source_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/

int Computed_field_set_type_vector_coordinate_transformation(struct Computed_field *field,
	struct Computed_field *vector_field,struct Computed_field *coordinate_field);
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_RC_VECTOR, combining a vector field
supplying a single vector (1,2 or 3 components), two vectors (4 or 6 components)
or three vectors (9 components) with a coordinate field. This field type ensures
that each source vector is converted to RC coordinates at the position given by
the coordinate field - as opposed to RC_COORDINATE which assumes the
transformation is always based at the origin.
Sets the number of components to 3 times the number of vectors expected from
the source vector_field.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/

int Computed_field_register_types_coordinate(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_COORDINATE_H) */
