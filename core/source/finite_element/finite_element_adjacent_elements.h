/*******************************************************************************
FILE : finite_element_adjacent_elements.h

LAST MODIFIED : 13 March 2003

DESCRIPTION :
Functions for finding elements adjacent to other ones.  These functions have
been separated out from finite_element.c due to their dependence on 
indexed_multi_range.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_ADJACENT_ELEMENTS_H)
#define FINITE_ELEMENT_ADJACENT_ELEMENTS_H

#include "zinc/element.h"
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
	cmzn_mesh_id mesh);
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
For a 1D top level element this routine will return the list of 
<adjacent_elements> not including <element> which share the node indicated by
<node_index>.  <adjacent_elements> is ALLOCATED to the 
correct size and should be DEALLOCATED when calls to this function are finished.
Note elements in adjacent_elements array are not accessed.
==============================================================================*/

/***************************************************************************//**
 * Creates a list indexed by node identifying elements which refer to each node.
 * @param mesh  Mesh to which elements must belong.
 */
struct LIST(Index_multi_range) *create_node_element_list(cmzn_mesh_id mesh);

#endif /* !defined (FINITE_ELEMENT_ADJACENT_ELEMENTS_H) */
