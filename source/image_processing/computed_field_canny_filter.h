/*******************************************************************************
FILE : computed_field_canny_filter.h

LAST MODIFIED : 18 March 2004

DESCRIPTION :
Implements image canny edge extraction on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_CANNY_FILTER_H)
#define COMPUTED_FIELD_CANNY_FILTER_H

int Computed_field_register_types_canny_filter(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 18 March 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_CANNY_FILTER_H) */
