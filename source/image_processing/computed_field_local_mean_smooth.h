/*******************************************************************************
FILE : computed_field_local_mean_smooth.h

LAST MODIFIED : 17 June 2004

DESCRIPTION :
Implements image neigborhood averaging on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_LOCAL_MEAN_SMOOTH_H)
#define COMPUTED_FIELD_LOCAL_MEAN_SMOOTH_H

int Computed_field_register_types_local_mean_smooth(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 17 June 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_LOCAL_MEAN_SMOOTH_H) */
