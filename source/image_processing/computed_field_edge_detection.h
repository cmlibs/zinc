/*******************************************************************************
FILE : computed_field_edge_detection.h

LAST MODIFIED : 13 July 2004

DESCRIPTION :
Implements image processing operations on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_EDGE_DETECTION_H)
#define COMPUTED_FIELD_EDGE_DETECTION_H

int Computed_field_register_types_edge_detection(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region,
	struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 13 July 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_EDGE_DETECTION_H) */
