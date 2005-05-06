/*******************************************************************************
FILE : computed_field_image_resample.h

LAST MODIFIED : 5 May 2005

DESCRIPTION :
Implements image resampling on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_IMAGE_RESAMPLE_H)
#define COMPUTED_FIELD_IMAGE_RESAMPLE_H

int Computed_field_register_types_image_resample(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_IMAGE_RESAMPLE_H) */
