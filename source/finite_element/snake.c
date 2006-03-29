/*******************************************************************************
FILE : snake.c

LAST MODIFIED : 29 March 2006

DESCRIPTION :
Functions for making a snake of 1-D cubic Hermite elements from a chain of
data points.
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
#include <math.h>
#include <stdio.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/snake.h"
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "user_interface/message.h"

/*
Module Constants
----------------
*/

/* gauss point positions and weights are on [-0.5, 0.5]. Both taken from CM */
double gauss_point[4] = { -0.4305681557970260, -0.1699905217924280, 0.1699905217924280, 0.4305681557970260 };
double gauss_weight[4] = { 0.1739274225687270, 0.3260725774312730, 0.3260725774312730, 0.1739274225687270 };

/*
Module functions
----------------
*/

struct FE_node_accumulate_length_data
/*******************************************************************************
LAST MODIFIED : 9 May 2001

DESCRIPTION :
User data for FE_node_accumulate_length function.
<lengths> must point to as many FE_values as there are nodes passed to the
function, while <coordinates> must have space for
number_of_coordinate_components per node.
Set node_number to zero before passing.
==============================================================================*/
{
	FE_value *fitting_field_values;
	/* Space to evaluate the coordinate field and keep the previous value */
	FE_value *coordinates;
	FE_value *lengths;
	int node_number;
	struct Computed_field *coordinate_field;
	struct LIST(FE_field) *fe_field_list;
	struct FE_node *current_node;
};

static int FE_field_evaluate_snake_position(struct FE_field *field,
	void *accumulate_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2006

DESCRIPTION :
Evaluates the <field> into the <fitting_field_values>.
==============================================================================*/
{
	int i, number_of_components, return_code;
	struct FE_node_accumulate_length_data *accumulate_data;

	ENTER(FE_node_accumulate_length);
	if (field && (accumulate_data =
		(struct FE_node_accumulate_length_data *)accumulate_data_void))
	{
		return_code = 1;
		number_of_components = get_FE_field_number_of_components(field);
		for (i = 0; (i < number_of_components) && return_code; i++)
		{
			if (get_FE_nodal_FE_value_value(accumulate_data->current_node,
				field, /*component_number*/i, /*version*/0,
				FE_NODAL_VALUE, /*time*/0, accumulate_data->fitting_field_values))
			{
				accumulate_data->fitting_field_values++;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_field_evaluate_snake_position.  Field %s component not defined at node or data %d",
					get_FE_field_name(field), 
					get_FE_node_identifier(accumulate_data->current_node));
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_evaluate_snake_position.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_evaluate_snake_position */

static int FE_node_accumulate_length(struct FE_node *node,
	void *accumulate_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2006

DESCRIPTION :
Calculates the coordinates and length from the first node.
==============================================================================*/
{
	double sum;
	FE_value *coordinates, distance, *lengths;
	int i, node_number, number_of_components, return_code;
	struct FE_node_accumulate_length_data *accumulate_data;
	struct Computed_field *coordinate_field;

	ENTER(FE_node_accumulate_length);
	if (node && (accumulate_data =
		(struct FE_node_accumulate_length_data *)accumulate_data_void) &&
		(coordinates = accumulate_data->coordinates) &&
		(lengths = accumulate_data->lengths) &&
		(0 <= (node_number = accumulate_data->node_number)) &&
		(coordinate_field = accumulate_data->coordinate_field) &&
		(1 < (number_of_components =
			Computed_field_get_number_of_components(coordinate_field))))
	{
		accumulate_data->current_node = node;
		return_code = FOR_EACH_OBJECT_IN_LIST(FE_field)(
			FE_field_evaluate_snake_position, accumulate_data_void,
			accumulate_data->fe_field_list);

		if (return_code)
		{
			if (Computed_field_evaluate_at_node(coordinate_field, node,
				/*time*/0.0, coordinates))
			{
				if (0 == node_number)
				{
					lengths[0] = 0.0;
					for (i = 0; i < number_of_components; i++)
					{
					coordinates[i + number_of_components] = coordinates[i];
					}
				}
				else
				{
					sum = 0.0;
					for (i = 0; i < number_of_components; i++)
					{
						distance = (coordinates[i] - coordinates[i + number_of_components]);
						coordinates[i + number_of_components] = coordinates[i];
						sum += distance*distance;
					}
					distance = sqrt(sum);
					lengths[node_number] = lengths[node_number - 1] + distance;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_node_accumulate_length.  Unable to evaluate coordinate field.");
				return_code = 0;
			}
			(accumulate_data->node_number)++;
		}
		accumulate_data->current_node = (struct FE_node *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_accumulate_length.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_accumulate_length */

int calculate_Hermite_basis_1d(double xi, double *phi)
/*******************************************************************************
LAST MODIFIED : 9 May 2001

DESCRIPTION :
Returns 1-D Hermite basis functions at <xi> in [0.0, 1.0].
<phi> must point to space for 4 doubles, where, on return:
<phi>[0] multiplies the position at the start of the curve.
<phi>[1] multiplies the derivative at the start of the curve.
<phi>[2] multiplies the position at the end of the curve.
<phi>[3] multiplies the derivative at the end of the curve.
==============================================================================*/
{
	double xi_2, xi_3;
	int return_code;

	ENTER(calculate_Hermite_basis_1d);
	if ((0.0 <= xi) && (1.0 >= xi) && phi)
	{
		xi_2 = xi*xi;
		xi_3 = xi_2*xi;
		phi[0] = 2.0*xi_3 - 3.0*xi_2 + 1.0;
		phi[1] = xi_3 - 2.0*xi_2 + xi;
		phi[2] = -2.0*xi_3 + 3.0*xi_2;
		phi[3] = xi_3 - xi_2;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_Hermite_basis_1d.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* calculate_Hermite_basis_1d */

int calculate_Hermite_basis_1d_derivatives(double xi, double *dphi_dxi)
/*******************************************************************************
LAST MODIFIED : 14 May 2001

DESCRIPTION :
Returns first derivatives of 1-D Hermite basis functions at <xi> in [0.0, 1.0].
<dphi_dxi> must point to space for 4 doubles, where, on return:
<dphi_dxi>[0] multiplies the position at the start of the curve.
<dphi_dxi>[1] multiplies the derivative at the start of the curve.
<dphi_dxi>[2] multiplies the position at the end of the curve.
<dphi_dxi>[3] multiplies the derivative at the end of the curve.
==============================================================================*/
{
	double xi_2;
	int return_code;

	ENTER(calculate_Hermite_basis_1d_derivatives);
	if ((0.0 <= xi) && (1.0 >= xi) && dphi_dxi)
	{
		xi_2 = xi*xi;
		dphi_dxi[0] = 6.0*(xi_2 - xi);
		dphi_dxi[1] = 3.0*xi_2 - 4.0*xi + 1;
		dphi_dxi[2] = -dphi_dxi[0];
		dphi_dxi[3] = 3.0*xi_2 - 2.0*xi;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_Hermite_basis_1d_derivatives.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* calculate_Hermite_basis_1d_derivatives */

int calculate_Hermite_basis_1d_second_derivatives(double xi, double *d2phi_dxi2)
/*******************************************************************************
LAST MODIFIED : 14 May 2001

DESCRIPTION :
Returns second derivatives of 1-D Hermite basis functions at <xi> in [0.0, 1.0].
<d2phi_dxi2> must point to space for 4 doubles, where, on return:
<d2phi_dxi2>[0] multiplies the position at the start of the curve.
<d2phi_dxi2>[1] multiplies the derivative at the start of the curve.
<d2phi_dxi2>[2] multiplies the position at the end of the curve.
<d2phi_dxi2>[3] multiplies the derivative at the end of the curve.
==============================================================================*/
{
	int return_code;

	ENTER(calculate_Hermite_basis_1d_second_derivatives);
	if ((0.0 <= xi) && (1.0 >= xi) && d2phi_dxi2)
	{
		d2phi_dxi2[0] = 12.0*xi - 6.0;
		d2phi_dxi2[1] = 6.0*xi - 4.0;
		d2phi_dxi2[2] = -d2phi_dxi2[0];
		d2phi_dxi2[3] = 6.0*xi - 2.0;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_Hermite_basis_1d_second_derivatives.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* calculate_Hermite_basis_1d_second_derivatives */

int define_FE_field_at_node_simple(struct FE_node *node, struct FE_field *field,
	int number_of_derivatives, enum FE_nodal_value_type *nodal_value_types)
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Defines <field> at <node> using the same <number_of_derivatives>
and <nodal_value_types> for each component, and only 1 version.
???RC Function could be used in other modules; move to finite_element.c?
==============================================================================*/
{
	int j, n, number_of_components, return_code;
	struct FE_node_field_creator *node_field_creator;

	ENTER(define_FE_field_at_node_simple);
	if (node && field &&
		(0 < (number_of_components = get_FE_field_number_of_components(field))) &&
		(0 <= number_of_derivatives) && nodal_value_types)
	{
		return_code = 1;
		if(node_field_creator = CREATE(FE_node_field_creator)(number_of_components))
		{
			for (n = 0; n < number_of_components; n++)
			{
				for (j = 0 ; j < number_of_derivatives ; j++)
				{
					FE_node_field_creator_define_derivative(node_field_creator, 
						/*component_number*/n, nodal_value_types[j + 1]);
				}
			}
			if (!define_FE_field_at_node(node, field, (struct FE_time_sequence *)NULL,
				node_field_creator))
			{
				display_message(ERROR_MESSAGE, "define_FE_field_at_node_simple.  "
					"Could not define field at node");
				return_code = 0;
			}
			DESTROY(FE_node_field_creator)(&(node_field_creator));
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_FE_field_at_node_simple.  ");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_FE_field_at_node_simple.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_FE_field_at_node_simple */

struct FE_element *create_1d_hermite_element(struct FE_region *fe_region,
	int number_of_fields, struct FE_field **field_array)
/*******************************************************************************
LAST MODIFIED : 29 March 2006

DESCRIPTION :
Creates and returns a 1-D line element with <field_array> defined using a 1-D cubic
Hermite basis over it.
==============================================================================*/
{
	int basis_type[2] = {1, CUBIC_HERMITE};
	int shape_type[1] = {LINE_SHAPE};
	int i, j, maximum_number_of_components, n, number_of_components, 
		number_of_nodes, number_of_scale_factors, return_code;
	struct CM_element_information cm;
	struct FE_basis *element_basis;
	struct FE_element *element;
	struct FE_element_field_component *component, **components;
	struct FE_element_shape *element_shape;
	struct FE_field *fe_field;
	struct MANAGER(FE_basis) *basis_manager;
	struct Standard_node_to_element_map *standard_node_map;

	ENTER(create_1d_hermite_element);
	element = (struct FE_element *)NULL;
	if (fe_region && (basis_manager = FE_region_get_basis_manager(fe_region)) &&
		(0 < number_of_fields) && field_array)
	{
		return_code = 1;

		/* make 1-d line shape */
		element_shape = CREATE(FE_element_shape)(/*dimension*/1, shape_type, fe_region);
		/* make 1-d cubic Hermite basis */
		element_basis = make_FE_basis(basis_type, basis_manager);

		cm.type = CM_ELEMENT;
		cm.number = 0;
		if (element = CREATE(FE_element)(&cm, element_shape, fe_region,
			/*template*/(struct FE_element *)NULL))
		{
			number_of_scale_factors = 4;
			number_of_nodes = 2;
			if (set_FE_element_number_of_nodes(element, number_of_nodes) &&
				set_FE_element_number_of_scale_factor_sets(element,
					/*number_of_scale_factor_sets*/1,
					/*scale_factor_set_identifiers*/(void *)&element_basis,
					/*numbers_in_scale_factor_sets*/&number_of_scale_factors))
			{
				/* set scale factors to 1.0 */
				for (i = 0; i < number_of_scale_factors; i++)
				{
					if (!set_FE_element_scale_factor(element, i, 1.0))
					{
						return_code = 0;
					}
				}

				maximum_number_of_components = get_FE_field_number_of_components(
					field_array[0]);
				for (i = 0 ; i < number_of_fields ; i++)
				{
					number_of_components = get_FE_field_number_of_components(
						field_array[i]);
					if (number_of_components > maximum_number_of_components)
					{
						maximum_number_of_components = number_of_components;
					}
				}
				
				if (ALLOCATE(components, struct FE_element_field_component *,
						maximum_number_of_components))
				{
					for (n = 0; n < maximum_number_of_components; n++)
					{
						components[n] = (struct FE_element_field_component *)NULL;
					}
					for (n = 0; (n < maximum_number_of_components) && element; n++)
					{
						if (component = CREATE(FE_element_field_component)(
								 STANDARD_NODE_TO_ELEMENT_MAP, number_of_nodes,
							element_basis, (FE_element_field_component_modify)NULL))
						{
							for (j = 0; j < number_of_nodes; j++)
							{
								if (standard_node_map = CREATE(Standard_node_to_element_map)(
									/*node_index*/j, /*number_of_values*/2))
								{
									if (!(Standard_node_to_element_map_set_nodal_value_index(
										standard_node_map,
										/*nodal_value_number*/0, /*nodal_value_index*/0) &&
										Standard_node_to_element_map_set_scale_factor_index(
											standard_node_map,
											/*nodal_value_number*/0, /*scale_factor_index*/j*2) &&
										Standard_node_to_element_map_set_nodal_value_index(
											standard_node_map,
											/*nodal_value_number*/1, /*nodal_value_index*/1) &&
										Standard_node_to_element_map_set_scale_factor_index(
											standard_node_map,
											/*nodal_value_number*/1, /*scale_factor_index*/j*2 + 1) &&
										FE_element_field_component_set_standard_node_map(
											component, /*node_number*/j, standard_node_map)))
									{
										return_code = 0;
									}
								}
								else
								{
									return_code = 0;
								}
							}
						}
						else
						{
							return_code = 0;
						}
						components[n] = component;
					}
					if (return_code)
					{
						for (i = 0 ; (i < number_of_fields) && return_code ; i++)
						{
							if (!define_FE_field_at_element(element, field_array[i], components))
							{
								display_message(ERROR_MESSAGE,"create_1d_hermite_element.  "
									"Could not define field on element");
								return_code = 0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_1d_hermite_element.  Could not create components");
					}
					for (n = 0; n < maximum_number_of_components; n++)
					{
						DESTROY(FE_element_field_component)(&(components[n]));
					}
					DEALLOCATE(components);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_1d_hermite_element.  Could not allocate components");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_1d_hermite_element.  "
					"Could not set element shape and field info");
				return_code = 0;
			}
			if (!return_code)
			{
				DESTROY(FE_element)(&element);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_1d_hermite_element.  Could not create element");
		}
		/* deaccess basis and shape so at most used by template element */
		if (element_basis)
		{
			DESTROY(FE_basis)(&element_basis);
		}
		if (element_shape)
		{
			DESTROY(FE_element_shape)(&element_shape);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_1d_hermite_element.  Invalid argument(s)");
	}
	LEAVE;

	return (element);
} /* create_1d_hermite_element */

struct FE_field_initialise_array_data
{
	int number_of_components;
	int fe_field_index;
	struct FE_field **fe_field_array;
}; /* struct FE_field_initialise_array_data */

static int FE_field_initialise_array(struct FE_field *field,
	void *fe_field_initialise_array_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2006

DESCRIPTION :
Iterator function for adding <field> to <field_list> if not currently in it.
==============================================================================*/
{
	int return_code;
	struct FE_field_initialise_array_data *data;

	ENTER(FE_field_initialise_array);
	if (field && (data = (struct FE_field_initialise_array_data *)fe_field_initialise_array_data_void))
	{
		data->number_of_components += get_FE_field_number_of_components(field);
		data->fe_field_array[data->fe_field_index] = field;
		data->fe_field_index++;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_initialise_array.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_initialise_array */

/*
Global functions
----------------
*/

int create_FE_element_snake_from_data_points(
	struct FE_region *fe_region, struct Computed_field *coordinate_field,
	int number_of_fitting_fields, struct Computed_field **fitting_fields,
	struct LIST(FE_node) *data_list,
	int number_of_elements, FE_value density_factor, FE_value stiffness)
/*******************************************************************************
LAST MODIFIED : 29 March 2006

DESCRIPTION :
Creates a snake out of <number_of_elements> 1-D cubic Hermite elements in
<element_manager> and <element_group>, nodes in <node_manager> and the node
group of the same name in <node_group_manager>. The snake follows the data in
<data_list>. <data_list> is unmodified by this function.
The <fitting_fields> which must be defined on the data are fitted and
defined on the created elements.  The <coordinate_field> is used to determine
distances between points.  The <density_factor> can vary from 0 to 1.
When <density_factor> is 0 then the elements are spread out to have the same
length in this coordinate field, when the <density_factor> is 1 then the
elements will each correspond to an equal proportion of the data points.
A positive value of <stiffness> penalises solutions with large second
derivatives; helps make smooth snakes from few data points.
==============================================================================*/
{
	double d, d2phi_dxi2[4], d2phi_dxi2_m, dxi_ds, dxi_ds_4, double_stiffness,
		double_xi, *force_vectors, phi[4], phi_m, *pos, *stiffness_matrix,
		*stiffness_offset, weight;
	enum FE_nodal_value_type hermite_1d_nodal_value_types[2] =
		{FE_NODAL_VALUE, FE_NODAL_D_DS1};
	FE_value *coordinates, *fitting_field_values, density_multiplier,
		length_multiplier, *lengths, *value, xi;
	int component, element_number, i, *indx, j, local_components, m, n, node_number, 
		number_of_components, number_of_coordinate_components, number_of_data,
		number_of_fe_fields, number_of_rows, return_code, row, start_row;
	struct CM_element_information cm;
	struct FE_element *element, *template_element;
	struct FE_node **nodes, *template_node;
 	struct FE_node_accumulate_length_data accumulate_data;
 	struct FE_field_initialise_array_data initialise_array_data;
	struct FE_field **fe_field_array;
	struct LIST(FE_field) *fe_field_list, *additional_fe_field_list;

	ENTER(create_FE_element_snake_from_data_points);
	if (fe_region && coordinate_field && 
		(number_of_fitting_fields > 0) && fitting_fields && data_list &&
		(0 < number_of_elements) &&
		(0.0 <= density_factor) && (1.0 >= density_factor) &&
		(0.0 <= (double_stiffness = (double)stiffness)))
	{
		return_code = 1;

		fe_field_list = Computed_field_get_defining_FE_field_list(fitting_fields[0]);
		for (i = 1 ; i < number_of_fitting_fields ; i++)
		{
			additional_fe_field_list = Computed_field_get_defining_FE_field_list(fitting_fields[i]);
			FOR_EACH_OBJECT_IN_LIST(FE_field)(ensure_FE_field_is_in_list,
				(void *)fe_field_list, additional_fe_field_list);
			DESTROY(LIST(FE_field))(&additional_fe_field_list);
		}
		
		number_of_fe_fields = NUMBER_IN_LIST(FE_field)(fe_field_list);
		if (ALLOCATE(fe_field_array, struct FE_field *, number_of_fe_fields))
		{
			initialise_array_data.number_of_components = 0;
			initialise_array_data.fe_field_index = 0;
			initialise_array_data.fe_field_array = fe_field_array;
			FOR_EACH_OBJECT_IN_LIST(FE_field)(FE_field_initialise_array,
				(void *)&initialise_array_data, fe_field_list);
			number_of_components = initialise_array_data.number_of_components;

			number_of_coordinate_components = Computed_field_get_number_of_components(coordinate_field);

			fitting_field_values = (FE_value *)NULL;
			lengths = (FE_value *)NULL;
			coordinates = (FE_value *)NULL;
			stiffness_matrix = (double *)NULL;
			force_vectors = (double *)NULL;
			indx = (int *)NULL;
			/* 1. Make table of lengths from first data point up to last */
			if (0 < number_of_components)
			{
				if (1 < (number_of_data = NUMBER_IN_LIST(FE_node)(data_list)))
				{
					if (ALLOCATE(lengths, FE_value, number_of_data) &&
						ALLOCATE(fitting_field_values, FE_value, number_of_components*number_of_data) &&
						ALLOCATE(coordinates, FE_value, 2 * number_of_coordinate_components))
					{
						accumulate_data.fitting_field_values = fitting_field_values;
						accumulate_data.lengths = lengths;
						accumulate_data.coordinates = coordinates;
						accumulate_data.node_number = 0;
						accumulate_data.coordinate_field = coordinate_field;
						accumulate_data.fe_field_list = fe_field_list;
						if (FOR_EACH_OBJECT_IN_LIST(FE_node)(FE_node_accumulate_length,
								(void *)&accumulate_data, data_list))
						{
							if (0.0 < lengths[number_of_data - 1])
							{
								if ((0.0 >= stiffness) &&
									((2 + number_of_elements*2) > number_of_data))
								{
									display_message(ERROR_MESSAGE,
										"create_FE_element_snake_from_data_points.  "
										"Not enough data; add more data or a stiffness");
									return_code = 0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_FE_element_snake_from_data_points.  Zero length");
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_FE_element_snake_from_data_points.  "
								"Could not calculate lengths");
							return_code = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_FE_element_snake_from_data_points.  "
							"Could not allocate lengths");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_FE_element_snake_from_data_points.  "
						"Not enough data points");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_FE_element_snake_from_data_points.  "
					"No values to fit for defining FE_fields of fitting_fields list.");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_FE_element_snake_from_data_points.  "
				"Unable to allocate fe_field array.");
			return_code = 0;
		}
		if (return_code)
		{
#if defined (DEBUG)
			/*???debug*/
			for (i = 0; i < number_of_data; i++)
			{
				printf("length[%d] = %f\n", i, lengths[i]);
			}
#endif /* defined (DEBUG) */
			/* convert lengths array into combined element:xi coordinates. The integer
				 part of the number is the element number, the fractional part is Xi */
			/* density_factor controls the density of elements along the snake,
				 together with the overall length and density of data points. Values:
				 0.0 = elements/xi are evenly spaced along the lengths;
				 1.0 = elements/xi are evenly spaced along the point numbers.
				 Values from 0.0 change the behaviour continuously between the two */
			length_multiplier = (FE_value)number_of_elements /
				accumulate_data.lengths[number_of_data - 1];
			density_multiplier =
				(FE_value)number_of_elements / (FE_value)(number_of_data - 1);
			for (i = 0; i < number_of_data; i++)
			{
				lengths[i] = (1.0 - density_factor)*lengths[i]*length_multiplier +
					density_factor*(FE_value)i*density_multiplier;
			}
#if defined (DEBUG)
			/*???debug*/
			for (i = 0; i < number_of_data; i++)
			{
				printf("element:xi[%d] = %f\n", i, accumulate_data.lengths[i]);
			}
#endif /* defined (DEBUG) */
			/* allocate stiffness_matrix with (number_of_elements + 1)*2 rows*columns,
				 and the force_vectors with (number_of_elements + 1)*2 rows for each
				 coordinate component. The factor or 2 adds the derivative DOFs for
				 Hermite basis functions.
				 Note that the same stiffness_matrix is used to fit each coordinate
				 component, but there is one force/RHS vector per component.
				 The indx array is required for LU_decompose */
			number_of_rows = 2*(number_of_elements + 1);
			if (ALLOCATE(stiffness_matrix, double, number_of_rows*number_of_rows) &&
				ALLOCATE(force_vectors, double, number_of_components*number_of_rows) &&
				ALLOCATE(indx, int, number_of_rows))
			{
				/* clear the stiffness_matrix and force_vectors */
				pos = stiffness_matrix;
				for (i = number_of_rows*number_of_rows; 0 < i; i--)
				{
					*pos = 0.0;
					pos++;
				}
				pos = force_vectors;
				for (i = number_of_components*number_of_rows; 0 < i; i--)
				{
					*pos = 0.0;
					pos++;
				}
				/* assemble the stiffness matrix and force_vectors */
				value = fitting_field_values;
				for (i = 0; i < number_of_data; i++)
				{
					element_number = (int)lengths[i];
					/* handle points at the end of the line specially */
					if (element_number >= number_of_elements)
					{
						element_number = number_of_elements - 1;
						xi = 1.0;
					}
					else
					{
						xi = lengths[i] - element_number;
					}
					start_row = element_number*2;
					calculate_Hermite_basis_1d(xi, phi);
					for (m = 0; m < 4; m++)
					{
						phi_m = phi[m];
						row = start_row + m;
						stiffness_offset =
							stiffness_matrix + row*number_of_rows + start_row;
						for (n = 0; n < 4; n++)
						{
							stiffness_offset[n] += phi_m*phi[n];
						}
						for (n = 0; n < number_of_components; n++)
						{
							force_vectors[n*number_of_rows + row] += phi_m * value[n];
						}
					}
					value += number_of_components;
				}
				/* add stiffness penalising second derivatives, if non-zero */
				if (0.0 < double_stiffness)
				{
					dxi_ds = (double)number_of_elements /
						(double)lengths[number_of_data - 1];
					dxi_ds_4 = dxi_ds*dxi_ds*dxi_ds*dxi_ds;
					for (element_number = 0; element_number < number_of_elements;
						element_number++)
					{
						start_row = element_number*2;
						for (i = 0; i < 4; i++)
						{
							weight = dxi_ds_4*double_stiffness*gauss_weight[i];
							double_xi = gauss_point[i] + 0.5;
							calculate_Hermite_basis_1d_second_derivatives(double_xi,
								d2phi_dxi2);
							for (m = 0; m < 4; m++)
							{
								d2phi_dxi2_m = d2phi_dxi2[m]*weight;
								row = start_row + m;
								stiffness_offset =
									stiffness_matrix + row*number_of_rows + start_row;
								for (n = 0; n < 4; n++)
								{
									stiffness_offset[n] += d2phi_dxi2_m*d2phi_dxi2[n];
								}
							}
						}
					}
				}
#if defined (DEBUG)
				/*???debug*/
				printf("Stiffness Matrix:\n");
				print_matrix(number_of_rows,number_of_rows,stiffness_matrix,"%8.4f");
				for (n = 0; n < number_of_components; n++)
				{
					printf("Force Vector %d:\n", n + 1);
					print_matrix(number_of_rows,1,
						force_vectors + n*number_of_rows,"%8.4f");
				}
#endif /* defined (DEBUG) */
				if (LU_decompose(number_of_rows, stiffness_matrix, indx, &d,/*singular_tolerance*/1.0e-12))
				{
					for (n = 0; (n < number_of_components) && return_code; n++)
					{
						if (!LU_backsubstitute(number_of_rows, stiffness_matrix, indx,
							force_vectors + n*number_of_rows))
						{
							display_message(ERROR_MESSAGE,
								"create_FE_element_snake_from_data_points.  "
								"Could not get solution for force_vector %d", n + 1);
							return_code = 0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_FE_element_snake_from_data_points.  "
						"Singular stiffness matrix");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_FE_element_snake_from_data_points.  "
					"Could not allocate stiffness matrix and force vector for fitting");
				return_code = 0;
			}
		}
		if (return_code)
		{
			FE_region_begin_change(fe_region);
#if defined (DEBUG)
			/*???debug*/
			for (n = 0; n < number_of_components; n++)
			{
				printf("Solution: Coordinate %d:\n", n + 1);
				print_matrix(number_of_rows,1,
					force_vectors + n*number_of_rows,"%8.4f");
			}
#endif /* defined (DEBUG) */
			template_node = (struct FE_node *)NULL;
			template_element = (struct FE_element *)NULL;
			nodes = (struct FE_node **)NULL;
			if (ALLOCATE(nodes, struct FE_node *, number_of_elements + 1))
			{
				for (j = 0; j <= number_of_elements; j++)
				{
					nodes[j] = (struct FE_node *)NULL;
				}
				/* create a template node suitable for 1-D Hermite interpolation of the
					 coordinate_field */
				if (template_node = CREATE(FE_node)(/*node_number*/0, fe_region,
					/*template_node*/(struct FE_node *)NULL))
				{
					for (n = 0 ; return_code && (n < number_of_fe_fields) ; n++)
					{
						return_code = define_FE_field_at_node_simple(template_node,
							fe_field_array[n], 
							/*number_of_derivatives*/1, hermite_1d_nodal_value_types);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_FE_element_snake_from_data_points.  "
						"Could not create template_node");
					return_code = 0;
				}
				/* create the nodes in the snake as copies of the template, access them
					 in the nodes array, set their coordinates and derivatives and add
					 them to the manager and node_group */
				node_number = 1;
				for (j = 0; (j <= number_of_elements) && return_code; j++)
				{
					/* get next unused node number from fe_region */
					node_number = FE_region_get_next_FE_node_identifier(fe_region,
						node_number);
					if (nodes[j] = CREATE(FE_node)(node_number, (struct FE_region *)NULL,
						template_node))
					{
						/* set the coordinate and derivatives */
						component = 0;
						for (n = 0; (n < number_of_fe_fields) && return_code; n++)
						{
							local_components = get_FE_field_number_of_components(fe_field_array[n]);
							for (i = 0 ; (i < local_components) && return_code ; i++)
							{
								if (!(set_FE_nodal_FE_value_value(nodes[j], fe_field_array[n],
									/*component_number*/i, /*version*/0, FE_NODAL_VALUE,
									/*time*/0, force_vectors[component*number_of_rows + j*2]) &&
									set_FE_nodal_FE_value_value(nodes[j], fe_field_array[n],
									/*component_number*/i,/*version*/0, FE_NODAL_D_DS1, /*time*/0, 
									force_vectors[component*number_of_rows + j*2 + 1])))
								{
									display_message(ERROR_MESSAGE,
										"create_FE_element_snake_from_data_points.  "
										"Could not set coordinates or derivatives");
									return_code = 0;
								}
								component++;
							}
						}
						if (return_code)
						{
							if (!FE_region_merge_FE_node(fe_region, nodes[j]))
							{
								display_message(ERROR_MESSAGE,
									"create_FE_element_snake_from_data_points.  "
									"Could not merge node into region");
								return_code = 0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_FE_element_snake_from_data_points.  "
							"Could not add create node from template");
						return_code = 0;
					}
				}
				if (return_code)
				{
					if (template_element = create_1d_hermite_element(fe_region,
						number_of_fe_fields, fe_field_array))
					{
						cm.type = CM_ELEMENT;
						cm.number = 1;
						for (j = 0; (j < number_of_elements) && return_code; j++)
						{
							/* get next unused element identifier from fe_region */
							cm.number = FE_region_get_next_FE_element_identifier(
								fe_region, cm.type, cm.number);
							if (element = CREATE(FE_element)(&cm,
								(struct FE_element_shape *)NULL, (struct FE_region *)NULL,
								template_element))
							{
								ACCESS(FE_element)(element);
								if (!(set_FE_element_node(element, 0, nodes[j]) &&
									set_FE_element_node(element, 1, nodes[j + 1])))
								{
									return_code = 0;
								}
								if (return_code)
								{
									if (!FE_region_merge_FE_element(fe_region, element))
									{
										display_message(ERROR_MESSAGE,
											"create_FE_element_snake_from_data_points.  "
											"Could not merge element into region");
										return_code = 0;
									}
								}
								DEACCESS(FE_element)(&element);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_FE_element_snake_from_data_points.  "
									"Could not create element");
								return_code = 0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_FE_element_snake_from_data_points.  "
							"Could not create template element");
						return_code = 0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_FE_element_snake_from_data_points.  "
					"Could not allocate nodes array");
				return_code = 0;
			}
			if (nodes)
			{
				DEALLOCATE(nodes);
			}
			if (template_node)
			{
				DESTROY(FE_node)(&template_node);
			}
			if (template_element)
			{
				DESTROY(FE_element)(&template_element);
			}
			FE_region_end_change(fe_region);
		}
		if (fitting_field_values)
		{
			DEALLOCATE(fitting_field_values);
		}
		if (lengths)
		{
			DEALLOCATE(lengths);
		}
		if (coordinates)
		{
			DEALLOCATE(coordinates);
		}
		if (stiffness_matrix)
		{
			DEALLOCATE(stiffness_matrix);
		}
		if (force_vectors)
		{
			DEALLOCATE(force_vectors);
		}
		if (indx)
		{
			DEALLOCATE(indx);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_FE_element_snake_from_data_points.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* create_FE_element_snake_from_data_points */
