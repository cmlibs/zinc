/*******************************************************************************
FILE : computed_field_image_tv_restoration.h

LAST MODIFIED : 17 March 2004

DESCRIPTION :
Implements image restoration on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_IMAGE_TV_RESTORATION_H)
#define COMPUTED_FIELD_IMAGE_TV_RESTORATION_H

int Computed_field_register_types_image_tv_restoration(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 17 March 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_IMAGE_TV_RESTORATION_H) */
