/*******************************************************************************
FILE : computed_field_update.h

LAST MODIFIED : 11 October 2001

DESCRIPTION :
Functions for updating values of one computed field from those of another.
==============================================================================*/
#if !defined (COMPUTED_FIELD_UPDATE_H)
#define COMPUTED_FIELD_UPDATE_H

#include "finite_element/finite_element.h"
#include "selection/element_point_ranges_selection.h"
#include "selection/element_selection.h"
#include "selection/node_selection.h"
#include "user_interface/message.h"

/*
Global types
------------
*/

int Computed_field_update_nodal_values_from_source(
	struct Computed_field *destination_field,	struct Computed_field *source_field,
	struct GROUP(FE_node) *node_group, struct MANAGER(FE_node) *node_manager,
	struct FE_node_selection *node_selection);
/*******************************************************************************
LAST MODIFIED : 11 October 2001

DESCRIPTION :
Set <destination_field> in all the nodes in <node_group> or <node_manager> if
not supplied to the values from <source_field>.
Restricts update to nodes in <node_selection>, if supplied.
==============================================================================*/

int Computed_field_update_element_values_from_source(
	struct Computed_field *destination_field,	struct Computed_field *source_field,
	struct GROUP(FE_element) *element_group,
	struct MANAGER(FE_element) *element_manager,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection);
/*******************************************************************************
LAST MODIFIED : 11 October 2001

DESCRIPTION :
Set grid-based <destination_field> in all the elements in <element_group> or
<element_manager> if not supplied to the values from <source_field>.
Restricts update to grid points which are in <element_point_ranges_selection>
or whose elements are in <element_selection>, if either supplied.
Note the union of these two selections is used if both supplied.
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_UPDATE_H) */
