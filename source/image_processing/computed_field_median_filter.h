/*******************************************************************************
FILE : computed_field_median_filter.h

LAST MODIFIED : 2 December 2003

DESCRIPTION :
Implements image processing operations on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_median_filter_H)
#define COMPUTED_FIELD_median_filter_H

//struct Computed_field_median_filter_package;
//struct Image_cache;
int Computed_field_register_types_median_filter(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_median_filter_H) */
