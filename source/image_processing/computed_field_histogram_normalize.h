/*******************************************************************************
FILE : computed_field_histogram_normalize.h

LAST MODIFIED : 17 June 2004

DESCRIPTION :
Implements histogram_based normalizing operations on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_HISTOGRAM_NORMALIZE_H)
#define COMPUTED_FIELD_HISTOGRAM_NORMALIZE_H

int Computed_field_register_types_histogram_normalize(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 17 June 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_HISTOGRAM_NORMALIZE_H) */
