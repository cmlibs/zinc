/*******************************************************************************
FILE : computed_field_region_label.h

LAST MODIFIED : 3 March 2004

DESCRIPTION :
Implements region labeling on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_REGION_LABEL_H)
#define COMPUTED_FIELD_REGION_LABEL_H

int Computed_field_register_types_region_label(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 4 March 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_REGION_LABEL_H) */
