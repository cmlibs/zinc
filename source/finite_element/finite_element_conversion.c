/*******************************************************************************
FILE : finite_element_conversion.c

LAST MODIFIED : 5 April 2006

DESCRIPTION :
Functions for converting one finite_element representation to another.
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
/* for IGES */
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_wrappers.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_conversion.h"
#include "general/debug.h"
#include "general/enumerator_private.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Convert_finite_elements_data
{
	enum Convert_finite_elements_mode mode;
	struct FE_node *template_node;
	struct FE_element *template_element;
	int number_of_fields;
	struct FE_field **fields;
	int maximum_number_of_components;
	FE_value *temporary_values;
	struct FE_region *destination_fe_region;

	int node_number; /* Do not to search from beginning for a valid node number each time */
	int element_number; /* Do not to search from beginning for a valid element number each time */
}; /* struct Convert_finite_elements_data */

/*
Module functions
----------------
*/

static int FE_field_initialise_array(struct FE_field *field,
	void *fe_field_initialise_array_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2006

DESCRIPTION :
Iterator function for adding <field> to <field_list> if not currently in it.
==============================================================================*/
{
	int number_of_components, return_code;
	struct Convert_finite_elements_data *data;

	ENTER(FE_field_initialise_array);
	if (field && (data = (struct Convert_finite_elements_data *)fe_field_initialise_array_data_void))
	{
		data->fields[data->number_of_fields] = field;
		data->number_of_fields++;
		number_of_components = get_FE_field_number_of_components(field);
		if (number_of_components > data->maximum_number_of_components)
		{
			data->maximum_number_of_components = number_of_components;
		}
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

static int FE_element_convert_element(struct FE_element *element,
	void *data_void)
/******************************************************************************
LAST MODIFIED : 5 April 2006

DESCRIPTION :
Converts the element as specified by data->mode and adds it to 
data->destination_fe_field.
==============================================================================*/
{
#define HERMITE_2D_NUMBER_OF_NODES (4)
	FE_value xi[HERMITE_2D_NUMBER_OF_NODES][2] = 
		{{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}};
	FE_value *values, *derivatives, *nodal_values;
	int dimension, i, j, k, number_of_components, number_of_values, return_code;
	struct Convert_finite_elements_data *data;
	struct CM_element_information identifier;
	struct FE_element *new_element;
	struct FE_element_field_values *fe_element_field_values;
	struct FE_field *field;
	struct FE_node *nodes[HERMITE_2D_NUMBER_OF_NODES];

	ENTER(FE_element_convert_element);

	return_code = 1;
	if (element && (data = (struct Convert_finite_elements_data *)data_void))
	{
		dimension = get_FE_element_dimension(element);
		switch (data->mode)
		{
			case CONVERT_TO_FINITE_ELEMENTS_HERMITE_2D_PRODUCT:
			{
				if (2 == dimension)
				{
					for (i = 0 ; return_code && (i < HERMITE_2D_NUMBER_OF_NODES) ; i++)
					{
						data->node_number = FE_region_get_next_FE_node_identifier(
							data->destination_fe_region, data->node_number);
						if (nodes[i] = CREATE(FE_node)(data->node_number, (struct FE_region *)NULL,
								data->template_node))
						{
							if (!FE_region_merge_FE_node(data->destination_fe_region, nodes[i]))
							{
								display_message(ERROR_MESSAGE,
									"FE_element_convert_element.  "
									"Could not merge node into region");
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"FE_element_convert_element.  "
								"Unable to create node.");
							return_code=0;
						}
					}
					for (j = 0 ; j < data->number_of_fields ; j++)
					{
						field = data->fields[j];
						number_of_components = get_FE_field_number_of_components(field);
						if ((fe_element_field_values = CREATE(FE_element_field_values)()) &&
							calculate_FE_element_field_values(element,
								field, /*time*/0.0, /*calculate_derivatives*/1,
								fe_element_field_values, /*top_level_element*/(struct FE_element *)NULL))
						{
							number_of_values = 4 * number_of_components;
							values = data->temporary_values;
							derivatives = data->temporary_values + number_of_components;
							nodal_values = data->temporary_values + number_of_values;
							for (i = 0 ; return_code && (i < HERMITE_2D_NUMBER_OF_NODES) ; i++)
							{
								calculate_FE_element_field(/*all components*/-1,
									fe_element_field_values, xi[i], values, derivatives);
								/* Reorder the separate lists of values and derivatives into
									a single mixed list */
								for (k = 0 ; k < number_of_components ; k++)
								{
									nodal_values[4 * k] = values[k];
									nodal_values[4 * k + 1] = derivatives[2 * k];
									nodal_values[4 * k + 2] = derivatives[2 * k + 1];
									nodal_values[4 * k + 3] = nodal_values[4 * k + 1] * nodal_values[4 * k + 2];
								}
								set_FE_nodal_field_FE_value_values(data->fields[j],
									nodes[i], nodal_values, &number_of_values);
							}
							
							DESTROY(FE_element_field_values)(&fe_element_field_values);
						}
						else
						{
							display_message(ERROR_MESSAGE,"FE_element_convert_element.  "
								"Unable to calculate_FE_element_field_values.");
							return_code=0;
						}
					}
					identifier.type = CM_ELEMENT;
					identifier.number =  FE_region_get_next_FE_element_identifier(
						data->destination_fe_region, identifier.type, data->element_number);
					data->element_number = identifier.number;
					if (new_element = CREATE(FE_element)(&identifier, (struct FE_element_shape *)NULL,
							(struct FE_region *)NULL, data->template_element))
					{
						/* The FE_element_define_tensor_product_basis function does not merge the
							nodes used to make it simple to add and a field to and existing node so
							we have to add the nodes for every field. */
						for (j = 0 ; return_code && (j < data->number_of_fields) ; j++)
						{
							for (i = 0 ; return_code && (i < HERMITE_2D_NUMBER_OF_NODES) ; i++)
							{
								if (!set_FE_element_node(new_element, j * HERMITE_2D_NUMBER_OF_NODES + i,
									nodes[i]))
								{
									display_message(ERROR_MESSAGE,"FE_element_convert_element.  "
										"Unable to set element node.");
									return_code = 0;
								}
							}
						}
						if (return_code)
						{
							if (!FE_region_merge_FE_element(data->destination_fe_region,
								new_element))
							{
								display_message(ERROR_MESSAGE,
									"FE_element_convert_element.  "
									"Could not merge node into region");
								return_code = 0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"FE_element_convert_element.  "
							"Unable to create element.");
						return_code=0;
					}
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"FE_element_convert_element.  "
					"Invalid or unimplemented conversion mode.");
				return_code=0;
			} break;
		}
	}
	LEAVE;
	
	return (return_code);
} /* FE_element_convert_element */
	
/*
Global functions
----------------
*/

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Convert_finite_elements_mode)
{
	char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Convert_finite_elements_mode));
	switch (enumerator_value)
	{
		case CONVERT_TO_FINITE_ELEMENTS_HERMITE_2D_PRODUCT:
		{

			enumerator_string = "convert_hermite_2D_product_elements";
		} break;
		default:
		{
			enumerator_string = (char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Convert_finite_elements_mode) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Convert_finite_elements_mode)

int finite_element_conversion(struct FE_region *source_fe_region, 
	struct FE_region *destination_fe_region,
	enum Convert_finite_elements_mode mode, int number_of_fields, 
	struct Computed_field **field_array)
/******************************************************************************
LAST MODIFIED : 5 April 2006

DESCRIPTION :
Convert the finite_elements in <source_fe_region> to new finite_elements
in <destination_fe_region> according to the <mode> defining the fields
in <field_array>.
==============================================================================*/
{
	enum FE_nodal_value_type hermite_2d_nodal_value_types[] = {
		FE_NODAL_D_DS1,  FE_NODAL_D_DS2, FE_NODAL_D2_DS1DS2};
	int i, return_code;
	struct Convert_finite_elements_data data;
	struct LIST(FE_field) *fe_field_list;

	ENTER(finite_element_conversion);

	return_code=0;
	if (source_fe_region && destination_fe_region && (0 < number_of_fields) &&
		(field_array))
	{
		fe_field_list = Computed_field_array_get_defining_FE_field_list(
			number_of_fields, field_array);

		data.mode = mode;
		data.template_node = (struct FE_node *)NULL;
		data.template_element = (struct FE_element *)NULL;
		data.number_of_fields = 0;  /* This will be incremented by the iterator */
		data.fields = ALLOCATE(data.fields, struct FE_field *, 
			NUMBER_IN_LIST(FE_field)(fe_field_list));
		data.maximum_number_of_components = 0; /* Initialised by iterator */
		data.temporary_values = (FE_value *)NULL; 
		FOR_EACH_OBJECT_IN_LIST(FE_field)(FE_field_initialise_array,
			(void *)&data, fe_field_list);
		data.destination_fe_region = destination_fe_region;
		data.node_number = 1;
		data.element_number = 1;

		/* Set up data */
		switch (mode)
		{
			case CONVERT_TO_FINITE_ELEMENTS_HERMITE_2D_PRODUCT:
			{
				if (ALLOCATE(data.temporary_values, FE_value,
						/*dofs*/2 * 4 * data.maximum_number_of_components))
				{
					return_code = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE,"finite_element_conversion.  "
						"Unable to allocate temporary values.");
					return_code=0;
				}
				if (return_code)
				{
					/* Make template node defining all the fields in the field list */
					if (data.template_node = CREATE(FE_node)(/*node_number*/0, destination_fe_region,
							/*template_node*/(struct FE_node *)NULL))
					{
						for (i = 0 ; return_code && (i < data.number_of_fields) ; i++)
						{
							return_code = define_FE_field_at_node_simple(data.template_node,
								data.fields[i], /*number_of_derivatives*/3, hermite_2d_nodal_value_types);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"finite_element_conversion.  "
							"Unable to make hermite template node.");
						return_code=0;
					}
				}
				if (return_code)
				{
					/* Make template element defining all the fields in the field list */
					if (data.template_element = create_FE_element_with_line_shape(/*element_number*/1,
						destination_fe_region, /*dimension*/2))
					{
						for (i = 0 ; return_code && (i < data.number_of_fields) ; i++)
						{
							return_code = FE_element_define_tensor_product_basis(data.template_element,
								/*dimension*/2, CUBIC_HERMITE, data.fields[i]);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"finite_element_conversion.  "
							"Unable to make hermite template node.");
						return_code=0;
					}
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"finite_element_conversion.  "
					"Invalid or unimplemented conversion mode.");
				return_code=0;
			} break;
		}

		FE_region_begin_change(destination_fe_region);

		/* For each node, not done yet as disjoint doesn't need it
		return_code = FE_region_for_each_FE_node(source_fe_region,
		   FE_element_convert_node, (void *)&data); */

		if (return_code)
		{
			/* For each element */
			return_code = FE_region_for_each_FE_element(source_fe_region,
				FE_element_convert_element, (void *)&data); 
		}

		FE_region_end_change(destination_fe_region);

		/* Clean up data */
		if (data.template_node)
		{
			DESTROY(FE_node)(&data.template_node);
		}
		if (data.temporary_values)
		{
			DEALLOCATE(data.temporary_values);
		}
		DESTROY(LIST(FE_field))(&fe_field_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,"finite_element_conversion.  "
			"Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* finite_element_conversion */
