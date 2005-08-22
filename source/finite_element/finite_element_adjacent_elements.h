/*******************************************************************************
FILE : finite_element_adjacent_elements.h

LAST MODIFIED : 13 March 2003

DESCRIPTION :
Functions for finding elements adjacent to other ones.  These functions have
been separated out from finite_element.c due to their dependence on 
indexed_multi_range.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
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
	struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
For a 1D top level element this routine will return the list of 
<adjacent_elements> not including <element> which share the node indicated by
<node_index>.  <adjacent_elements> is ALLOCATED to the 
correct size and should be DEALLOCATED when calls to this function are finished.
==============================================================================*/

struct LIST(Index_multi_range) *create_node_element_list(
	struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Creates a list indexed by node identifying elements which refer to each node.
==============================================================================*/
#endif /* !defined (FINITE_ELEMENT_ADJACENT_ELEMENTS_H) */
