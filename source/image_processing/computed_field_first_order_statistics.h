/*******************************************************************************
FILE : computed_field_first_order_statistics.h

LAST MODIFIED : 17 June 2004

DESCRIPTION :
Implements image neigborhood averaging on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_FIRST_ORDER_STATISTICS_H)
#define COMPUTED_FIELD_FIRST_ORDER_STATISTICS_H

int Computed_field_register_types_first_order_statistics(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 17 June 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_FIRST_ORDER_STATISTICS_H) */
