/*******************************************************************************
FILE : computed_field_heat_equation.h

LAST MODIFIED : 17 March 2004

DESCRIPTION :
Implements image smoothing on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_HEAT_EQUATION_H)
#define COMPUTED_FIELD_HEAT_EQUATION_H

int Computed_field_register_types_heat_equation(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 17 March 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_HEAT_EQUATION_H) */
