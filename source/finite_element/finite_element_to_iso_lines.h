/*******************************************************************************
FILE : finite_element_to_iso_lines.h

LAST MODIFIED : 28 January 2000

DESCRIPTION :
Functions for computing, sorting and storing polylines of constant scalar field
value over 2-D elements.
==============================================================================*/
#if !defined (FINITE_ELEMENT_TO_ISO_LINES_H)
#define FINITE_ELEMENT_TO_ISO_LINES_H

#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/graphics_object.h"

int create_iso_lines_from_FE_element(struct FE_element *element,
	struct Computed_field *coordinate_field,
	struct Computed_field *scalar_field,FE_value iso_value,
	struct Computed_field *data_field,int number_of_segments_in_xi1_requested,
	int number_of_segments_in_xi2_requested,struct FE_element *top_level_element,
	struct GT_object *graphics_object,float time);
/*******************************************************************************
LAST MODIFIED : 28 January 2000

DESCRIPTION :
Fills <graphics_object> (of type g_POLYLINE) with polyline contours of
<iso_scalar_field> at <iso_value>.
==============================================================================*/

#endif /* !defined (FINITE_ELEMENT_TO_ISO_LINES_H) */
