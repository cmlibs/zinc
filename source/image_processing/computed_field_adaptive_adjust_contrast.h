/*******************************************************************************
FILE : computed_field_adaptive_adjust_contrast.h

LAST MODIFIED : 12 May 2004

DESCRIPTION :
Implements image contrast adjusting on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_ADAPTIVE_ADJUST_CONTRAST_H)
#define COMPUTED_FIELD_ADAPTIVE_ADJUST_CONTRAST_H

int Computed_field_register_types_adaptive_adjust_contrast(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 12 May 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_ADAPTIVE_ADJUST_CONTRAST_H) */
