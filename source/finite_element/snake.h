/*******************************************************************************
FILE : snake.h

LAST MODIFIED : 14 May 2001

DESCRIPTION :
Functions for making a snake of 1-D cubic Hermite elements from a chain of
data points.
==============================================================================*/
#if !defined (SNAKE_H)
#define SNAKE_H

int create_FE_element_snake_from_data_points(
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(FE_node) *node_manager,
	struct GROUP(FE_element) *element_group,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(FE_basis) *basis_manager,
	struct FE_field *coordinate_field,
	struct LIST(FE_node) *data_list,
	int number_of_elements,
	FE_value density_factor,
	FE_value stiffness);
/*******************************************************************************
LAST MODIFIED : 14 May 2001

DESCRIPTION :
Creates a snake out of <number_of_elements> 1-D cubic Hermite elements in
<element_manager> and <element_group>, nodes in <node_manager> and the node
group of the same name in <node_group_manager>. The snake follows the data in
<data_field>, using the same <coordinate_field> in the elements as the data.
<data_list> is unmodified by this function.
A positive value of <stiffness> penalises solutions with large second
derivatives; helps make smooth snakes from few data points.
==============================================================================*/

#endif /* !defined (SNAKE_H) */
