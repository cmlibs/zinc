/*******************************************************************************
FILE : computed_field_gabor_filter.h

LAST MODIFIED :  22 July 2004

DESCRIPTION :
Implements image filtering on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_GABOR_FILTER_H)
#define COMPUTED_FIELD_GABOR_FILTER_H

int Computed_field_register_types_gabor_filter(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 22 July 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_GABOR_FILTER_H) */
