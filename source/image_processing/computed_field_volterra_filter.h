/*******************************************************************************
FILE : computed_field_volterrra_filter.h

LAST MODIFIED : 1 October 2004

DESCRIPTION :
Implements image edge enhancement on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_VOLTERRA_FILTER_H)
#define COMPUTED_FIELD_VOLTERRA_FILTER_H

int Computed_field_register_types_volterra_filter(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 1 October 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_VOLTERRA_FILTER_H) */
