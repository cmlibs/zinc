/*******************************************************************************
FILE : computed_field_spatial_skeleton.h

LAST MODIFIED :  8 September 2004

DESCRIPTION :
Implements image skeletonizing on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_SPATIAL_SKELETON_H)
#define COMPUTED_FIELD_SPATIAL_SKELETON_H

int Computed_field_register_types_spatial_skeleton(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, 
	struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 18 March 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_SPATIAL_SKELETON_H) */
