/*******************************************************************************
FILE : computed_field_convolution_filter.h

LAST MODIFIED : 13 May 2005

DESCRIPTION :
Implements image convolution on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_CONVOLUTION_FILTER_H)
#define COMPUTED_FIELD_CONVOLUTION_FILTER_H

int Computed_field_register_types_convolution_filter(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 13 May 2005

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_CONVOLUTION_FILTER_H) */
