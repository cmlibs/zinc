/*******************************************************************************
FILE : computed_field_derivatives.h

LAST MODIFIED : 1 November 2000

DESCRIPTION :
Implements computed_fields for calculating various derivative quantities such
as derivatives w.r.t. Xi, gradient, curl, divergence etc.
==============================================================================*/
#if !defined (COMPUTED_FIELD_DERIVATIVES_H)
#define COMPUTED_FIELD_DERIVATIVES_H

char *Computed_field_derivative_type_string(void);
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Return the static type string which identifies this type.
==============================================================================*/

int Computed_field_register_types_derivatives(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_DERIVATIVES_H) */
