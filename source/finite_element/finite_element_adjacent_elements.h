/*******************************************************************************
FILE : finite_element_adjacent_elements.h

LAST MODIFIED : 1 November 2000

DESCRIPTION :
Functions for finding elements adjacent to other ones.  These functions have
been separated out from finite_element.c due to their dependence on 
indexed_multi_range.
==============================================================================*/
#if !defined (FINITE_ELEMENT_ADJACENT_ELEMENTS_H)
#define FINITE_ELEMENT_ADJACENT_ELEMENTS_H

#include "general/indexed_multi_range.h"

int adjacent_FE_element(struct FE_element *element,
	int face_number, int *number_of_adjacent_elements, 
	struct FE_element ***adjacent_elements);
/*******************************************************************************
LAST MODIFIED : 27 October 2000

DESCRIPTION :
Returns the list of <adjacent_elements> not including <element> which share the 
face indicated by <face_number>.  <adjacent_elements> is ALLOCATED to the 
correct size and should be DEALLOCATED when finished with.
==============================================================================*/

int adjacent_FE_element_from_nodes(struct FE_element *element,
	int node_index, int *number_of_adjacent_elements, 
	struct FE_element ***adjacent_elements, 
	struct LIST(Index_multi_range) *node_element_list,
	struct MANAGER(FE_element) *fe_element_manager);
/*******************************************************************************
LAST MODIFIED : 27 October 2000

DESCRIPTION :
For a 1D top level element this routine will return the list of 
<adjacent_elements> not including <element> which share the node indicated by
<node_index>.  <adjacent_elements> is REALLOCATED to the 
correct size and should be DEALLOCATED when calls to this function are finished.
The <node_element_list> should be created using create_node_element_list which
creates a list indexed by node identifying elements which refer to each node.
==============================================================================*/

struct LIST(Index_multi_range) *create_node_element_list(
   struct MANAGER(FE_element) *fe_element_manager);
/*******************************************************************************
LAST MODIFIED : 1 November 2000

DESCRIPTION :
Creates a list indexed by node identifying elements which refer to each node.
==============================================================================*/
#endif /* !defined (FINITE_ELEMENT_ADJACENT_ELEMENTS_H) */
