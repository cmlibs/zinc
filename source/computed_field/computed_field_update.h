/*******************************************************************************
FILE : computed_field_update.h

LAST MODIFIED : 4 February 2002

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

int Computed_field_copy_values_at_node(struct FE_node *node,
	struct Computed_field *destination_field,
	struct Computed_field *source_field, FE_value time);
/*******************************************************************************
LAST MODIFIED : 4 February 2002

DESCRIPTION :
Evaluates <source_field> at node and sets <destination_field> to those values.
<node> must not be managed -- ie. it should be a local copy.
Both fields must have the same number of values.
Assumes both fields are defined at the node.
Up to user to call Computed_field_clear_cache for each field after calls to
this function are finished.
==============================================================================*/

int Computed_field_update_nodal_values_from_source(
	struct Computed_field *destination_field,	struct Computed_field *source_field,
	struct GROUP(FE_node) *node_group, struct MANAGER(FE_node) *node_manager,
	struct FE_node_selection *node_selection, FE_value time);
/*******************************************************************************
LAST MODIFIED : 21 November 2001

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
	struct FE_element_selection *element_selection, FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Set grid-based <destination_field> in all the elements in <element_group> or
<element_manager> if not supplied to the values from <source_field>.
Restricts update to grid points which are in <element_point_ranges_selection>
or whose elements are in <element_selection>, if either supplied.
Note the union of these two selections is used if both supplied.
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_UPDATE_H) */
