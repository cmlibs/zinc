/*******************************************************************************
FILE : computed_field_adjusted_gaussian.h

LAST MODIFIED : 16 November 2004

DESCRIPTION :
Implements image Gaussian smoothing on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_ADJUSTED_GAUSSIAN_H)
#define COMPUTED_FIELD_ADJUSTED_GAUSSIAN_H

int Computed_field_register_types_adjusted_gaussian(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, 
	struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 16 November 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_ADJUSTED_GAUSSIAN_H) */
