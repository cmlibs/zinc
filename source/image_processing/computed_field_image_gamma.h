/*******************************************************************************
FILE : computed_field_image_gamma.h

LAST MODIFIED : 1 April 2005

DESCRIPTION :
Implements image Gamma correction on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_IMAGE_GAMMA_H)
#define COMPUTED_FIELD_IMAGE_GAMMA_H

int Computed_field_register_types_image_gamma(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, 
	struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 1 April 2005

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_IMAGE_GAMMA_H) */
