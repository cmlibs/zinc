/*******************************************************************************
FILE : computed_field_integration.h

LAST MODIFIED : 26 October 2000

DESCRIPTION :
==============================================================================*/
#if !defined (COMPUTED_FIELD_INTEGRATION_H)
#define COMPUTED_FIELD_INTEGRATION_H

int Computed_field_is_type_xi_texture_coordinates(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
==============================================================================*/

int Computed_field_set_type_xi_texture_coordinates(struct Computed_field *field,
	struct FE_element *seed_element, struct MANAGER(FE_element) *fe_element_manager);
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_XI_TEXTURE_COORDINATES.
The seed element is set to the number given and the mapping calculated.
Sets the number of components to the dimension of the given element.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
=============================================================================*/

int Computed_field_get_type_xi_texture_coordinates(struct Computed_field *field,
	struct FE_element **seed_element);
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_XI_TEXTURE_COORDINATES, 
the seed element used for the mapping is returned - otherwise an error is reported.
==============================================================================*/

int Computed_field_register_type_integration(
	struct Computed_field_package *computed_field_package,
	struct MANAGER(FE_element) *fe_element_manager);
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_INTEGRATION_H) */
