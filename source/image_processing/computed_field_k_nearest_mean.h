/*******************************************************************************
FILE : computed_field_k_nearest_mean.h

LAST MODIFIED : 17 June 2004

DESCRIPTION :
Implements image K_Nearest Neighborhood averaging on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_K_NEAREST_MEAN_H)
#define COMPUTED_FIELD_K_NEAREST_MEAN_H

int Computed_field_register_types_k_nearest_mean(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 17 June 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_K_NEAREST_MEAN_H) */
