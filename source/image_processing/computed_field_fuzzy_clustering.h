/*******************************************************************************
FILE : computed_field_fuzzy_clustering.h

LAST MODIFIED : 22 December 2004

DESCRIPTION :
Implements image segmentation with fuzzy c-mean clustering on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_FUZZY_CLUSTERING_H)
#define COMPUTED_FIELD_FUZZY_CLUSTERING_H

int Computed_field_register_types_fuzzy_clustering(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, 
	struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 22 December 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_FUZZY_CLUSTERING_H) */
