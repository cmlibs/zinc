/*******************************************************************************
FILE : computed_field_image_enhancement.h

LAST MODIFIED : 5 March 2004

DESCRIPTION :
Implements image enhancement operation on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_IMAGE_ENHANCEMENT_H)
#define COMPUTED_FIELD_IMAGE_ENHANCEMENT_H

int Computed_field_register_types_image_enhancement(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region,
	struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_IMAGE_ENHANCEMENT_H) */
