/*******************************************************************************
FILE : element_operations.h

LAST MODIFIED : 20 June 2001

DESCRIPTION :
Interactive operations for selecting elements with mouse and other devices.
==============================================================================*/
#if !defined (ELEMENT_OPERATIONS_H)
#define ELEMENT_OPERATIONS_H

#include "selection/element_selection.h"
#include "selection/element_point_ranges_selection.h"

int destroy_listed_elements(struct LIST(FE_element) *element_list,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct FE_element_selection *element_selection,
	struct Element_point_ranges_selection *element_point_ranges_selection);
/*******************************************************************************
LAST MODIFIED : 2 March 2001

DESCRIPTION :
Destroys all the elements in <element_list> that are not accessed outside
<element_manager>, the groups in <element_group_manager>,
<element_selection> and <element_point_ranges_selection>.
<element_group_manager>, <element_selection> and
<element_point_ranges_selection> are optional. Upon return <element_list>
contains all the elements that could not be destroyed.
???RC Should really be in its own module.
Note: currently requires all elements in the <element_list> to be of the same
CM_element_type, otherwise likely to fail. ???RC Fix this by filtering out
elements with all parents also in the list?
==============================================================================*/

#endif /* !defined (ELEMENT_OPERATIONS_H) */
