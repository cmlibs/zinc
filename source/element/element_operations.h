/*******************************************************************************
FILE : element_operations.h

LAST MODIFIED : 3 March 2003

DESCRIPTION :
FE_element functions that utilise non finite element data structures and
therefore cannot reside in finite element modules.
==============================================================================*/
#if !defined (ELEMENT_OPERATIONS_H)
#define ELEMENT_OPERATIONS_H

#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "general/multi_range.h"
#include "selection/element_selection.h"
#include "selection/element_point_ranges_selection.h"

/*
Global functions
----------------
*/

struct LIST(FE_element) *
	FE_element_list_from_fe_region_selection_ranges_condition(
		struct FE_region *fe_region, enum CM_element_type cm_element_type,
		struct FE_element_selection *element_selection, int selected_flag,
		struct Multi_range *element_ranges,
		struct Computed_field *conditional_field, FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Creates and returns an element list that is the intersection of:
- all the elements in <fe_region>;
- all elements in the <element_selection> if <selected_flag> is set;
- all elements in the given <element_ranges>, if any.
- all elements for which the <conditional_field> evaluates as "true"
  in its centre at the specified <time>
Up to the calling function to destroy the returned element list.
==============================================================================*/

int FE_region_change_element_identifiers(struct FE_region *fe_region,
	enum CM_element_type cm_type,	int element_offset,
	struct Computed_field *sort_by_field, FE_value time);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
Changes the identifiers of all elements of <cm_type> in <fe_region>.
If <sort_by_field> is NULL, adds <element_offset> to the identifiers.
If <sort_by_field> is specified, it is evaluated at the centre of all elements
in the group and the elements are sorted by it - changing fastest with the first
component and keeping the current order where the field has the same values.
Checks for and fails if attempting to give any of the elements in <fe_region> an
identifier already used by an element in the same master FE_region.
Calls to this function should be enclosed in FE_region_begin_change/end_change.
Note function avoids iterating through FE_region element lists as this is not
allowed during identifier changes.
==============================================================================*/

#endif /* !defined (ELEMENT_OPERATIONS_H) */
