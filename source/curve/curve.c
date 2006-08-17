/*******************************************************************************
FILE : curve.c

LAST MODIFIED : 16 April 2003

DESCRIPTION :
Definition of struct Curve used to describe time-value or x-y functions.
Simulation/animation parameters are controlled over time by these curves.
The Curve structure is private, and functions are defined to access its
contents. Internally, all effort has been put into providing a level of
abstraction between what the user sees and the finite element/node/field
structures used internally. It is designed to be flexible rather than fast.
Note that 4 basis types are currently supported: cubic Hermite, linear Lagrange,
quadratic Lagrange, cubic Lagrange.
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
#include <stdlib.h>
#include <stdio.h>
#include "command/parser.h"
#include "curve/curve.h"
#include "general/object.h"
#include "finite_element/export_finite_element.h"
#include "finite_element/finite_element.h"
#include "finite_element/import_finite_element.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "user_interface/message.h"

/*
Global curves
----------------
*/
/*
Module types
------------
*/

struct Curve
/*******************************************************************************
LAST MODIFIED : 22 November 1999

DESCRIPTION :
Stores a function returning a number or N-vector over a range of parameters.
This structure is private; use functions to access its contents.
It is designed to be flexible rather than fast.
==============================================================================*/
{
	char *name;
	enum FE_basis_type fe_basis_type;
	int number_of_components;
	enum Curve_extend_mode extend_mode;
	enum Curve_type type;
	int value_nodes_per_element,value_derivatives_per_node;

	/* each Curve is like an FE_region with its own name space */
	struct MANAGER(FE_basis) *basis_manager;
	struct LIST(FE_element_shape) *element_shape_list;
	struct FE_region *fe_region;
	struct FE_field *parameter_field,*value_field;
	struct FE_node *template_node;
	struct FE_element *template_element;

	/* information useful for editing Curve */
	FE_value *max_value,*min_value;
	FE_value parameter_grid,value_grid;

	/* cache for rapid parameter look-up */
	FE_value *parameter_table;
	int parameter_table_size;

	int access_count;
}; /* struct Curve */

FULL_DECLARE_INDEXED_LIST_TYPE(Curve);

FULL_DECLARE_MANAGER_TYPE(Curve);

/*
Module functions
----------------
*/

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Curve,name,char *,strcmp)

DECLARE_LOCAL_MANAGER_FUNCTIONS(Curve)

static struct FE_element *cc_get_element(struct Curve *curve,
	int element_no)
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Returns the element at position element_no in the curve.
==============================================================================*/
{
	struct FE_element *element;
	struct CM_element_information cm;

	ENTER(cc_get_element);
	if (curve)
	{
		cm.number=element_no;
		cm.type = CM_ELEMENT;
		element=FE_region_get_FE_element_from_identifier(curve->fe_region,&cm);
	}
	else
	{
		display_message(ERROR_MESSAGE,"cc_get_element.  Invalid argument(s)");
		element=(struct FE_element *)NULL;
	}
	LEAVE;

	return (element);
} /* cc_get_element */

static int cc_get_node_field_values(struct FE_node *node,struct FE_field *field,
	enum FE_nodal_value_type nodal_value_type,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Returns in [user pre-allocated] values array the field->number_of_components
values of <nodal_value_type> of <field> stored at <node>.
For Curves, <nodal_value_type> can be FE_NODAL_VALUE or FE_NODAL_D_DS1.
==============================================================================*/
{
	FE_value *value;
	int i,number_of_components,return_code;

	ENTER(cc_get_node_field_values);
	if (node&&field&&(value=values)&&
		(0<(number_of_components=get_FE_field_number_of_components(field))))
	{
		return_code=1;
		for (i=0;(i<number_of_components)&&return_code;i++)
		{
			if (!get_FE_nodal_FE_value_value(node,field,/*component_number*/i,
				/*version*/0,nodal_value_type,/*time*/0,value))
			{
				display_message(ERROR_MESSAGE,"cc_get_node_field_values.  "
					"Field/nodal value type not defined at node");
				return_code=0;
			}
			value++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cc_get_node_field_values.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cc_get_node_field_values */

static int cc_set_node_field_values(struct FE_node *node,struct FE_field *field,
	enum FE_nodal_value_type nodal_value_type,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Sets the field->number_of_components values of <nodal_value_type> of <field>
stored at <node>.
For Curves, <nodal_value_type> can be FE_NODAL_VALUE or FE_NODAL_D_DS1.
==============================================================================*/
{
	FE_value *value;
	int i,number_of_components,return_code;

	ENTER(cc_set_node_field_values);
	if (node&&field&&(value=values)&&
		(0<(number_of_components=get_FE_field_number_of_components(field))))
	{
		return_code=1;
		for (i=0;(i<number_of_components)&&return_code;i++)
		{
			if (!set_FE_nodal_FE_value_value(node,field,/*component_number*/i,
				/*version*/0, nodal_value_type, /*time*/0, *value))
			{
				display_message(ERROR_MESSAGE,"cc_set_node_field_values.  "
					"Field/nodal value type not defined at node");
				return_code=0;
			}
			value++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cc_set_node_field_values.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cc_set_node_field_values */

static int cc_calculate_element_field_values(struct FE_element *element,
	FE_value xi,struct FE_field *field,FE_value *values,FE_value *derivatives)
/*******************************************************************************
LAST MODIFIED : 18 March 2003

DESCRIPTION :
Calculates the <values> and <derivatives> of <field> at <element> <xi>.
Space for values and derivatives should have been allocated by calling function.
If <derivatives> is NULL, they will not be calculated.
==============================================================================*/
{
	int return_code;
	struct FE_element_field_values *element_field_values;

	ENTER(cc_calculate_element_field_values);
	if (element&&field&&values)
	{
		if ((element_field_values = CREATE(FE_element_field_values)()) &&
			calculate_FE_element_field_values(element,field,/*time*/0,
			((FE_value *)NULL != derivatives),element_field_values,
			/*top_level_element*/(struct FE_element *)NULL))
		{
			return_code=calculate_FE_element_field(-1,element_field_values,&xi,
				values,derivatives);
			DESTROY(FE_element_field_values)(&element_field_values);
		}
		else
		{
			return_code=0;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"cc_calculate_element_field_values.  Error calculating field");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cc_calculate_element_field_values.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cc_calculate_element_field_values */

static int cc_clear_parameter_table(struct Curve *curve)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Clears the quick reference table for the curve parameter values. Note that
Curve functions that add or remove nodes or change parameters should
call this after they have made their changes.
==============================================================================*/
{
	int return_code;

	ENTER(cc_clear_parameter_table);
	if (curve)
	{
		return_code=1;
		if (curve->parameter_table)
		{
			DEALLOCATE(curve->parameter_table);
			curve->parameter_table_size=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"cc_clear_parameter_table.  Missing curve");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cc_clear_parameter_table */

static int cc_build_parameter_table(struct Curve *curve)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Builds a quick reference table for the curve parameter values. Note that
Curve functions that add or remove nodes or change parameters should
call cc_clear_parameter_table. This function should be called as soon as the
table needs to be used when curve->parameter_table is NULL.
==============================================================================*/
{
	FE_value parameter;
	int i,node_no,node_number_increment,number_of_elements,return_code;
	struct FE_node *node;

	ENTER(cc_build_parameter_table);
	if (curve&&(!curve->parameter_table))
	{
		number_of_elements=Curve_get_number_of_elements(curve);
		if (0<number_of_elements)
		{
			curve->parameter_table_size = number_of_elements+1;
			if (ALLOCATE(curve->parameter_table,FE_value,curve->parameter_table_size))
			{
				return_code=1;
				node_no=1;
				node_number_increment=curve->value_nodes_per_element-1;
				for (i=0;(i<=number_of_elements)&&return_code;i++)
				{
					if ((node=FE_region_get_FE_node_from_identifier(curve->fe_region,
						node_no))&&
						cc_get_node_field_values(node,curve->parameter_field,
							FE_NODAL_VALUE,&parameter))
					{
						curve->parameter_table[i]=parameter;
						node_no += node_number_increment;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"cc_build_parameter_table.  Could not get node parameter");
						DEALLOCATE(curve->parameter_table);
						curve->parameter_table_size=0;
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"cc_build_parameter_table.  Not enough memory");
				return_code=0;
			}
		}
		else
		{
			/* no elements */;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cc_build_parameter_table.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cc_build_parameter_table */

static int cc_clean_up(struct Curve *curve)
/*******************************************************************************
LAST MODIFIED : 26 November 1999

DESCRIPTION :
Frees all dynamic or accessed information in <curve> except the name identifier.
Used for copy operations and as part of the DESTROY function.
==============================================================================*/
{
	int return_code;

	ENTER(cc_clean_up);
	if (curve)
	{
		DEACCESS(FE_element)(&(curve->template_element));
		DEACCESS(FE_node)(&(curve->template_node));
		DEACCESS(FE_field)(&curve->parameter_field);
		DEACCESS(FE_field)(&curve->value_field);
		DEACCESS(FE_region)(&curve->fe_region);
		DESTROY(MANAGER(FE_basis))(&curve->basis_manager);
		DESTROY(LIST(FE_element_shape))(&curve->element_shape_list);

		DEALLOCATE(curve->min_value);
		DEALLOCATE(curve->max_value);

		cc_clear_parameter_table(curve);

		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"cc_clean_up.  Missing curve");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cc_clean_up */

static struct Curve *cc_create_blank(char *name)
/*******************************************************************************
LAST MODIFIED : 26 November 1999

DESCRIPTION :
Creates a blank Curve with the given <name>, but no fe_basis_type, no
components, but all managers created. The curve must be modified to be valid,
but it is at least destroyable when returned from this function.
==============================================================================*/
{
	struct Curve *curve;

	ENTER(cc_create_blank);
	if (name)
	{
		if (ALLOCATE(curve,struct Curve,1))
		{
			if (ALLOCATE(curve->name,char,strlen(name)+1))
			{
				strcpy(curve->name,name);
			}
			curve->fe_basis_type=NO_RELATION;
			curve->number_of_components=0;
			curve->extend_mode=CONTROL_CURVE_EXTEND_CLAMP;
			curve->type=CONTROL_CURVE_TYPE_DEFAULT;
			curve->value_nodes_per_element=0;
			curve->value_derivatives_per_node=0;

			curve->basis_manager=CREATE(MANAGER(FE_basis))();
			curve->element_shape_list=CREATE(LIST(FE_element_shape))();
			curve->fe_region=ACCESS(FE_region)(CREATE(FE_region)
				(/*master_fe_region*/(struct FE_region *)NULL, curve->basis_manager,
				curve->element_shape_list));
			curve->parameter_field=(struct FE_field *)NULL;
			curve->value_field=(struct FE_field *)NULL;
			curve->template_node=(struct FE_node *)NULL;
			curve->template_element=(struct FE_element *)NULL;

			curve->max_value=(FE_value *)NULL;
			curve->min_value=(FE_value *)NULL;
			curve->parameter_grid=0.0;
			curve->value_grid=0.0;

			curve->parameter_table=(FE_value *)NULL;
			curve->parameter_table_size=0;

			curve->access_count=0;

			if (!(curve->name&&curve->basis_manager&&curve->element_shape_list&&curve->fe_region))
			{
				display_message(ERROR_MESSAGE,
					"cc_create_blank.  Could not create basis manager, element shape list and region");
				DESTROY(Curve)(&curve);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"cc_create_blank.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"cc_create_blank.  Invalid argument(s)");
		curve=(struct Curve *)NULL;
	}
	LEAVE;

	return (curve);
} /* cc_create_blank */

static int cc_establish(struct Curve *curve,
	enum FE_basis_type fe_basis_type,int number_of_components)
/*******************************************************************************
LAST MODIFIED : 26 November 1999

DESCRIPTION :
From a curve created with cc_create_blank, establishes its <fe_basis_type> and
<number_of_components>. Also allocates ranges for components and gives these
and grids default values.
Does not establish fields and template elements and nodes.
==============================================================================*/
{
	int i,return_code;

	ENTER(cc_establish);
	if (curve&&(NO_RELATION==curve->fe_basis_type)&&
		(0==curve->number_of_components))
	{
		if (0<number_of_components)
		{
			/* check fe_basis_type supported and get values for it */
			return_code=1;
			switch (fe_basis_type)
			{
				case CUBIC_HERMITE:
				{
					curve->value_nodes_per_element=2;
					curve->value_derivatives_per_node=1;
				} break;
				case LINEAR_LAGRANGE:
				{
					curve->value_nodes_per_element=2;
					curve->value_derivatives_per_node=0;
				} break;
				case QUADRATIC_LAGRANGE:
				{
					curve->value_nodes_per_element=3;
					curve->value_derivatives_per_node=0;
				} break;
				case CUBIC_LAGRANGE:
				{
					curve->value_nodes_per_element=4;
					curve->value_derivatives_per_node=0;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"cc_establish.  Unsupported FE_basis_type");
					return_code=0;
				} break;
			}
			if (return_code)
			{
				curve->fe_basis_type=fe_basis_type;
				curve->number_of_components=number_of_components;
				/* build dynamic contents of curve structure */
				if (ALLOCATE(curve->min_value,FE_value,number_of_components)&&
					ALLOCATE(curve->max_value,FE_value,number_of_components))
				{
					/* set values allocated above */
					for (i=0;i<number_of_components;i++)
					{
						curve->min_value[i]=0.0;
						curve->max_value[i]=1.0;
					}
					curve->parameter_grid=0.1;
					curve->value_grid=0.1;
				}
				else
				{
					display_message(ERROR_MESSAGE,"cc_establish.  Not enough memory");
					return_code=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cc_establish.  Invalid number of components");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"cc_establish.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cc_establish */

static int cc_add_duplicate_element(struct Curve *destination,
	struct Curve *source,int element_no)
/*******************************************************************************
LAST MODIFIED : 22 November 1999

DESCRIPTION :
Adds element <element_no> of <source> curve to <curve> making it as near as
possible to the source element if in a different basis or with different number
of components.
Ensures slope continuity in destination curve when cubic hermite basis.
==============================================================================*/
{
	FE_value *values,*derivatives,*left_derivatives,left_parameter,
		left_parameter_change,left_sf,parameter,parameter_change,sf,slope,
		right_parameter,temp_sf,*temp_values,xi;
	int comp_no,i,max_number_of_components,node_no,number_of_components,
		return_code;
	struct FE_node *destination_node,*left_node,*right_node,*source_node;
	struct FE_element *destination_element,*left_element,*source_element;

	ENTER(cc_add_duplicate_element);
	if (destination&&source&&
		(source_element=cc_get_element(source,element_no)))
	{
		max_number_of_components=
			number_of_components=destination->number_of_components;
		if (max_number_of_components<source->number_of_components)
		{
			max_number_of_components=source->number_of_components;
		}
		values=(FE_value *)NULL;
		derivatives=(FE_value *)NULL;
		left_derivatives=(FE_value *)NULL;
		temp_values=(FE_value *)NULL;
		if (ALLOCATE(values,FE_value,max_number_of_components)&&
			((0==Curve_get_derivatives_per_node(destination))||
				(ALLOCATE(derivatives,FE_value,max_number_of_components)&&
					ALLOCATE(left_derivatives,FE_value,max_number_of_components)&&
					ALLOCATE(temp_values,FE_value,max_number_of_components))))
		{
			/* clear new values in case number_of_components increasing */
			for (i=source->number_of_components;i<number_of_components;i++)
			{
				values[i]=0.0;
				if (derivatives)
				{
					derivatives[i]=0.0;
					left_derivatives[i]=0.0;
				}
			}
			if (Curve_add_element(destination,element_no)&&
				(destination_element=cc_get_element(destination,element_no)))
			{
				return_code=1;
				for (node_no=0;(node_no<destination->value_nodes_per_element)&&
					return_code;node_no++)
				{
					if (get_FE_element_node(destination_element,node_no,
						&destination_node))
					{
						left_element=(struct FE_element *)NULL;
						if (destination->fe_basis_type == source->fe_basis_type)
						{
							/* get values directly from the node */
							if (!(get_FE_element_node(source_element,node_no,&source_node)&&
								cc_get_node_field_values(source_node,source->parameter_field,
									FE_NODAL_VALUE,&parameter)&&
								cc_get_node_field_values(source_node,source->value_field,
									FE_NODAL_VALUE,values)&&
								((!derivatives)||(
									cc_get_node_field_values(source_node,source->value_field,
										FE_NODAL_D_DS1,derivatives)&&
									get_FE_element_scale_factor(source_element,node_no,&sf)))))
							{
								display_message(ERROR_MESSAGE,
									"cc_add_duplicate_element.  Could not get values from node");
								return_code=0;
							}
						}
						else
						{
							/* get values from the element */
							xi=(FE_value)node_no/
								(FE_value)(destination->value_nodes_per_element-1.0);
							if (Curve_get_parameter_in_element(source,element_no,xi,
								&parameter)&&
								cc_calculate_element_field_values(source_element,xi,
									source->value_field,values,derivatives))
							{
								if (derivatives)
								{
									Curve_unitize_vector(derivatives,
										number_of_components,&sf);

									if ((0==node_no)&&(1<element_no)&&
										cc_get_node_field_values(destination_node,
											destination->value_field,FE_NODAL_D_DS1,
											left_derivatives)&&
										(left_element=cc_get_element(destination,element_no-1)))
									{
										get_FE_element_scale_factor(left_element,
											destination->value_nodes_per_element-1,&left_sf);

										/* get parameter change in this element and left_element */
										get_FE_element_node(cc_get_element(source,element_no-1),
											0,&left_node);
										get_FE_element_node(source_element,
											source->value_nodes_per_element-1,&right_node);
										cc_get_node_field_values(left_node,
											source->parameter_field,FE_NODAL_VALUE,&left_parameter);
										cc_get_node_field_values(right_node,
											source->parameter_field,FE_NODAL_VALUE,&right_parameter);
										parameter_change=right_parameter-parameter;
										left_parameter_change=parameter-left_parameter;

										if (0.0 < left_parameter_change)
										{
											if (0.0 < parameter_change)
											{
												/* average slope with that currently in node */
												for (comp_no=0;comp_no<number_of_components;comp_no++)
												{
													derivatives[comp_no] =
														(derivatives[comp_no]*sf*parameter_change+
														left_derivatives[comp_no]*left_sf*
															left_parameter_change)/
														(parameter_change+left_parameter_change);
												}
												Curve_unitize_vector(derivatives,
													number_of_components,&temp_sf);
												if (0==temp_sf)
												{
													left_sf=sf=0.0;
												}
												else
												{
													slope = 0.5*(left_sf/left_parameter_change+
														sf/parameter_change);
													left_sf=slope*left_parameter_change;
													sf=slope*parameter_change;
												}
											}
											else
											{
												/* use left derivatives instead */
												for (comp_no=0;comp_no<number_of_components;comp_no++)
												{
													derivatives[comp_no] = left_derivatives[comp_no];
												}
												sf=0.0;
											}
										}
										else
										{
											if (0.0 < parameter_change)
											{
												left_sf = sf/parameter_change*left_parameter_change;
											}
										}
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"cc_add_duplicate_element.  "
									"Could not get values from element");
								return_code=0;
							}
						}
						if (return_code)
						{
							if (!(cc_set_node_field_values(destination_node,
								source->parameter_field,FE_NODAL_VALUE,&parameter)&&
								cc_set_node_field_values(destination_node,
									destination->value_field,FE_NODAL_VALUE,values)&&
								((!derivatives)||(
									cc_set_node_field_values(destination_node,
										destination->value_field,FE_NODAL_D_DS1,derivatives)&&
									set_FE_element_scale_factor(destination_element,node_no,sf)&&
									((!left_element)||set_FE_element_scale_factor(left_element,
										destination->value_nodes_per_element-1,left_sf))))))
							{
								display_message(ERROR_MESSAGE,
									"cc_add_duplicate_element.  Could not set values");
								return_code=0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"cc_add_duplicate_element.  Could not get node");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"cc_add_duplicate_element.  Could not add element");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cc_add_duplicate_element.  Not enough memory for values");
			return_code=0;
		}
		if (values)
		{
			DEALLOCATE(values);
		}
		if (derivatives)
		{
			DEALLOCATE(derivatives);
		}
		if (left_derivatives)
		{
			DEALLOCATE(left_derivatives);
		}
		if (temp_values)
		{
			DEALLOCATE(temp_values);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cc_add_duplicate_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cc_add_duplicate_element */

#if defined (DEBUG)
static int cc_list(struct Curve *curve)
/*******************************************************************************
LAST MODIFIED : 22 November 1999

DESCRIPTION :
Lists the contents of the curve.
==============================================================================*/
{
	FE_value parameter,sf,*values;
	int element_no,i,node_no,number_of_elements,return_code;
	struct FE_element *element;
	struct FE_node *node;
	
	ENTER(cc_list);
	if (curve)
	{
		return_code=1;
		if (ALLOCATE(values,FE_value,curve->number_of_components))
		{
			number_of_elements=Curve_get_number_of_elements(curve);
			for (element_no=1;(element_no<=number_of_elements)&&return_code;
				element_no++)
			{
				printf("element %d:\n",element_no);
				element=cc_get_element(curve,element_no);
				for (node_no=0;node_no<curve->value_nodes_per_element;node_no++)
				{
					get_FE_element_node(element,node_no,&node);
					cc_get_node_field_values(node,
						curve->parameter_field,FE_NODAL_VALUE,&parameter);
					cc_get_node_field_values(node,
						curve->value_field,FE_NODAL_VALUE,values);
					printf("  node %d (%d): parameter=%g",node_no,
						get_FE_node_cm_node_identifier(node),parameter);
					printf("    values =");
					for (i=0;i<curve->number_of_components;i++)
					{
						printf(" %g",values[i]);
					}
					printf("\n");
					if ((0<curve->value_derivatives_per_node)&&
						get_FE_element_scale_factor(element,node_no,&sf)&&
						cc_get_node_field_values(node,
							curve->value_field,FE_NODAL_D_DS1,values))
					{
						printf("    (sf=%g) derivatives =",sf);
						for (i=0;i<curve->number_of_components;i++)
						{
							printf(" %g",values[i]);
						}
						printf("\n");
					}
				}
			}
			DEALLOCATE(values);
		}
		else
		{
			display_message(ERROR_MESSAGE,"cc_list.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"cc_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cc_list */
#endif /* defined (DEBUG) */

static int cc_copy_convert_without_name(struct Curve *destination,
	enum FE_basis_type fe_basis_type,int number_of_components,
	struct Curve *source)
/*******************************************************************************
LAST MODIFIED : 15 April 2003

DESCRIPTION :
Makes a copy of <source> but with the given <fe_basis_type> and
<number_of_components>, cleans up dynamic fields in <destination> then points
them to those in the copy of source and safely cleans it up.
Does not modify the name identifier of <destination>.
If function fails, <destination> is left unchanged.
Used for various copy operations.
Works even when <destination> and <source> are the same.
==============================================================================*/
{
	struct Curve *temp_curve;
	int element_no,i,number_of_components_to_copy,number_of_elements,return_code;

	ENTER(cc_copy_convert_without_name);
	if (destination&&source)
	{
		number_of_components_to_copy=source->number_of_components;
		if (number_of_components < number_of_components_to_copy)
		{
			number_of_components_to_copy=number_of_components;
		}
		/* create copy of source with all its nodes, elements and attributes */
		if (temp_curve=CREATE(Curve)(source->name,fe_basis_type,
			number_of_components))
		{
			return_code=1;
			temp_curve->extend_mode=source->extend_mode;
			temp_curve->type=source->type;
			for (i=0;i<number_of_components_to_copy;i++)
			{
				temp_curve->max_value[i]=source->max_value[i];
				temp_curve->min_value[i]=source->min_value[i];
			}
			temp_curve->parameter_grid=source->parameter_grid;
			temp_curve->value_grid=source->value_grid;
			number_of_elements=Curve_get_number_of_elements(source);
			for (element_no=1;(element_no<=number_of_elements)&&return_code;
				element_no++)
			{
				return_code=cc_add_duplicate_element(temp_curve,source,element_no);
			}
			if (return_code)
			{
				/* transfer contents of temp_curve to destination; note no allocations
					 or accesses -- clean up manually */
				cc_clean_up(destination);
				destination->fe_basis_type=temp_curve->fe_basis_type;
				destination->number_of_components=temp_curve->number_of_components;
				destination->extend_mode=temp_curve->extend_mode;
				destination->type=temp_curve->type;
				destination->value_nodes_per_element=
					temp_curve->value_nodes_per_element;
				destination->value_derivatives_per_node=
					temp_curve->value_derivatives_per_node;
				destination->basis_manager=temp_curve->basis_manager;
				destination->element_shape_list=temp_curve->element_shape_list;
				destination->fe_region = temp_curve->fe_region;
				destination->parameter_field=temp_curve->parameter_field;
				destination->value_field=temp_curve->value_field;
				destination->template_node=temp_curve->template_node;
				destination->template_element=temp_curve->template_element;
				destination->max_value=temp_curve->max_value;
				destination->min_value=temp_curve->min_value;
				destination->parameter_grid=temp_curve->parameter_grid;
				destination->value_grid=temp_curve->value_grid;

				/* now safely remove what is left of temp_curve */
				DEALLOCATE(temp_curve->name);
				DEALLOCATE(temp_curve);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"cc_copy_convert_without_name.  Could not copy source elements");
				DESTROY(Curve)(&temp_curve);
				return_code=0;
			}
			/* make sure parameter_table is cleared in destination */
			if (destination->parameter_table)
			{
				DEALLOCATE(destination->parameter_table);
			}
			destination->parameter_table_size=0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cc_copy_convert_without_name.  Could not copy source curve");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cc_copy_convert_without_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cc_copy_convert_without_name */

/*
Global functions
----------------
*/

char **Curve_FE_basis_type_get_valid_strings(
	int *number_of_valid_strings)
/*******************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Returns and allocated array of pointers to all static strings for all
Fe_basis_types valid with Curves - obtained from function
FE_basis_type_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/
{
	char **valid_strings;

	ENTER(Curve_FE_basis_type_get_valid_strings);
	if (number_of_valid_strings)
	{
		*number_of_valid_strings=4;
		if (ALLOCATE(valid_strings,char *,*number_of_valid_strings))
		{
			valid_strings[0]=FE_basis_type_string(CUBIC_HERMITE);
			valid_strings[1]=FE_basis_type_string(CUBIC_LAGRANGE);
			valid_strings[2]=FE_basis_type_string(LINEAR_LAGRANGE);
			valid_strings[3]=FE_basis_type_string(QUADRATIC_LAGRANGE);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Curve_FE_basis_type_get_valid_strings.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_FE_basis_type_get_valid_strings.  Invalid argument");
		valid_strings=(char **)NULL;
	}
	LEAVE;

	return (valid_strings);
} /* Curve_FE_basis_type_get_valid_strings */

enum FE_basis_type Curve_FE_basis_type_from_string(
	char *fe_basis_type_string)
/*******************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Returns the FE_basis_type described by <fe_basis_type_string>, if valid for
use in Curves.
==============================================================================*/
{
	enum FE_basis_type fe_basis_type;

	ENTER(Curve_FE_basis_type_from_string);
	if (fe_basis_type_string)
	{
		if (fuzzy_string_compare_same_length(fe_basis_type_string,
			FE_basis_type_string(CUBIC_HERMITE)))
		{
			fe_basis_type=CUBIC_HERMITE;
		}
		else if (fuzzy_string_compare_same_length(fe_basis_type_string,
			FE_basis_type_string(CUBIC_LAGRANGE)))
		{
			fe_basis_type=CUBIC_LAGRANGE;
		}
		else if (fuzzy_string_compare_same_length(fe_basis_type_string,
			FE_basis_type_string(LINEAR_LAGRANGE)))
		{
			fe_basis_type=LINEAR_LAGRANGE;
		}
		else if (fuzzy_string_compare_same_length(fe_basis_type_string,
			FE_basis_type_string(QUADRATIC_LAGRANGE)))
		{
			fe_basis_type=QUADRATIC_LAGRANGE;
		}
		else
		{
			fe_basis_type=FE_BASIS_TYPE_INVALID;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_FE_basis_type_from_string.  Invalid argument");
		fe_basis_type=FE_BASIS_TYPE_INVALID;
	}
	LEAVE;

	return (fe_basis_type);
} /* Curve_FE_basis_type_from_string */

char *Curve_extend_mode_string(
	enum Curve_extend_mode extend_mode)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Returns a pointer to a static string describing the extend_mode, eg.
CONTROL_CURVE_EXTEND_CLAMP == "extend_clamp".
The calling function must not deallocate the returned string.
==============================================================================*/
{
	char *return_string;

	ENTER(Curve_extend_mode_string);
	switch (extend_mode)
	{
		case CONTROL_CURVE_EXTEND_CLAMP:
		{
			return_string="extend_clamp";
		} break;
		case CONTROL_CURVE_EXTEND_CYCLE:
		{
			return_string="extend_cycle";
		} break;
		case CONTROL_CURVE_EXTEND_SWING:
		{
			return_string="extend_swing";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Curve_extend_mode_string.  Invalid extend_mode");
			return_string=(char *)NULL;
		} break;
	}
	LEAVE;

	return (return_string);
} /* Curve_extend_mode_string */

char **Curve_extend_mode_get_valid_strings(
	int *number_of_valid_strings)
/*******************************************************************************
LAST MODIFIED : 10 November 1999

DESCRIPTION :
Returns and allocated array of pointers to all static strings for valid
Curve_extend_modes - obtained from function
Curve_extend_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/
{
	char **valid_strings;
	enum Curve_extend_mode extend_mode;
	int i;

	ENTER(Curve_extend_mode_get_valid_strings);
	if (number_of_valid_strings)
	{
		*number_of_valid_strings=0;
		extend_mode=CONTROL_CURVE_EXTEND_MODE_BEFORE_FIRST;
		extend_mode++;
		while (extend_mode<CONTROL_CURVE_EXTEND_MODE_AFTER_LAST)
		{
			(*number_of_valid_strings)++;
			extend_mode++;
		}
		if (ALLOCATE(valid_strings,char *,*number_of_valid_strings))
		{
			extend_mode=CONTROL_CURVE_EXTEND_MODE_BEFORE_FIRST;
			extend_mode++;
			i=0;
			while (extend_mode<CONTROL_CURVE_EXTEND_MODE_AFTER_LAST)
			{
				valid_strings[i]=Curve_extend_mode_string(extend_mode);
				i++;
				extend_mode++;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Curve_extend_mode_get_valid_strings.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_extend_mode_get_valid_strings.  Invalid argument");
		valid_strings=(char **)NULL;
	}
	LEAVE;

	return (valid_strings);
} /* Curve_extend_mode_get_valid_strings */

enum Curve_extend_mode Curve_extend_mode_from_string(
	char *extend_mode_string)
/*******************************************************************************
LAST MODIFIED : 10 November 1999

DESCRIPTION :
Returns the <Curve_extend_mode> described by <extend_mode_string>.
==============================================================================*/
{
	enum Curve_extend_mode extend_mode;

	ENTER(Curve_extend_mode_from_string);
	if (extend_mode_string)
	{
		extend_mode=CONTROL_CURVE_EXTEND_MODE_BEFORE_FIRST;
		extend_mode++;
		while ((extend_mode<CONTROL_CURVE_EXTEND_MODE_AFTER_LAST)&&
			(!fuzzy_string_compare_same_length(extend_mode_string,
				Curve_extend_mode_string(extend_mode))))
		{
			extend_mode++;
		}
		if (CONTROL_CURVE_EXTEND_MODE_AFTER_LAST==extend_mode)
		{
			extend_mode=CONTROL_CURVE_EXTEND_MODE_INVALID;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_extend_mode_from_string.  Invalid argument");
		extend_mode=CONTROL_CURVE_EXTEND_MODE_INVALID;
	}
	LEAVE;

	return (extend_mode);
} /* Curve_extend_mode_from_string */

struct Curve *CREATE(Curve)(char *name,
	enum FE_basis_type fe_basis_type,int number_of_components)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
Allocates memory and assigns fields for a struct Curve using the given
basis type - if supported - and number of components - if not to large).
The curve starts off with no elements and no nodes defined, so its returned
value will be zero in its initial state.
==============================================================================*/
{
	struct Coordinate_system coordinate_system;
	FE_element_field_component_modify modify;
	int number_of_scale_factor_sets,numbers_in_scale_factor_sets[1];
	int basis_type[2],i,j,return_code,shape_type[1];
	struct CM_element_information element_identifier;
	struct FE_basis *parameter_basis,*value_basis;
	struct FE_element_shape *element_shape;
	struct FE_element_field_component *parameter_component,**value_components;
	struct FE_node_field_creator *node_field_creator;
	struct Standard_node_to_element_map *standard_node_map;
	struct Curve *curve;
	void *scale_factor_set_identifiers[1];

	ENTER(CREATE(Curve));
	if (name)
	{
		if (curve=cc_create_blank(name))
		{
			return_code=1;
			if (cc_establish(curve,fe_basis_type,number_of_components))
			{
				/* set values allocated above */
				strcpy(curve->name,name);
				for (i=0;i<number_of_components;i++)
				{
					curve->min_value[i]=0.0;
					curve->max_value[i]=1.0;
				}
				curve->parameter_grid=0.1;
				curve->value_grid=0.1;
				coordinate_system.type=RECTANGULAR_CARTESIAN;
				/* create the parameter field = real, 1 component */
				if (curve->parameter_field =
					CREATE(FE_field)("parameter", curve->fe_region))
				{
					ACCESS(FE_field)(curve->parameter_field);
					if (!(set_FE_field_CM_field_type(curve->parameter_field,
						CM_GENERAL_FIELD) &&
						set_FE_field_value_type(curve->parameter_field,FE_VALUE_VALUE) &&
						set_FE_field_coordinate_system(curve->parameter_field,
							&coordinate_system) &&
						set_FE_field_number_of_components(curve->parameter_field,1) &&
						set_FE_field_type_general(curve->parameter_field) &&
						FE_region_merge_FE_field(curve->fe_region, curve->parameter_field)))
					{
						DEACCESS(FE_field)(&(curve->parameter_field));
						return_code = 0;
					}
				}
				else
				{
					return_code = 0;
				}
				/* create the value field = real, number_of_components */
				if (curve->value_field = CREATE(FE_field)("value", curve->fe_region))
				{
					ACCESS(FE_field)(curve->value_field);
					if (!((set_FE_field_CM_field_type(curve->value_field,
						CM_COORDINATE_FIELD) &&
						set_FE_field_value_type(curve->value_field,FE_VALUE_VALUE)&&
						set_FE_field_coordinate_system(curve->value_field,
							&coordinate_system)&&
						set_FE_field_number_of_components(curve->value_field,
							number_of_components)&&
						FE_region_merge_FE_field(curve->fe_region, curve->value_field))))
					{
						DEACCESS(FE_field)(&(curve->value_field));
						return_code = 0;
					}
				}
				else
				{
					return_code = 0;
				}
				if (return_code)
				{
					/* curve must access template_node */
					if (curve->template_node=ACCESS(FE_node)(CREATE(FE_node)(
						/*node_identifier*/0,curve->fe_region,(struct FE_node *)NULL)))
					{
						node_field_creator = CREATE(FE_node_field_creator)(
							/*number_of_components*/1);
						FE_node_field_creator_define_derivative(node_field_creator, 
							0, FE_NODAL_D_DS1);
						if (!define_FE_field_at_node(curve->template_node,
							curve->parameter_field,(struct FE_time_sequence *)NULL,
							node_field_creator))
						{
							return_code=0;
						}
						DESTROY(FE_node_field_creator)(&(node_field_creator));
						/* define value_field at template_node */
						node_field_creator = CREATE(FE_node_field_creator)(
							number_of_components);
						for (i=0;i<number_of_components;i++)
						{							
							for (j=0;i<curve->value_derivatives_per_node;i++)
							{
								FE_node_field_creator_define_derivative(
									node_field_creator, j, FE_NODAL_D_DS1);
							}
						}
						if (!define_FE_field_at_node(curve->template_node,
							curve->value_field,(struct FE_time_sequence *)NULL,
							node_field_creator))
						{
							return_code=0;
						}
						DESTROY(FE_node_field_creator)(&(node_field_creator));
						if (return_code)
						{
							element_identifier.type=CM_ELEMENT;
							element_identifier.number=0;
							shape_type[0]=LINE_SHAPE;
							modify=(FE_element_field_component_modify)NULL;
							element_shape=ACCESS(FE_element_shape)(
								CREATE(FE_element_shape)(/*dimension*/1,shape_type,
								curve->fe_region));
							/* create bases for parameter and value fields */
							basis_type[0]=1;
							basis_type[1]=(int)(LINEAR_LAGRANGE);
							parameter_basis=ACCESS(FE_basis)(CREATE(FE_basis)(basis_type));
							ADD_OBJECT_TO_MANAGER(FE_basis)(parameter_basis,
								curve->basis_manager);
							if (LINEAR_LAGRANGE == curve->fe_basis_type)
							{
								value_basis=ACCESS(FE_basis)(parameter_basis);
							}
							else
							{
								basis_type[0]=1;
								basis_type[1]=(int)(curve->fe_basis_type);
								value_basis=ACCESS(FE_basis)(CREATE(FE_basis)(basis_type));
								ADD_OBJECT_TO_MANAGER(FE_basis)(value_basis,
									curve->basis_manager);
							}
							/* only use scale factors with cubic Hermite derivatives */
							if (CUBIC_HERMITE==curve->fe_basis_type)
							{
								number_of_scale_factor_sets=1;
								scale_factor_set_identifiers[0]=(void *)(value_basis);
								/* one scale factor per node for 1 derivative per node */
								numbers_in_scale_factor_sets[0]=curve->value_nodes_per_element*
									curve->value_derivatives_per_node;
							}
							else
							{
								number_of_scale_factor_sets=0;
							}
							/* curve must access template_element */
							if ((curve->template_element = ACCESS(FE_element)(
								CREATE(FE_element)(&element_identifier, element_shape,
							   curve->fe_region, (struct FE_element *)NULL))) &&
								set_FE_element_number_of_nodes(curve->template_element,
									curve->value_nodes_per_element) &&
								set_FE_element_number_of_scale_factor_sets(
									curve->template_element, number_of_scale_factor_sets,
									scale_factor_set_identifiers, numbers_in_scale_factor_sets))
							{
								/* define parameter_field in template_element */
								if (parameter_component=CREATE(FE_element_field_component)(
									STANDARD_NODE_TO_ELEMENT_MAP,/*parameter_nodes_per_element*/2,
									parameter_basis,modify))
								{
									if (standard_node_map=CREATE(Standard_node_to_element_map)(
										/*node_number*/0,/*number_of_component_values*/1))
									{
										if (!(Standard_node_to_element_map_set_nodal_value_index(
											standard_node_map, /*nodal_value_number*/0, /*nodal_value_index*/0) &&
										/* -1 = use default scale factor of 1 */
										Standard_node_to_element_map_set_scale_factor_index(
											standard_node_map, /*nodal_value_number*/0, /*scale_factor_index*/-1) &&
										FE_element_field_component_set_standard_node_map(
											parameter_component, /*local_node_number*/0, standard_node_map)))
										{
											DESTROY(Standard_node_to_element_map)(&standard_node_map);
											return_code = 0;
										}
									}
									else
									{
										return_code=0;
									}
									if (standard_node_map=CREATE(Standard_node_to_element_map)(
										/*node_number*/curve->value_nodes_per_element-1,
										/*number_of_component_values*/1))
									{
										if (!(Standard_node_to_element_map_set_nodal_value_index(
											standard_node_map, /*nodal_value_number*/0, /*nodal_value_index*/0) &&
										/* -1 = use default scale factor of 1 */
										Standard_node_to_element_map_set_scale_factor_index(
											standard_node_map, /*nodal_value_number*/0, /*scale_factor_index*/-1) &&
										FE_element_field_component_set_standard_node_map(
											parameter_component, /*local_node_number*/1, standard_node_map)))
										{
											DESTROY(Standard_node_to_element_map)(&standard_node_map);
											return_code = 0;
										}
									}
									else
									{
										return_code=0;
									}
									if (return_code)
									{
										if (!define_FE_field_at_element(curve->template_element,
											curve->parameter_field,&parameter_component))
										{
											return_code=0;
										}
									}
									DESTROY(FE_element_field_component)(&parameter_component);
								}
								else
								{
									return_code=0;
								}
								/* define value_field in template_element */
								if (ALLOCATE(value_components,
									struct FE_element_field_component *,number_of_components))
								{
									for (i=0;i<number_of_components;i++)
									{
										value_components[i]=
											(struct FE_element_field_component *)NULL;
									}
									for (i=0;(i<number_of_components)&&return_code;i++)
									{
										if (value_components[i]=CREATE(FE_element_field_component)(
											STANDARD_NODE_TO_ELEMENT_MAP,
											curve->value_nodes_per_element,value_basis,modify))
										{
											/* loop over local nodes */
											for (j=0;(j<curve->value_nodes_per_element)&&
												return_code;j++)
											{
												if (standard_node_map=
													CREATE(Standard_node_to_element_map)(
														/*node_number*/j,/*number_of_component_values*/
														(1+curve->value_derivatives_per_node)))
												{
													if (Standard_node_to_element_map_set_nodal_value_index(
														standard_node_map, /*nodal_value_number*/0, /*nodal_value_index*/0) &&
														/* -1 = use default scale factor of 1 */
														Standard_node_to_element_map_set_scale_factor_index(
													   standard_node_map, /*nodal_value_number*/0, /*scale_factor_index*/-1))
													{
														if (CUBIC_HERMITE==curve->fe_basis_type)
														{
															Standard_node_to_element_map_set_nodal_value_index(
																standard_node_map, /*nodal_value_number*/1, /*nodal_value_index*/1);
															/* multiply scale factor for derivative */
															Standard_node_to_element_map_set_scale_factor_index(
																standard_node_map, /*nodal_value_number*/1, /*scale_factor_index*/j);
															/* default scale factor of zero for derivative */
															set_FE_element_scale_factor(curve->template_element,j,0.0);
														}
														if (!(FE_element_field_component_set_standard_node_map(
															value_components[i], /*local_node_number*/j, standard_node_map)))
														{
															DESTROY(Standard_node_to_element_map)(&standard_node_map);
															return_code = 0;
														}
													}
													else
													{
														DESTROY(Standard_node_to_element_map)(&standard_node_map);
														return_code = 0;
													}
												}
												else
												{
													return_code=0;
												}
											}
										}
										else
										{
											return_code=0;
										}
									}
									if (return_code)
									{
										if (!define_FE_field_at_element(curve->template_element,
											curve->value_field,value_components))
										{
											return_code=0;
										}
									}
									for (i=0;i<number_of_components;i++)
									{
										DESTROY(FE_element_field_component)(&(value_components[i]));
									}
									DEALLOCATE(value_components);
								}
								else
								{
									return_code=0;
								}
								if (!return_code)
								{
									display_message(ERROR_MESSAGE,"CREATE(Curve).  "
										"Could not define fields in template_element");
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"CREATE(Curve).  Could not create template_element");
								return_code=0;
							}
							DEACCESS(FE_basis)(&parameter_basis);
							DEACCESS(FE_basis)(&value_basis);
							DEACCESS(FE_element_shape)(&element_shape);
						}
						else
						{
							display_message(ERROR_MESSAGE,"CREATE(Curve).  "
								"Could not define fields at template_node");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(Curve).  Could not create template_node");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Curve).  Could not create fields");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"CREATE(Curve).  "
					"Could not set basis type or number of components");
				return_code=0;
			}
			if (!return_code)
			{
				DESTROY(Curve)(&curve);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Curve).  Could not create curve");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Curve).  Invalid argument(s)");
		curve=(struct Curve *)NULL;
	}
	LEAVE;

	return (curve);
} /* CREATE(Curve) */

int DESTROY(Curve)(struct Curve **curve_ptr)
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Frees the memory for the fields of <**curve_ptr>, frees the memory for
<**curve_ptr> and sets <*curve_ptr> to NULL.
==============================================================================*/
{
	struct Curve *curve;
	int return_code;

	ENTER(DESTROY(Curve));
	if (curve_ptr)
	{
		if (curve= *curve_ptr)
		{
			DEALLOCATE(curve->name);

			cc_clean_up(curve);
			if (0!=curve->access_count)
			{
				display_message(ERROR_MESSAGE,
					"DESTROY(Curve).  Non-zero access_count");
			}
			DEALLOCATE(*curve_ptr);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Curve).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Curve) */

DECLARE_OBJECT_FUNCTIONS(Curve)
DECLARE_INDEXED_LIST_FUNCTIONS(Curve)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Curve,name,char *, \
	strcmp)
DECLARE_INDEXED_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Curve,name)

DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Curve)

PROTOTYPE_COPY_OBJECT_FUNCTION(Curve)
/*******************************************************************************
LAST MODIFIED : 1 September 1997

DESCRIPTION :
syntax: COPY(Curve)(destination,source)
Just calls manager copy function which does the same thing.
???RC I think copy with/without identifier should be object functions.
==============================================================================*/
{
	int return_code;

	ENTER(COPY(Curve));
	return_code=MANAGER_COPY_WITH_IDENTIFIER(Curve,name)(
		destination,source);
	LEAVE;

	return (return_code);
} /* COPY(Curve) */

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Curve,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Curve,name));
	if (source&&destination)
	{
		return_code=((MANAGER_COPY_WITHOUT_IDENTIFIER(Curve,name)
			(destination,source))&&
			(MANAGER_COPY_IDENTIFIER(Curve,name)(destination,source->name)));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITH_IDENTIFIER(Curve,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Curve,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Curve,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Curve,name));
	return_code=0;		
	if (destination&&source)
	{
		/* check not changing number of components while curve is in use */
		if ((source->number_of_components!=destination->number_of_components)&&
			Curve_is_in_use(destination))
		{
			display_message(ERROR_MESSAGE,
				"MANAGER_COPY_WITHOUT_IDENTIFIER(Curve,name).  "
				"Cannot change number of components while curve is in use");
		}
		else
		{
			if (cc_copy_convert_without_name(destination,
				source->fe_basis_type,source->number_of_components,source))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITHOUT_IDENTIFIER(Curve,name).  "
					"Could not copy source curve");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Curve,name).  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Curve,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Curve,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Curve,name));
	if (name&&destination)
	{
		if (ALLOCATE(destination_name,char,strlen(name)+1))
		{
			strcpy(destination_name,name);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"MANAGER_COPY_IDENTIFIER(Curve,name).  Insufficient memory");
			return_code=0;
		}
		if (return_code)
		{
			/* copy name */
			DEALLOCATE(destination->name);
			destination->name=destination_name;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Curve,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Curve,name) */

DECLARE_MANAGER_FUNCTIONS(Curve)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Curve)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Curve,name,char *)

int Curve_unitize_vector(FE_value *vector,int number_of_components,
	FE_value *norm)
/*******************************************************************************
LAST MODIFIED : 14 October 1997

DESCRIPTION :
Turns <vector> into a unit vector, returning its former magnitude as <norm>.
If <vector> is a zero it is made into a unit vector in the direction [1,1,1...]
while norm is returned as zero.
==============================================================================*/
{
	int return_code,i;

	ENTER(Curve_unitize_vector);
	if (vector&&number_of_components&&norm)
	{
		/* get new norm or vector */
		*norm=0.0;
		for (i=0;i<number_of_components;i++)
		{
			*norm += vector[i]*vector[i];
		}
		if (1.0E-6 > (*norm=sqrt(*norm)))
		{
			*norm=0.0;
		}
		/* convert to unit vector */
		for (i=0;i<number_of_components;i++)
		{
			if (0<*norm)
			{
				vector[i] /= *norm;
			}
			else
			{
				vector[i]=1/sqrt(number_of_components);
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_unitize_vector.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_unitize_vector */

int Curve_get_node_scale_factor_dparameter(struct Curve *curve,
	int element_no,int local_node_no,FE_value *sf_dparam)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Returns the scale factor at the node divided by the parameter change over the
element. If the parameter_change is zero, try to calculate this quantity from
the neighbouring element, if any. Otherwise zero is returned.
This value can be used to maintain continuity across element boundaries.
==============================================================================*/
{
	int return_code,number_of_nodes;
	FE_value sf,dparam;

	ENTER(Curve_get_node_scale_factor_dparameter);
	if (curve&&sf_dparam&&
		Curve_get_scale_factor(curve,element_no,local_node_no,&sf)&&
		Curve_get_element_parameter_change(curve,element_no,&dparam))
	{
		return_code=1;
		number_of_nodes=Curve_get_nodes_per_element(curve);
		if (0<dparam)
		{
			*sf_dparam=sf/dparam;
		}
		else
		{
			if ((0==local_node_no)&&(1<element_no))
			{
				if (Curve_get_scale_factor(curve,element_no-1,
					number_of_nodes-1,&sf)&&Curve_get_element_parameter_change(
					curve,element_no-1,&dparam)&&(0<dparam))
				{
					*sf_dparam=sf/dparam;
				}
				else
				{
					*sf_dparam=0.0;
				}
			}
			else
			{
				if (((number_of_nodes-1)==local_node_no)&&
					(Curve_get_number_of_elements(curve)>element_no))
				{
					if (Curve_get_scale_factor(curve,element_no+1,0,&sf)&&
						Curve_get_element_parameter_change(curve,element_no+1,
						&dparam)&&(0<dparam))
					{
						*sf_dparam=sf/dparam;
					}
					else
					{
						*sf_dparam=0.0;
					}
				}
				else
				{
					*sf_dparam=0.0;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_node_scale_factor_dparameter.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_get_node_scale_factor_dparameter */

int Curve_enforce_continuity(struct Curve *curve,
	int element_no,int local_node_no,int equal_priority,
	enum Curve_continuity_mode continuity_mode)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Used to adjust derivatives and scaling factors on boundary nodes of cubic
Hermite elements to enforce a specified degree of continuity.
If the <equal_priority> flag is set, scaling factors on both sides of the
boundary node will be adjusted to enforce continuity. Otherwise, the specified
<element_no,node_no> will be unchanged and its parter adjusted.
==============================================================================*/
{
	FE_value dparam1,dparam2,*parameters,sf1,sf2,slope,sf;
	int return_code,first,last;
	struct FE_element *element1,*element2;

	ENTER(Curve_enforce_continuity);
	if (curve)
	{
		if (curve->parameter_table||cc_build_parameter_table(curve))
		{
			return_code=1;
			/* check if needs enforcing at all: */
			if ((0<curve->value_derivatives_per_node)&&
				(((0==local_node_no)&&(1<element_no))||
					(((curve->value_nodes_per_element-1)==local_node_no)&&
						(element_no<(curve->parameter_table_size-1)))))
			{
				parameters=curve->parameter_table;
				if (0==local_node_no)
				{
					first=element_no-1;
					last=element_no;
				}
				else
				{
					first=element_no;
					last=element_no+1;
				}
				switch (continuity_mode)
				{
					case CONTROL_CURVE_CONTINUITY_SLOPE:
					{
						/* ensure sf/dparam is constant across node */
						dparam1=parameters[first]-parameters[first-1];
						dparam2=parameters[last]-parameters[last-1];
						if (return_code=((element1=cc_get_element(curve,first))&&
							(element2=cc_get_element(curve,last))&&
							get_FE_element_scale_factor(element1,
								curve->value_nodes_per_element-1,&sf1)&&
							get_FE_element_scale_factor(element2,0,&sf2)))
						{
							/* get slope to be set on each side of node */
							slope=0.0;
							if (equal_priority)
							{
								if (0<dparam1)
								{
									if (0<dparam2)
									{
										slope=0.5*(sf1/dparam1+sf2/dparam2);
									}
									else
									{
										slope=sf1/dparam1;
									}
								}
								else
								{
									if (0<dparam2)
									{
										slope=sf2/dparam2;
									}
								}
							}
							else
							{
								if (0==local_node_no)
								{
									if (0<dparam2)
									{
										slope=sf2/dparam2;
									}
									else
									{
										if (0<dparam1)
										{
											slope=sf1/dparam1;
										}
									}
								}
								else
								{
									if (0<dparam1)
									{
										slope=sf1/dparam1;
									}
									else
									{
										if (0<dparam2)
										{
											slope=sf2/dparam2;
										}
									}
								}
							}
							return_code=(set_FE_element_scale_factor(element1,
								curve->value_nodes_per_element-1,slope*dparam1)&&
								set_FE_element_scale_factor(element2,0,slope*dparam2));
						}
					} break;
					case CONTROL_CURVE_CONTINUITY_C1:
					{
						/* ensure sf is constant across node */
						if (return_code=((element1=cc_get_element(curve,first))&&
							(element2=cc_get_element(curve,last))&&
							get_FE_element_scale_factor(element1,
								curve->value_nodes_per_element-1,&sf1)&&
							get_FE_element_scale_factor(element2,0,&sf2)))
						{
							if (equal_priority)
							{
								sf=0.5*(sf1+sf2);
							}
							else
							{
								if (0==local_node_no)
								{
									sf=sf2;
								}
								else
								{
									sf=sf1;
								}
							}
							return_code=(set_FE_element_scale_factor(element1,
								curve->value_nodes_per_element-1,sf)&&
								set_FE_element_scale_factor(element2,0,sf));
						}
					} break;
					case CONTROL_CURVE_CONTINUITY_G1:
					{
						/* Hermite elements are g1 continuous by default */
						/* (unless scaling factors are zero) */
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Curve_enforce_continuity.  Unknown continuity mode");
						return_code=0;
					} break;
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,
						"Curve_enforce_continuity.  Error encountered");
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Curve_enforce_continuity.  Could not build parameter table");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_enforce_continuity.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_enforce_continuity */

int Curve_has_1_component(struct Curve *curve,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 30 November 1999

DESCRIPTION :
Returns true if the curve has 1 component - used with choosers.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_has_1_component);
	USE_PARAMETER(dummy_void);
	if (curve)
	{
		return_code=(1==curve->number_of_components);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_has_1_component.  Missing curve");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_has_1_component */

int Curve_is_in_use(struct Curve *curve)
/*******************************************************************************
LAST MODIFIED : 4 November 1999

DESCRIPTION :
Returns true if the curve is accessed more than once; ie. it is in use
somewhere else in the program - apart from being accessed by its manager.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_is_in_use);
	if (curve)
	{
		return_code=(1 < curve->access_count);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Curve_is_in_use.  Missing curve");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_is_in_use */

int Curve_get_number_of_elements(struct Curve *curve)
/*******************************************************************************
LAST MODIFIED : 12 November 1999

DESCRIPTION :
Returns the number of elements in the curve.
==============================================================================*/
{
	int number_of_elements;

	ENTER(Curve_get_number_of_elements);
	if (curve)
	{
		number_of_elements=FE_region_get_number_of_FE_elements(curve->fe_region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_number_of_elements.  Invalid argument(s)");
		number_of_elements=0;
	}
	LEAVE;

	return (number_of_elements);
} /* Curve_get_number_of_elements */

int Curve_add_element(struct Curve *curve,int element_no)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
Adds an element to the list of elements in the curve at position element_no.
Valid element_nos are from 1 to the number of elements in the curve plus 1.
New elements occupy a point (ie. zero size and the same parameter at the start and
end of the element). If the element is added at the end of the list, the last
node will be copied to achieve this. The nodes in elements added before the
end of the list are copies of the first node in the element formerly at that
space, which will have its element_no incremented, as will all subsequent
elements. Neighbouring elements are made to share the node on their common
boundary.
If there are no existing elements, an element is created spanning from 0,0 to
0,0 with zero derivatives.
In all cases subsequent calls to change coordinate and derivative values are
expected to make the element have a finite size.
==============================================================================*/
{
	int i,node_no,node_no_increment,number_of_elements,number_of_nodes,
		return_code;
	struct CM_element_information cm;
	struct FE_node *node,*node_to_copy;
	struct FE_element *element,*existing_element,*last_element;

	ENTER(Curve_add_element);
	if (curve&&(0<=(number_of_elements=
		FE_region_get_number_of_FE_elements(curve->fe_region)))&&
		(0<element_no)&&((element_no <= number_of_elements+1)))
	{
		cm.type = CM_ELEMENT;
		cm.number=element_no;
		/* create the element from the template_element */
		if (element=CREATE(FE_element)(&cm, (struct FE_element_shape *)NULL,
			(struct FE_region *)NULL, curve->template_element))
		{
			return_code=1;
			if (0==number_of_elements)
			{
				/* first element: all new nodes from curve->template_node */
				for (i=0;(i<curve->value_nodes_per_element)&&return_code;i++)
				{
					if (node = CREATE(FE_node)(i+1, (struct FE_region *)NULL,
						curve->template_node))
					{
						if (!set_FE_element_node(element,i,node))
						{
							DESTROY(FE_node)(&node);
							return_code=0;
						}
					}
					else
					{
						return_code=0;
					}
				}
			}
			else if (element_no>number_of_elements)
			{
				/* last element: share last node of last element, make new nodes as
					 copy of it */
				cm.number=number_of_elements;
				if ((last_element=FE_region_get_FE_element_from_identifier(
					curve->fe_region, &cm))&&
					get_FE_element_node(last_element,curve->value_nodes_per_element-1,
						&node_to_copy))
				{
					if (!set_FE_element_node(element,0,node_to_copy))
					{
						return_code=0;
					}
					node_no=get_FE_node_identifier(node_to_copy);
					for (i=1;(i<curve->value_nodes_per_element)&&return_code;i++)
					{
						if (node=CREATE(FE_node)(node_no+i, (struct FE_region *)NULL, node_to_copy))
						{
							if (!set_FE_element_node(element,i,node))
							{
								DESTROY(FE_node)(&node);
								return_code=0;
							}
						}
						else
						{
							return_code=0;
						}
					}
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				/* interior elements: take first node of element_at_pos for new element
					 and add copies of it for remainder of nodes. Set first node of
					 element_at_pos to last new node for continuity. Renumber subsequent
					 nodes and elements to keep sequential */
				if ((existing_element=FE_region_get_FE_element_from_identifier
					(curve->fe_region, &cm))&&
					get_FE_element_node(existing_element,0,&node_to_copy)&&
					(node_no=get_FE_node_identifier(node_to_copy))&&
					(number_of_nodes=FE_region_get_number_of_FE_nodes(curve->fe_region)))
				{
					/* increment numbers of elements to follow */
					for (i=number_of_elements;(i>=element_no)&&return_code;i--)
					{
						cm.number=i;
						if (existing_element=FE_region_get_FE_element_from_identifier
							(curve->fe_region, &cm))
						{
							cm.number++;
							if (!FE_region_change_FE_element_identifier(
								curve->fe_region, existing_element, &cm))
							{
								return_code=0;
							}
						}
					}
					/* increment numbers of nodes to follow - not node_no though! */
					node_no_increment=curve->value_nodes_per_element-1;
					for (i=number_of_nodes;(i>node_no)&&return_code;i--)
					{
						if (!FE_region_change_FE_node_identifier(curve->fe_region,
							FE_region_get_FE_node_from_identifier(
							curve->fe_region, i), i+node_no_increment))
						{
							return_code=0;
						}
					}
					/* take first node of existing_element */
					if (!set_FE_element_node(element,0,node_to_copy))
					{
						return_code=0;
					}
					/* create copies of this node for rest of new element */
					for (i=1;(i<curve->value_nodes_per_element)&&return_code;i++)
					{
						if (node=CREATE(FE_node)(node_no+i, (struct FE_region *)NULL,
							node_to_copy))
						{
							if (!set_FE_element_node(element,i,node))
							{
								DESTROY(FE_node)(&node);
								return_code=0;
							}
						}
						else
						{
							return_code=0;
						}
					}
					if (return_code)
					{
						/* for continuity, existing_element must get last new node */
						if (!set_FE_element_node(existing_element,0,node))
						{
							return_code=0;
						}
					}
				}
			}
			/* make sure new nodes are in manager */
			for (i=0;(i<curve->value_nodes_per_element)&&return_code;i++)
			{
				if (get_FE_element_node(element,i,&node))
				{
					if (!FE_region_get_FE_node_from_identifier(curve->fe_region,
						get_FE_node_identifier(node)))
					{
						if (!FE_region_merge_FE_node(curve->fe_region, node))
						{
							return_code=0;
						}
					}
				}
				else
				{
					return_code=0;
				}
			}
			if (return_code)
			{
				if (!FE_region_merge_FE_element(curve->fe_region, element))
				{
					return_code = 0;
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Curve_add_element.  Could not put element in curve");
				DESTROY(FE_element)(&element);
			}
			cc_clear_parameter_table(curve);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Curve_add_element.  Could not create new element");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_add_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_add_element */

int Curve_delete_element(struct Curve *curve,int element_no,
	int local_node_no)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Deletes the element with with the given number, cleaning up links with
neighbouring elements.
If the local_node_no is in either the first or the last element and not shared
by a neighbouring element, that element is deleted outright with the parameter range
reducing accordingly.
Interior local_nodes are treated as if local_node_no=0. Neighbouring elements
which share the specified local_node_no have that local node changed to the one
at the other end of the element being deleted.
==============================================================================*/
{
	int first_node_to_delete,i,last_node_to_delete,node_no_decrement,
		number_of_elements,number_of_nodes,return_code;
	struct CM_element_information cm;
	struct FE_element *element_to_delete,*existing_element,*left_element,
		*right_element;
	struct FE_node *left_node,*right_node;

	ENTER(Curve_delete_element);
	if (curve&&
		(number_of_elements=FE_region_get_number_of_FE_elements(curve->fe_region))&&
		(number_of_nodes=FE_region_get_number_of_FE_nodes(curve->fe_region))&&
		(0<element_no)&&(element_no <= number_of_elements))
	{
		return_code=1;
		cm.type = CM_ELEMENT;
		cm.number=element_no;
		if ((element_to_delete=FE_region_get_FE_element_from_identifier(
			curve->fe_region, &cm))&&
			get_FE_element_node(element_to_delete,0,&left_node)&&
			get_FE_element_node(element_to_delete,curve->value_nodes_per_element-1,
				&right_node))
		{
			cm.number=element_no-1;
			left_element=FE_region_get_FE_element_from_identifier(
				curve->fe_region, &cm);
			cm.number=element_no+1;
			right_element=FE_region_get_FE_element_from_identifier(
				curve->fe_region, &cm);
			first_node_to_delete=get_FE_node_identifier(left_node);
			last_node_to_delete=get_FE_node_identifier(right_node);
			if (1==element_no)
			{
				/* first element */
				if (1<number_of_elements)
				{
					if (right_element)
					{
						if (0<local_node_no)
						{
							first_node_to_delete++;
							if (!set_FE_element_node(right_element,0,left_node))
							{
								return_code=0;
							}
						}
						else
						{
							last_node_to_delete--;
						}
					}
					else
					{
						return_code=0;
					}
				}
			}
			else if (element_no<number_of_elements)
			{
				/* middle element */
				if (left_element&&right_element)
				{
					if (local_node_no<(curve->value_nodes_per_element/2))
					{
						last_node_to_delete--;
						if (!set_FE_element_node(left_element,
							curve->value_nodes_per_element-1,right_node))
						{
							return_code=0;
						}
					}
					else
					{
						first_node_to_delete++;
						if (!set_FE_element_node(right_element,0,left_node))
						{
							return_code=0;
						}
					}
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				/* last element */
				if (left_element)
				{
					if (local_node_no<curve->value_nodes_per_element-1)
					{
						last_node_to_delete--;
						if (!set_FE_element_node(left_element,
							curve->value_nodes_per_element-1,right_node))
						{
							return_code=0;
						}
					}
					else
					{
						first_node_to_delete++;
					}
				}
				else
				{
					return_code=0;
				}
			}
			if (return_code)
			{
				/* remove the element and nodes */
				if (FE_region_remove_FE_element(curve->fe_region, element_to_delete))
				{
					for (i=first_node_to_delete;(i<=last_node_to_delete)&&return_code;i++)
					{
						if (!FE_region_remove_FE_node(curve->fe_region,
							FE_region_get_FE_node_from_identifier(curve->fe_region, i)))
						{
							return_code=0;
						}
					}
				}
				else
				{
					return_code=0;
				}
			}
			/* decrement numbers of elements to follow */
			for (i=element_no+1;(i<=number_of_elements)&&return_code;i++)
			{
				cm.number=i;
				if (existing_element=FE_region_get_FE_element_from_identifier(
					curve->fe_region, &cm))
				{
					cm.number--;
					if (!FE_region_change_FE_element_identifier(curve->fe_region,
						existing_element,&cm))
					{
						return_code=0;
					}
				}
			}
			/* decrement numbers of nodes to follow */
			node_no_decrement=(last_node_to_delete-first_node_to_delete)+1;
			for (i=last_node_to_delete+1;(i<=number_of_nodes)&&return_code;i++)
			{
				if (!FE_region_change_FE_node_identifier(curve->fe_region,
					FE_region_get_FE_node_from_identifier(curve->fe_region,
				   i),i-node_no_decrement))
				{
					return_code=0;
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Curve_delete_element.  Could not delete element from curve");
				return_code=0;
			}
			cc_clear_parameter_table(curve);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Curve_delete_element.  Missing element");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_delete_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_delete_element */

int Curve_subdivide_element(struct Curve *curve,
	int element_no,FE_value split_xi)
/*******************************************************************************
LAST MODIFIED : 22 November 1999

DESCRIPTION :
Subdivides element_no at split_xi, making the 2 new elements follow the curve
of the original.
==============================================================================*/
{
	int return_code,node_no,number_of_components,nodes_per_element;
	FE_value *values,*derivatives,parameter1,parameter2,xi,sf;
	struct FE_node *destination,*source;
	struct FE_element *element1,*element2,*original_element;

	ENTER(Curve_subdivide_element);
	if (curve&&(original_element=cc_get_element(curve,element_no))&&
		(0 <= split_xi)&&(1>=split_xi))
	{
		nodes_per_element=curve->value_nodes_per_element;
		if (Curve_get_parameter(curve,element_no,0,&parameter1)&&
			Curve_get_parameter(curve,element_no,nodes_per_element-1,
				&parameter2))
		{
			/* add two new elements to the curve to replace the existing one */
			if (Curve_add_element(curve,element_no)&&
				Curve_add_element(curve,element_no))
			{
				number_of_components=curve->number_of_components;
				/* copy values from original_element to the 2 elements replacing it */
				Curve_set_parameter(curve,element_no+1,0,
					parameter1+split_xi*(parameter2-parameter1));
				Curve_set_parameter(curve,element_no+2,0,parameter2);
				if ((element1=cc_get_element(curve,element_no))&&
					(element2=cc_get_element(curve,element_no+1))&&
					ALLOCATE(values,FE_value,number_of_components)&&
					ALLOCATE(derivatives,FE_value,number_of_components))
				{
					return_code=1;
					if (0<curve->value_derivatives_per_node)
					{
						/* cubic Hermite basis: ensure continuous slope */
						if (get_FE_element_scale_factor(original_element,0,&sf)&&
							set_FE_element_scale_factor(element1,0,split_xi*sf)&&
							get_FE_element_scale_factor(original_element,nodes_per_element-1,
								&sf)&&
							set_FE_element_scale_factor(element2,nodes_per_element-1,
								(1.0-split_xi)*sf)&&
							cc_calculate_element_field_values(original_element,split_xi,
								curve->value_field,values,derivatives)&&
							(get_FE_element_node(element2,0,&destination))&&
							cc_set_node_field_values(destination,curve->value_field,
								FE_NODAL_VALUE,values))
						{
							/* get magnitude of derivatives at split = sf */
							Curve_unitize_vector(derivatives,number_of_components,
								&sf);
							return_code=(cc_set_node_field_values(destination,
								curve->value_field,FE_NODAL_D_DS1,derivatives)&&
								set_FE_element_scale_factor(element1,nodes_per_element-1,
									split_xi*sf)&&
								set_FE_element_scale_factor(element2,0,(1.0-split_xi)*sf));
						}
						else
						{
							return_code=0;
						}
					}
					else
					{
						/* copy values of nodes of Lagrangian elements */
						for (node_no=1;(node_no<nodes_per_element)&&return_code;node_no++)
						{
							get_FE_element_node(element1,node_no,&destination);
							xi=split_xi*(FE_value)node_no/
								(curve->value_nodes_per_element-1.0);
							return_code=(cc_calculate_element_field_values(original_element,
								xi,curve->value_field,values,(FE_value *)NULL)&&
								cc_set_node_field_values(destination,curve->value_field,
									FE_NODAL_VALUE,values));
						}
						for (node_no=1;(node_no<nodes_per_element-1)&&return_code;node_no++)
						{
							get_FE_element_node(element2,node_no,&destination);
							xi=split_xi+(1.0-split_xi)*
								(FE_value)node_no/(curve->value_nodes_per_element-1.0);
							return_code=(cc_calculate_element_field_values(original_element,
								xi,curve->value_field,values,(FE_value *)NULL)&&
								cc_set_node_field_values(destination,curve->value_field,
									FE_NODAL_VALUE,values));
						}
					}
					if (return_code)
					{
						/* copy right node of original_element to right node of element2 */
						if (get_FE_element_node(original_element,nodes_per_element-1,
							&source)&&
							get_FE_element_node(element2,nodes_per_element-1,&destination)&&
							cc_get_node_field_values(source,curve->value_field,
								FE_NODAL_VALUE,values)&&
							cc_set_node_field_values(destination,curve->value_field,
								FE_NODAL_VALUE,values))
						{
							if (0<curve->value_derivatives_per_node)
							{
								return_code=(cc_get_node_field_values(source,
									curve->value_field,FE_NODAL_D_DS1,derivatives)&&
									cc_set_node_field_values(destination,
										curve->value_field,FE_NODAL_D_DS1,derivatives));
							}
						}
						else
						{
							return_code=0;
						}
					}
					DEALLOCATE(values);
					DEALLOCATE(derivatives);
				}
				else
				{
					if (values)
					{
						DEALLOCATE(values);
					}
					return_code=0;
				}
				if (return_code)
				{
					/* remove the original element */
					Curve_delete_element(curve,element_no+2,
						curve->value_nodes_per_element-1);
				}
				else
				{
					/* remove the new elements */
					Curve_delete_element(curve,element_no,
						curve->value_nodes_per_element-1);
					Curve_delete_element(curve,element_no,
						curve->value_nodes_per_element-1);
				}
			}
			else
			{
				return_code=0;
			}
			cc_clear_parameter_table(curve);
		}
		else
		{
			return_code=0;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Curve_subdivide_element.  Error subdividing element");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_subdivide_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_subdivide_element */

int Curve_get_number_of_components(struct Curve *curve)
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Returns the number of components in the curve field.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_get_number_of_components);
	if (curve)
	{
		return_code=curve->number_of_components;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_number_of_components.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_get_number_of_components */

int Curve_set_number_of_components(struct Curve *curve,
	int number_of_components)
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Changes the number of components in the curve. Component information is
copied to the new curve that is created to replace the current one. If the
new number of components is less than the current number, information is lost.
If it is greater, new components values are set to default values.
Returns true if the conversion is successful.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_set_number_of_components);
	if (curve)
	{
		if (number_of_components != curve->number_of_components)
		{
			if (cc_copy_convert_without_name(curve,curve->fe_basis_type,
				number_of_components,curve))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Curve_set_number_of_components.  Could not convert curve");
				return_code=0;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_set_number_of_components.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_set_number_of_components */

int Curve_get_nodes_per_element(struct Curve *curve)
/*******************************************************************************
LAST MODIFIED : 16 November 1999

DESCRIPTION :
Returns the number of nodes in each element in the curve.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_get_nodes_per_element);
	if (curve)
	{
		return_code=curve->value_nodes_per_element;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_nodes_per_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_get_nodes_per_element */

int Curve_get_derivatives_per_node(struct Curve *curve)
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Returns the number of derivatives the curve stores at each node.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_get_derivatives_per_node);
	if (curve)
	{
		return_code=curve->value_derivatives_per_node;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_derivatives_per_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_get_derivatives_per_node */

enum FE_basis_type Curve_get_fe_basis_type(struct Curve *curve)
/*******************************************************************************
LAST MODIFIED : 15 November 1999

DESCRIPTION :
Returns the FE_basis_type used by the curve.
==============================================================================*/
{
	enum FE_basis_type fe_basis_type;

	ENTER(Curve_get_fe_basis_type);
	if (curve)
	{
		fe_basis_type=curve->fe_basis_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_fe_basis_type.  Invalid argument(s)");
		fe_basis_type=NO_RELATION;
	}
	LEAVE;

	return (fe_basis_type);
} /* Curve_get_fe_basis_type */

int Curve_set_fe_basis_type(struct Curve *curve,
	enum FE_basis_type fe_basis_type)
/*******************************************************************************
LAST MODIFIED : 26 July 2002

DESCRIPTION :
Allows the basis to be changed for an existing curve. The elements and node
values will be transformed into the new basis function, sometimes with a loss
of information, and when changing to cubic Hermite the slopes will be smoothed.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_set_fe_basis_type);
	if (curve)
	{
		if (fe_basis_type != curve->fe_basis_type)
		{
			if (cc_copy_convert_without_name(curve,fe_basis_type,
				curve->number_of_components,curve))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Curve_set_fe_basis_type.  Could not convert curve");
				return_code=0;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_set_fe_basis_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_set_fe_basis_type */

struct FE_field *Curve_get_value_field(struct Curve *curve)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Returns the value field used by the curve. Needed to access component names.
???RC Don't like this being available!
==============================================================================*/
{
	struct FE_field *field;

	ENTER(Curve_get_value_field);
	if (curve)
	{
		field=curve->value_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_value_field.  Invalid argument(s)");
		field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (field);
} /* Curve_get_value_field */

int Curve_get_node_values(struct Curve *curve,
	int element_no,int local_node_no,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Gets the field->number_of_components of value field at a node in the <curve>,
identified by the <element> it is in and the <local_node_no>. Calling function
must ensure <values> array can contain number_of_components FE_values.
==============================================================================*/
{
	int return_code;
	struct FE_element *element;
	struct FE_node *node;

	ENTER(Curve_get_node_values);
	if (curve&&values)
	{
		if ((element=cc_get_element(curve,element_no))&&
			(get_FE_element_node(element,local_node_no,&node))&&
			cc_get_node_field_values(node,curve->value_field,FE_NODAL_VALUE,values))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Curve_get_node_values.  Error getting node values");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_node_values.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_get_node_values */

int Curve_set_node_values(struct Curve *curve,
	int element_no,int local_node_no,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Sets the value field of a node in the <curve>, identified by the <element>
it is in and the <local_node_no>.
Call Curve_number_of_components to get the number of components to pass.
==============================================================================*/
{
	int i, return_code;
	struct FE_element *element;
	struct FE_node *node;

	ENTER(Curve_set_node_values);
	if (curve&&values)
	{
		if ((element=cc_get_element(curve,element_no))&&
			(get_FE_element_node(element,local_node_no,&node))&&
			cc_set_node_field_values(node,curve->value_field,FE_NODAL_VALUE,values))
		{
			/* SAB I have added this to automatically expand the
				range shown in the curve editor when it has a range
				that doesn't include the new values */
			for ( i = 0 ; i < curve->number_of_components ; i++ )
			{
				if ( values[i] < curve->min_value[i] )
				{
					curve->min_value[i] = values[i];
				}
				if ( values[i] > curve->max_value[i] )
				{
					curve->max_value[i] = values[i];
				}
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Curve_set_node_values.  Error setting node values");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_set_node_values.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_set_node_values */

int Curve_get_node_derivatives(struct Curve *curve,
	int element_no,int local_node_no,FE_value *derivatives)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Gets the field->number_of_components derivatives of a node in the <curve>,
identified by the <element> it is in and the <local_node_no>. Calling function
must ensure <derivatives> array can contain number_of_components FE_values.
==============================================================================*/
{
	int return_code;
	struct FE_element *element;
	struct FE_node *node;

	ENTER(Curve_get_node_derivatives);
	if (curve&&derivatives)
	{
		if ((element=cc_get_element(curve,element_no))&&
			(get_FE_element_node(element,local_node_no,&node))&&
			cc_get_node_field_values(node,curve->value_field,FE_NODAL_D_DS1,
				derivatives))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Curve_get_node_derivatives.  Error getting node derivatives");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_node_derivatives.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_get_node_derivatives */

int Curve_set_node_derivatives(struct Curve *curve,
	int element_no,int local_node_no,FE_value *derivatives)
/*******************************************************************************
LAST MODIFIED : 22 November 1999

DESCRIPTION :
Sets the derivatives of a node in the <curve>, identified by the <element>
it is in and the <local_node_no>.
Call Curve_number_of_components to get the number of components to pass.
Note no checks are made on the derivatives. A normal way to use the derivatives
is for them to be a unit vector and scaling factors at each local node adjusting
the length of the vector.
==============================================================================*/
{
	int return_code;
	struct FE_element *element;
	struct FE_node *node;

	ENTER(Curve_set_node_derivatives);
	if (curve&&derivatives)
	{
		if ((element=cc_get_element(curve,element_no))&&
			(get_FE_element_node(element,local_node_no,&node))&&
			cc_set_node_field_values(node,curve->value_field,FE_NODAL_D_DS1,
				derivatives))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Curve_set_node_derivatives.  Error setting node derivatives");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_set_node_derivatives.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_set_node_derivatives */

int Curve_get_scale_factor(struct Curve *curve,
	int element_no,int local_node_no,FE_value *scale_factor)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Gets the scale factor that the derivatives at <local_node_no> in <element_no>
are multiplied by - cubic Hermite basis only.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_get_scale_factor);
	if (curve&&scale_factor)
	{
		if (CUBIC_HERMITE==curve->fe_basis_type)
		{
			return_code=get_FE_element_scale_factor(cc_get_element(curve,element_no),
				local_node_no,scale_factor);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Curve_get_scale_factor.  "
				"Only cubic Hermite basis has scale factors");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_scale_factor.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_get_scale_factor */

int Curve_set_scale_factor(struct Curve *curve,
	int element_no,int local_node_no,FE_value scale_factor)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Sets the scale factor that the derivatives at <local_node_no> in <element_no>
are multiplied by - cubic Hermite basis only. Apply continuous velocities by
adjusting these values for the parameter change over the element.
See also Curve_set_node_derivatives.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_set_scale_factor);
	if (curve)
	{
		if (CUBIC_HERMITE==curve->fe_basis_type)
		{
			return_code=set_FE_element_scale_factor(cc_get_element(curve,element_no),
				local_node_no,scale_factor);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Curve_set_scale_factor.  "
				"Only cubic Hermite basis has scale factors");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_set_scale_factor.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_set_scale_factor */

int Curve_is_node_parameter_modifiable(struct Curve *curve,
	int local_node_no)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Returns true if the parameter can be changed for the given local_node_no.
Parameter may NOT be independently set for interior nodes, since parameter is
kept proportional to xi over each element.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_is_node_parameter_modifiable);
	if (curve&&(0<=local_node_no)&&(local_node_no<curve->value_nodes_per_element))
	{
		return_code=((0 == local_node_no) ||
			((curve->value_nodes_per_element-1) == local_node_no));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_is_node_parameter_modifiable.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_is_node_parameter_modifiable */

int Curve_get_parameter(struct Curve *curve,
	int element_no,int local_node_no,FE_value *parameter)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Returns the parameter at the given <element_no> <local_node_no> in <curve>.
==============================================================================*/
{
	FE_value f;
	int return_code;

	ENTER(Curve_get_parameter);
	if (curve&&(0<=local_node_no)&&
		(local_node_no<curve->value_nodes_per_element)&&parameter)
	{
		if (curve->parameter_table||cc_build_parameter_table(curve))
		{
			if ((0<element_no)&&(element_no<curve->parameter_table_size))
			{
				if (0 == local_node_no)
				{
					*parameter=curve->parameter_table[element_no-1];
				}
				else
				{
					if ((curve->value_nodes_per_element-1) == local_node_no)
					{
						*parameter=curve->parameter_table[element_no];
					}
					else
					{
						/* interior nodes: linearly interpolate to get parameter */
						f = (FE_value)local_node_no /
							((FE_value)curve->value_nodes_per_element-1.0);
						*parameter = (1.0-f)*curve->parameter_table[element_no-1] +
							f*curve->parameter_table[element_no];
					}
				}
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Curve_get_parameter.  Missing element");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Curve_get_parameter.  Could not build parameter table");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_parameter.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_get_parameter */

int Curve_set_parameter(struct Curve *curve,
	int element_no,int local_node_no,FE_value parameter)
/*******************************************************************************
LAST MODIFIED : 22 November 1999

DESCRIPTION :
Sets the parameter at the given <element_no> <local_node_no> in <curve>.
???RC Previously limited lower and upper bounds of parameter value.
==============================================================================*/
{
	int return_code;
	struct FE_element *element;
	struct FE_node *node;

	ENTER(Curve_get_parameter);
	if (curve&&((0==local_node_no)||
		((curve->value_nodes_per_element-1)==local_node_no)))
	{
		if (element=cc_get_element(curve,element_no))
		{
			if (get_FE_element_node(element,local_node_no,&node))
			{
				if (cc_set_node_field_values(node,curve->parameter_field,FE_NODAL_VALUE,
					&parameter))
				{
					cc_clear_parameter_table(curve);
					return_code=1;
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Curve_set_parameter.  Missing node");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Curve_set_parameter.  Missing element");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_set_parameter.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_set_parameter */

int Curve_get_edit_component_range(struct Curve *curve,
	int comp_no,FE_value *min_range,FE_value *max_range)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Returns the range of values allowed in component <comp_no> of the <curve>.
These values are used by the interactive curve editor.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_get_edit_component_range);
	/* check arguments */
	if (curve&&curve->min_value&&curve->max_value&&
		min_range&&max_range&&(0 <= comp_no)&&
		(comp_no<curve->number_of_components))
	{
		*max_range=curve->max_value[comp_no];
		*min_range=curve->min_value[comp_no];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_edit_component_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_get_edit_component_range */

int Curve_set_edit_component_range(struct Curve *curve,
	int comp_no,FE_value min_range,FE_value max_range)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Sets the range of values allowed in component <comp_no> of the <curve>.
These values are used by the interactive curve editor.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_set_edit_component_range);
	/* check arguments */
	if (curve&&curve->max_value&&curve->min_value&&
		(0 <= comp_no)&&(comp_no<curve->number_of_components))
	{
		/* always want max_value to be highest */
		if (min_range>max_range)
		{
			curve->max_value[comp_no]=min_range;
			curve->min_value[comp_no]=max_range;
		}
		else
		{
			curve->max_value[comp_no]=max_range;
			curve->min_value[comp_no]=min_range;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Curve_set_edit_component_range.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_set_edit_component_range */

int Curve_get_element_parameter_change(struct Curve *curve,
	int element_no,FE_value *parameter_change)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Returns the change in parameter over <element_no> of <curve>.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_get_element_parameter_change);
	if (curve&&parameter_change)
	{
		if (curve->parameter_table||cc_build_parameter_table(curve))
		{
			if ((0<element_no)&&(element_no<curve->parameter_table_size))
			{
				*parameter_change = curve->parameter_table[element_no] -
					curve->parameter_table[element_no-1];
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Curve_get_element_parameter_change.  Invalid element");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Curve_get_element_parameter_change.  "
				"Could not build parameter table");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_element_parameter_change.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_get_element_parameter_change */

int Curve_get_parameter_range(struct Curve *curve,
	FE_value *min_parameter,FE_value *max_parameter)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Returns the minimum and maximum parameter values in <curve>.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_get_parameter_range);
	if (curve&&min_parameter&&max_parameter)
	{
		if (curve->parameter_table||cc_build_parameter_table(curve))
		{
			if (1<curve->parameter_table_size)
			{
				*min_parameter = curve->parameter_table[0];
				*max_parameter = curve->parameter_table[curve->parameter_table_size-1];
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Curve_get_parameter_range.  Invalid element");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Curve_get_parameter_range.  "
				"Could not build parameter table");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_parameter_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_get_parameter_range */

int Curve_get_parameter_grid(struct Curve *curve,
	FE_value *parameter_grid)
/*******************************************************************************
LAST MODIFIED : 10 November 1999

DESCRIPTION :
Returns the <parameter_grid> of the <curve> = smallest change in parameter
allowed by the curve editor. Also controls display of grids in the editor.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_get_parameter_grid);
	if (curve&&parameter_grid)
	{
		*parameter_grid=curve->parameter_grid;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_parameter_grid.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_get_parameter_grid */

int Curve_set_parameter_grid(struct Curve *curve,
	FE_value parameter_grid)
/*******************************************************************************
LAST MODIFIED : 10 November 1999

DESCRIPTION :
Sets the <parameter_grid> of the <curve> = smallest change in parameter
allowed by the curve editor. Also controls display of grids in the editor.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_set_parameter_grid);
	if (curve&&(0.0 <= parameter_grid))
	{
		curve->parameter_grid=parameter_grid;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_set_parameter_grid.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_set_parameter_grid */

int Curve_get_value_grid(struct Curve *curve,
	FE_value *value_grid)
/*******************************************************************************
LAST MODIFIED : 22 November 1999

DESCRIPTION :
Returns the <value_grid> of the <curve> = smallest change in value
allowed by the curve editor. Also controls display of grids in the editor.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_get_value_grid);
	if (curve&&value_grid)
	{
		*value_grid=curve->value_grid;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_value_grid.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_get_value_grid */

int Curve_set_value_grid(struct Curve *curve,
	FE_value value_grid)
/*******************************************************************************
LAST MODIFIED : 10 November 1999

DESCRIPTION :
Sets the <value_grid> of the <curve> = smallest change in value
allowed by the curve editor. Also controls display of grids in the editor.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_set_value_grid);
	if (curve&&(0.0 <= value_grid))
	{
		curve->value_grid=value_grid;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_set_value_grid.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_set_value_grid */

enum Curve_extend_mode Curve_get_extend_mode(
	struct Curve *curve)
/*******************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Returns the mode used for evaluating the values returned for a parameter
outside the range of parameters defined for a curve. The definition of
enum Curve_extend_mode gives more information.
==============================================================================*/
{
	enum Curve_extend_mode extend_mode;

	ENTER(Curve_get_extend_mode);
	if (curve)
	{
		extend_mode = curve->extend_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_extend_mode.  Invalid argument(s)");
		extend_mode=CONTROL_CURVE_EXTEND_MODE_INVALID;
	}
	LEAVE;

	return (extend_mode);
} /* Curve_get_extend_mode */

int Curve_set_extend_mode(struct Curve *curve,
	enum Curve_extend_mode extend_mode)
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Sets the mode used for evaluating the values returned for a parameter outside the
range of parameters defined for a curve.  The definition of
enum Curve_extend_mode gives more information.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_set_extend_mode);
	/* check arguments */
	if (curve)
	{
		switch( extend_mode )
		{
			case CONTROL_CURVE_EXTEND_CLAMP:
			case CONTROL_CURVE_EXTEND_CYCLE:
			case CONTROL_CURVE_EXTEND_SWING:
			{
				curve->extend_mode = extend_mode;
				return_code=1;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Curve_set_extend_mode.  Unknown play mode");
				return_code=0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_set_extend_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_set_extend_mode */

int Curve_get_type(struct Curve *curve,
	enum Curve_type *type)
/*******************************************************************************
LAST MODIFIED : 15 April 1998

DESCRIPTION :
Returns the type which applications can use to distinguish between parameter curves
for other purposes and the parameter curves applicable to them.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_get_type);
	/* check arguments */
	if (curve&&type)
	{
		*type = curve->type;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_get_type */

int Curve_set_type(struct Curve *curve,
	enum Curve_type type)
/*******************************************************************************
LAST MODIFIED : 15 April 1998

DESCRIPTION :
Sets the type which applications can use to distinguish between parameter curves
for other purposes and the parameter curves applicable to them.  Applications
can define their own type in the enum Curve_type in curve/curve.h
==============================================================================*/
{
	int return_code;

	ENTER(Curve_set_type);
	/* check arguments */
	if (curve)
	{
		curve->type = type;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_set_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_set_type */

int Curve_get_values_at_parameter(struct Curve *curve,
	FE_value parameter,FE_value *values,FE_value *derivatives)
/*******************************************************************************
LAST MODIFIED : 16 November 1999

DESCRIPTION :
Returns <values> and optional <derivatives> - w.r.t. the parameter - of the
<curve> at <parameter>. Note that if the parameter change over an element is not
positive, zero derivatives are returned instead with a warning.
Call Curve_number_of_components to get number of components returned.
Calling function must ensure values and derivatives each allocated with enough
space for number_of_components in the curve.
==============================================================================*/
{
	FE_value delta_parameter,dist,*parameters,parameter_range,xi;
	int comp_no,element_no,number_of_elements,return_code;
	struct FE_element *element;

	ENTER(Curve_get_values_at_parameter);
	if (curve&&values)
	{
		if (curve->parameter_table||cc_build_parameter_table(curve))
		{
			return_code=1;
			if (1<curve->parameter_table_size)
			{
				parameters=curve->parameter_table;
				number_of_elements=curve->parameter_table_size-1;
				/* get change in parameter over whole curve */
				parameter_range=parameters[number_of_elements]-parameters[0];
				/* if parameter out of range, get the equivalent parameter in the range
					 for the current extend_mode */
				if (parameter < parameters[0])
				{
					switch (curve->extend_mode)
					{
						case CONTROL_CURVE_EXTEND_CLAMP:
						{
							parameter=parameters[0];
						} break;
						case CONTROL_CURVE_EXTEND_CYCLE:
						{
							if (0.0<parameter_range)
							{
								dist=floor((parameter-parameters[0])/parameter_range);
								parameter -= dist*parameter_range;
							}
							else
							{
								parameter=parameters[0];
							}
						} break;
						case CONTROL_CURVE_EXTEND_SWING:
						{
							if (0.0<parameter_range)
							{
								dist=floor((parameter-parameters[0])/parameter_range);
								parameter -= dist*parameter_range;
								if (1==(abs((int)dist) % 2))
								{
									/* play backwards */
									parameter = parameters[0] +
										parameters[number_of_elements] - parameter;
								}
							}
							else
							{
								parameter=parameters[0];
							}
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"Curve_get_values_at_parameter.  Invalid play mode");
							return_code=0;
						} break;
					}
				}
				else if (parameter > parameters[number_of_elements])
				{
					switch (curve->extend_mode)
					{
						case CONTROL_CURVE_EXTEND_CLAMP:
						{
							parameter=parameters[number_of_elements];
						} break;
						case CONTROL_CURVE_EXTEND_CYCLE:
						{
							if (0.0<parameter_range)
							{
								dist=floor((parameter-parameters[0])/parameter_range);
								parameter -= dist*parameter_range;
							}
							else
							{
								parameter=parameters[number_of_elements];
							}
						} break;
						case CONTROL_CURVE_EXTEND_SWING:
						{
							if (0.0<parameter_range)
							{
								dist=floor((parameter-parameters[0])/parameter_range);
								parameter -= dist*parameter_range;
								if (1==((int)dist % 2))
								{
									/* play backwards */
									parameter = parameters[0] +
										parameters[number_of_elements] - parameter;
								}
							}
							else
							{
								parameter=parameters[number_of_elements];
							}
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"Curve_get_values_at_parameter.  Invalid play mode");
							return_code=0;
						} break;
					}
				}
				if (return_code)
				{
					element_no=1;
					while (parameter>parameters[element_no])
					{
						element_no++;
					}
					if (element=cc_get_element(curve,element_no))
					{
						/* get change in parameter over this element */
						delta_parameter=parameters[element_no]-parameters[element_no-1];
						/* calculate xi = linearly proportional to parameter in element */
						if (0==delta_parameter)
						{
							xi=0.0;
						}
						else
						{
							xi=(parameter-parameters[element_no-1])/delta_parameter;
						}
						if (return_code=cc_calculate_element_field_values(element,xi,
							curve->value_field,values,derivatives))
						{
							if (derivatives)
							{
								/* convert derivatives to be w.r.t parameter instead of xi */
								if (0.0<delta_parameter)
								{
									for (comp_no=0;comp_no<curve->number_of_components;comp_no++)
									{
										derivatives[comp_no] /= delta_parameter;
									}
								}
								else
								{
									display_message(WARNING_MESSAGE,
										"Curve_get_values_in_element.  "
										"Parameter change not >0 in element; zero derivatives");
									for (comp_no=0;comp_no<curve->number_of_components;comp_no++)
									{
										derivatives[comp_no] = 0.0;
									}
								}
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Curve_get_values_at_parameter.  Missing element");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Curve_get_values_at_parameter.  Empty curve");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Curve_get_values_at_parameter.  "
				"Could not build parameter table");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_values_at_parameter.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return return_code;
} /* Curve_get_values_at_parameter */

int Curve_get_values_in_element(struct Curve *curve,
	int element_no,FE_value xi,FE_value *values,FE_value *derivatives)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Returns <values> and optional <derivatives> - w.r.t. xi - of the <curve> in
element <element_no> at <xi> (from 0.0 to 1.0).
Call Curve_number_of_components to get number of components returned.
Calling function must ensure values and derivatives each allocated with enough
space for number_of_components in the curve.
==============================================================================*/
{
	int return_code;
	struct FE_element *element;

	ENTER(Curve_get_values_in_element);
	if (curve&&(0.0 <= xi)&&(1.0 >= xi)&&values)
	{
		if (element=cc_get_element(curve,element_no))
		{
			if (cc_calculate_element_field_values(element,xi,curve->value_field,
				values,derivatives))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Curve_get_values_in_element.  Unable to calculate values");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Curve_get_values_in_element.  Missing element");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_values_in_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return return_code;
} /* Curve_get_values_in_element */

int Curve_calculate_component_over_element(
	struct Curve *curve,int element_no,int component_no,
	int num_segments,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Calculates the value of the given <component_no> of the <curve> in element
<element_no> at <num_segments>+1 points evenly spaced in xi from 0 to 1.
<values> must be preallocated with enough space for the FE_values.
Used for efficiently drawing the curve in the Curve editor.
==============================================================================*/
{
	FE_value float_num_segments,xi;
	int i,return_code;
	struct FE_element *element;
	struct FE_element_field_values *element_field_values;

	ENTER(Curve_calculate_component_over_element);
	if (curve&&values)
	{
		if (element=cc_get_element(curve,element_no))
		{
			if ((element_field_values = CREATE(FE_element_field_values)()) &&
				calculate_FE_element_field_values(element,curve->value_field,
				/*time*/0,/*calculate_derivatives*/0,element_field_values,
				/*top_level_element*/(struct FE_element *)NULL))
			{
				return_code=1;
				float_num_segments=(FE_value)num_segments;
				for (i=0;(i<=num_segments)&&return_code;i++)
				{
					xi=(FE_value)i/float_num_segments;
					if (!calculate_FE_element_field(component_no,element_field_values,
						&xi,&(values[i]),(FE_value *)NULL))
					{
						display_message(ERROR_MESSAGE,
							"cc_calculate_element_field_component_over_element.  "
							"Could not calculate element field");
						return_code=0;
					}
				}
				DESTROY(FE_element_field_values)(&element_field_values);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"cc_calculate_element_field_component_over_element.  "
					"Could not calculate element field values");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cc_calculate_element_field_component_over_element.  Missing element");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_calculate_component_over_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Curve_calculate_component_over_element */

int Curve_get_parameter_in_element(struct Curve *curve,
	int element_no,FE_value xi,FE_value *parameter)
/*******************************************************************************
LAST MODIFIED : 16 November 1999

DESCRIPTION :
Returns the <parameter> at <element_no> <xi> of <curve>.
==============================================================================*/
{
	int return_code;

	ENTER(Curve_get_parameter_in_element);
	if (curve&&(0.0 <= xi)&&(1.0 >= xi)&&parameter)
	{
		if (curve->parameter_table||cc_build_parameter_table(curve))
		{
			if ((0<element_no)&&(element_no<curve->parameter_table_size))
			{
				*parameter = curve->parameter_table[element_no-1] +
					xi*(curve->parameter_table[element_no]-
						curve->parameter_table[element_no-1]);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Curve_get_parameter_in_element.  Missing element");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Curve_get_parameter_in_element.  "
				"Could not build parameter table");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_get_parameter_in_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return return_code;
} /* Curve_get_parameter_in_element */

int Curve_find_node_at_parameter(struct Curve *curve,
	FE_value parameter, int *element_no, int *local_node_no )
/*******************************************************************************
LAST MODIFIED : 16 November 1999

DESCRIPTION :
If there is a node in <curve> exactly at <parameter>, the
<element_no> <local_node_no> for the first such node is returned.
If no such node exists the return code is zero.
==============================================================================*/
{
	int temp_element_no, temp_local_node_no, nodes_per_element, return_code;
	FE_value *parameters, temp_parameter, temp_xi;

	ENTER(Curve_find_node_at_parameter);
	if ( curve && element_no && local_node_no )
	{
		if (curve->parameter_table||cc_build_parameter_table(curve))
		{
			if (1<curve->parameter_table_size)
			{
				parameters=curve->parameter_table;
				temp_element_no = 1;
				while ( temp_element_no < curve->parameter_table_size &&
					parameters[temp_element_no] <= parameter )
				{
					temp_element_no++;
				}
				if ( parameter == parameters[temp_element_no-1] )
				{
					/* First node in element */
					*element_no = temp_element_no;
					*local_node_no = 0;
					return_code = 1;
				}
				else
				{
					nodes_per_element = Curve_get_nodes_per_element(curve);
					if ( parameter == parameters[temp_element_no])
					{
						/* Special case of last node in parameter curve */
						*element_no = temp_element_no;
						*local_node_no = nodes_per_element - 1;
						return_code = 1;
					}
					else
					{
						/* Check intermediate nodes if they exist */
						temp_local_node_no = 1;
						return_code = 0;
						while ((temp_local_node_no < nodes_per_element-1) && !return_code )
						{
							temp_xi = temp_local_node_no / (nodes_per_element - 1);
							temp_parameter = parameters[temp_element_no-1] * temp_xi
								+ parameters[temp_element_no] * (1.0 - temp_xi);
							if ( temp_parameter == parameter )
							{
								*element_no = temp_element_no;
								*local_node_no = temp_local_node_no;
								return_code = 1;
							}
							else
							{
								temp_local_node_no++;
							}
						}
					}
				}
			}
			else
			{
				/* no elements */
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Curve_get_parameter_in_element.  "
				"Could not build parameter table");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_find_node_at_parameter.  Invalid argument(s)");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* Curve_find_node_at_parameter */

int Curve_find_element_at_parameter(struct Curve *curve,
	FE_value parameter, int *element_no, FE_value *xi )
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
If there is an element in <curve> defined at <parameter>, returns the
<element_no> and <xi> value for that parameter in the first such element.
==============================================================================*/
{
	int temp_element_no,number_of_elements,return_code;
	FE_value *parameters;

	ENTER(Curve_find_element_at_parameter);
	if ( curve && element_no && xi )
	{
		if (curve->parameter_table||cc_build_parameter_table(curve))
		{
			parameters=curve->parameter_table;
			number_of_elements=curve->parameter_table_size-1;
			if ((1<curve->parameter_table_size)&&
				( parameter >= parameters[0] ) &&
				( parameter <= parameters[number_of_elements] ))
			{
				temp_element_no = 1;
				while ( temp_element_no < number_of_elements &&
					parameters[temp_element_no] <= parameter )
				{
					temp_element_no++;
				}
				if ( parameter == parameters[temp_element_no-1] )
				{
					*element_no = temp_element_no;
					*xi = 0;
				}
				else
				{
					*element_no = temp_element_no;
					*xi = (parameter - parameters[temp_element_no-1]) /
						( parameters[temp_element_no] - parameters[temp_element_no-1]);
				}
				return_code = 1;
			}
			else
			{
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Curve_find_element_at_parameter.  "
				"Could not build parameter table");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_find_element_at_parameter.  Invalid argument(s)");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* Curve_find_element_at_parameter */

struct Curve_definition
/*******************************************************************************
LAST MODIFIED : 29 November 1999

DESCRIPTION :
==============================================================================*/
{
	char *name;
	enum FE_basis_type fe_basis_type;
	int fe_basis_type_set;
	int number_of_components;
	int number_of_components_set;
	struct Curve *curve,*curve_to_be_modified;
	struct IO_stream_package *io_stream_package;
}; /* struct Curve_definition */

int define_Curve_information(struct Parse_state *state,
	void *curve_definition_void,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
==============================================================================*/
{
	char *extend_mode_string,*file_name,**valid_strings;
	enum Curve_extend_mode extend_mode;
	enum FE_basis_type new_fe_basis_type;
	FE_value *max_value,*min_value,parameter_grid,value_grid;
	int comp_no,existing_number_of_components,new_number_of_components,
		number_of_components,number_of_valid_strings,return_code;
	struct Curve *temp_curve;
	struct Curve_definition *curve_definition;
	struct Option_table *option_table;

	ENTER(define_Curve_information);
	USE_PARAMETER(dummy_void);
	if (state&&
		(curve_definition=(struct Curve_definition *)curve_definition_void))
	{
		file_name=(char *)NULL;
		min_value=(FE_value *)NULL;
		max_value=(FE_value *)NULL;
		number_of_components=curve_definition->number_of_components;
		if (ALLOCATE(min_value,FE_value,number_of_components)&&
			ALLOCATE(max_value,FE_value,number_of_components))
		{
			if (curve_definition->curve=CREATE(Curve)(curve_definition->name,
				curve_definition->fe_basis_type,curve_definition->number_of_components))
			{
				ACCESS(Curve)(curve_definition->curve);
				if (curve_definition->curve_to_be_modified)
				{
					existing_number_of_components=Curve_get_number_of_components(
						curve_definition->curve_to_be_modified);
					for (comp_no=0;comp_no<number_of_components;comp_no++)
					{
						if (comp_no < existing_number_of_components)
						{
							Curve_get_edit_component_range(
								curve_definition->curve_to_be_modified,comp_no,
								&(min_value[comp_no]),&(max_value[comp_no]));
						}
						else
						{
							Curve_get_edit_component_range(curve_definition->curve,
								comp_no,&(min_value[comp_no]),&(max_value[comp_no]));
						}
					}
					Curve_get_parameter_grid(
						curve_definition->curve_to_be_modified,&parameter_grid);
					Curve_get_value_grid(
						curve_definition->curve_to_be_modified,&value_grid);
					extend_mode=Curve_get_extend_mode(
						curve_definition->curve_to_be_modified);
				}
				else
				{
					for (comp_no=0;comp_no<number_of_components;comp_no++)
					{
						Curve_get_edit_component_range(curve_definition->curve,
							comp_no,&(min_value[comp_no]),&(max_value[comp_no]));
					}
					Curve_get_parameter_grid(curve_definition->curve,
						&parameter_grid);
					Curve_get_value_grid(curve_definition->curve,&value_grid);
					extend_mode=Curve_get_extend_mode(curve_definition->curve);
				}

				option_table=CREATE(Option_table)();
				/* extend mode */
				extend_mode_string=Curve_extend_mode_string(extend_mode);
				valid_strings=Curve_extend_mode_get_valid_strings(
					&number_of_valid_strings);
				Option_table_add_enumerator(option_table,number_of_valid_strings,
					valid_strings,&extend_mode_string);
				DEALLOCATE(valid_strings);
				/* file */
				Option_table_add_entry(option_table,"file",&file_name,
					(void *)1,set_name);
				/* max_value */
				Option_table_add_entry(option_table,"max_value",max_value,
					&number_of_components,set_float_vector);
				/* min_value */
				Option_table_add_entry(option_table,"min_value",min_value,
					&number_of_components,set_float_vector);
				/* parameter_grid */
				Option_table_add_entry(option_table,"parameter_grid",&parameter_grid,
					NULL,set_float_non_negative);
				/* value_grid */
				Option_table_add_entry(option_table,"value_grid",&value_grid,
					NULL,set_float_non_negative);
				if (return_code=Option_table_multi_parse(option_table,state))
				{
					if (file_name)
					{
						if (temp_curve=create_Curve_from_file(
							curve_definition->curve->name,file_name,
							curve_definition->io_stream_package))
						{
							if (curve_definition->fe_basis_type_set)
							{
								new_fe_basis_type=curve_definition->fe_basis_type;
							}
							else
							{
								new_fe_basis_type=Curve_get_fe_basis_type(temp_curve);
							}
							if (curve_definition->number_of_components_set)
							{
								new_number_of_components=curve_definition->number_of_components;
							}
							else
							{
								new_number_of_components=
									Curve_get_number_of_components(temp_curve);
							}
							if ((Curve_get_number_of_components(temp_curve)==
								new_number_of_components)&&(new_fe_basis_type ==
								(Curve_get_fe_basis_type(temp_curve))))
							{
								/* use this instead of curve created earlier */
								REACCESS(Curve)(&(curve_definition->curve),temp_curve);
							}
							else
							{
								if (!(return_code=cc_copy_convert_without_name(
									curve_definition->curve,new_fe_basis_type,
									new_number_of_components,temp_curve)))
								{
									display_message(ERROR_MESSAGE,
										"define_Curve_information.  "
										"Could not copy curve from file %s",file_name);
									DEACCESS(Curve)(&(curve_definition->curve));
									return_code=0;
								}
								DESTROY(Curve)(&temp_curve);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"define_Curve_information.  "
								"Could not read curve from file %s",file_name);
							DEACCESS(Curve)(&(curve_definition->curve));
							return_code=0;
						}
					}
					else if (curve_definition->curve_to_be_modified)
					{
						if (!(return_code=cc_copy_convert_without_name(
							curve_definition->curve,curve_definition->fe_basis_type,
							number_of_components,curve_definition->curve_to_be_modified)))
						{
							display_message(ERROR_MESSAGE,
								"define_Curve_information.  Could not copy curve");
						}
						new_number_of_components=number_of_components;
					}
					if (return_code)
					{
						for (comp_no=0;comp_no<number_of_components;comp_no++)
						{
							if (comp_no<new_number_of_components)
							{
								Curve_set_edit_component_range(curve_definition->curve,
									comp_no,min_value[comp_no],max_value[comp_no]);
							}
						}
						Curve_set_parameter_grid(curve_definition->curve,
							parameter_grid);
						Curve_set_value_grid(curve_definition->curve,
							value_grid);
						Curve_set_extend_mode(curve_definition->curve,
							Curve_extend_mode_from_string(extend_mode_string));
					}
				}
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Curve_information.  Could not create curve");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Curve_information.  Not enough memory");
			return_code=0;
		}
		if (min_value)
		{
			DEALLOCATE(min_value);
		}
		if (max_value)
		{
			DEALLOCATE(max_value);
		}
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Curve_information.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Curve_information */

int define_Curve_number_of_components(struct Parse_state *state,
	void *curve_definition_void,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 29 November 1999

DESCRIPTION :
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Curve_definition *curve_definition;
	struct Modifier_entry
		help_option_table[]=
		{
			{"<number_of_components #>",NULL,NULL,define_Curve_information},
			{NULL,NULL,NULL,NULL}
		};

	ENTER(define_Curve_number_of_components);
	USE_PARAMETER(dummy_void);
	if (state&&
		(curve_definition=(struct Curve_definition *)curve_definition_void))
	{
		if (current_token=state->current_token)
		{
			/* read the optional number_of_components parameter */
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (fuzzy_string_compare(current_token,"number_of_components"))
				{
					shift_Parse_state(state,1);
					if (current_token=state->current_token)
					{
						if ((1==sscanf(current_token," %d ",
							&(curve_definition->number_of_components)))&&
							(0<curve_definition->number_of_components))
						{
							curve_definition->number_of_components_set=1;
							shift_Parse_state(state,1);
							return_code=1;
						}
						else
						{
							return_code=0;
						}
					}
				}
				if (return_code)
				{
					return_code=define_Curve_information(state,
						curve_definition_void,(void *)NULL);
				}
			}
			else
			{
				/* write help */
				(help_option_table[0]).to_be_modified=curve_definition_void;
				return_code=process_option(state,help_option_table);
			}
		}
		else
		{
			return_code=define_Curve_information(state,
				curve_definition_void,(void *)NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Curve_number_of_components.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Curve_number_of_components */

int define_Curve_fe_basis_type(struct Parse_state *state,
	void *curve_definition_void,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 29 November 1999

DESCRIPTION :
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Curve_definition *curve_definition;
	struct Modifier_entry
		help_option_table[]=
		{
			{"<c.Hermite|c.Lagrange|l.Lagrange|q.Lagrange>",NULL,NULL,
			 define_Curve_number_of_components},
			{NULL,NULL,NULL,NULL}
		};

	ENTER(define_Curve_fe_basis_type);
	USE_PARAMETER(dummy_void);
	if (state&&
		(curve_definition=(struct Curve_definition *)curve_definition_void))
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
#if defined (OLD_CODE)
				/*???RC Currently can't handle these optional tokens that also know if
					set, hence leave with old parsing stuff for now */
				/* basis type */
				fe_basis_type_string=
					FE_basis_type_string(curve_definition->fe_basis_type);
				valid_strings=Curve_FE_basis_type_get_valid_strings(
					&number_of_valid_strings);
				Option_table_add_enumerator(option_table,number_of_valid_strings,
					valid_strings,&fe_basis_type_string);
				DEALLOCATE(valid_strings);
#endif /* defined (OLD_CODE) */
				/* read the optional fe_basis_type parameter */
				if (fuzzy_string_compare(current_token,"c.Hermite"))
				{
					curve_definition->fe_basis_type=CUBIC_HERMITE;
					curve_definition->fe_basis_type_set=1;
					shift_Parse_state(state,1);
				}
				else if (fuzzy_string_compare(current_token,"c.Lagrange"))
				{
					curve_definition->fe_basis_type=CUBIC_LAGRANGE;
					curve_definition->fe_basis_type_set=1;
					shift_Parse_state(state,1);
				}
				else if (fuzzy_string_compare(current_token,"l.Lagrange"))
				{
					curve_definition->fe_basis_type=LINEAR_LAGRANGE;
					curve_definition->fe_basis_type_set=1;
					shift_Parse_state(state,1);
				}
				else if (fuzzy_string_compare(current_token,"q.Lagrange"))
				{
					curve_definition->fe_basis_type=QUADRATIC_LAGRANGE;
					curve_definition->fe_basis_type_set=1;
					shift_Parse_state(state,1);
				}
				return_code=define_Curve_number_of_components(state,
					curve_definition_void,(void *)NULL);
			}
			else
			{
				/* write help */
				(help_option_table[0]).to_be_modified=curve_definition_void;
				return_code=process_option(state,help_option_table);
			}
		}
		else
		{
			return_code=define_Curve_information(state,
				curve_definition_void,(void *)NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Curve_fe_basis_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Curve_fe_basis_type */

int gfx_define_Curve(struct Parse_state *state,
	void *dummy_to_be_modified,void *curve_command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 November 1999

DESCRIPTION :
==============================================================================*/
{
	auto struct Modifier_entry
		help_option_table[]=
		{
			{"CURVE_NAME",NULL,NULL,define_Curve_fe_basis_type},
			{NULL,NULL,NULL,NULL}
		};
	char *current_token;
	int return_code;
	struct Curve_definition curve_definition;
	struct Curve_command_data *command_data;

	ENTER(gfx_define_Curve);
	USE_PARAMETER(dummy_to_be_modified);
	return_code=0;
	if (state)
	{
		if (command_data=(struct Curve_command_data *)curve_command_data_void)
		{
			if (current_token=state->current_token)
			{
				return_code=1;
				curve_definition.name=(char *)NULL;
				curve_definition.fe_basis_type=LINEAR_LAGRANGE;
				curve_definition.fe_basis_type_set=0;
				curve_definition.number_of_components=1;
				curve_definition.number_of_components_set=0;
				curve_definition.curve=(struct Curve *)NULL;
				curve_definition.curve_to_be_modified=(struct Curve *)NULL;
				curve_definition.io_stream_package=command_data->io_stream_package;
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					curve_definition.name=duplicate_string(current_token);
					if (curve_definition.curve_to_be_modified=
						FIND_BY_IDENTIFIER_IN_MANAGER(Curve,name)(
							curve_definition.name,command_data->curve_manager))
					{
						curve_definition.fe_basis_type=
							Curve_get_fe_basis_type(
								curve_definition.curve_to_be_modified);
						curve_definition.number_of_components=
							Curve_get_number_of_components(
								curve_definition.curve_to_be_modified);
					}
					shift_Parse_state(state,1);
					if (define_Curve_fe_basis_type(state,
						(void *)&curve_definition,(void *)NULL))
					{
						if (curve_definition.curve_to_be_modified)
						{
							return_code=MANAGER_MODIFY_NOT_IDENTIFIER(Curve,name)(
								curve_definition.curve_to_be_modified,curve_definition.curve,
								command_data->curve_manager);
						}
						else
						{
							return_code=ADD_OBJECT_TO_MANAGER(Curve)(
								curve_definition.curve,command_data->curve_manager);
						}
					}
					else
					{
						return_code=0;
					}
				}
				else
				{
					curve_definition.name=duplicate_string("CURVE_NAME");
					(help_option_table[0]).to_be_modified=(void *)&curve_definition;
					return_code=process_option(state,help_option_table);
				}
				if (curve_definition.name)
				{
					DEALLOCATE(curve_definition.name);
				}
				if (curve_definition.curve)
				{
					DEACCESS(Curve)(&(curve_definition.curve));
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing curve name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_define_Curve.  Missing curve_command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_define_Curve.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_define_Curve */

int gfx_destroy_Curve(struct Parse_state *state,
	void *dummy_to_be_modified,void *curve_manager_void)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Executes a GFX DESTROY CURVE command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct MANAGER(Curve) *curve_manager;
	struct Curve *curve;

	ENTER(gfx_destroy_Curve);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (curve_manager=
			(struct MANAGER(Curve) *)curve_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (curve=FIND_BY_IDENTIFIER_IN_MANAGER(Curve,name)(
						current_token,curve_manager))
					{
						return_code=REMOVE_OBJECT_FROM_MANAGER(Curve)(curve,
							curve_manager);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown curve: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," CURVE_NAME");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing curve name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_Curve.  Missing curve_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_Curve.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_Curve */

int gfx_list_Curve(struct Parse_state *state,
	void *dummy_to_be_modified,void *curve_manager_void)
/*******************************************************************************
LAST MODIFIED : 28 November 2000

DESCRIPTION :
Executes a GFX LIST CURVE.
==============================================================================*/
{
	int return_code;
	struct MANAGER(Curve) *curve_manager;
	struct Curve *curve;
	struct Option_table *option_table;

	ENTER(gfx_list_Curve);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (curve_manager=
		(struct MANAGER(Curve) *)curve_manager_void))
	{
		curve = (struct Curve *)NULL;
		option_table=CREATE(Option_table)();
		/* default option: curve name */
		Option_table_add_entry(option_table, (char *)NULL, &curve,
			curve_manager_void, set_Curve);
		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (curve)
			{
				return_code = list_Curve(curve, (void *)NULL);
			}
			else
			{
				display_message(INFORMATION_MESSAGE,"Control curves:\n");
				return_code = FOR_EACH_OBJECT_IN_MANAGER(Curve)(
					list_Curve, (void *)NULL, curve_manager);
			}
		}
		DESTROY(Option_table)(&option_table);
		if (curve)
		{
			DEACCESS(Curve)(&curve);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_Curve.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_Curve */

int list_Curve(struct Curve *curve,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 9 November 1999

DESCRIPTION :
Curve iterator function for writing out the names of curves.
==============================================================================*/
{
	int return_code;

	ENTER(list_Curve);
	USE_PARAMETER(dummy_void);
	if (curve)
	{
		display_message(INFORMATION_MESSAGE,"%s = %s, %d component(s)\n",
			curve->name,FE_basis_type_string(curve->fe_basis_type),
			curve->number_of_components);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"list_Curve.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Curve */

int set_Curve(struct Parse_state *state,void *curve_address_void,
	void *curve_manager_void)
/*******************************************************************************
LAST MODIFIED : 5 November 1999

DESCRIPTION :
Modifier function to set the curve from a command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct MANAGER(Curve) *curve_manager;
	struct Curve *temp_curve,**curve_address;

	ENTER(set_Curve);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((curve_address=(struct Curve **)curve_address_void)&&
					(curve_manager=
						(struct MANAGER(Curve) *)curve_manager_void))
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*curve_address)
						{
							DEACCESS(Curve)(curve_address);
							*curve_address=(struct Curve *)NULL;
						}
						return_code=1;
					}
					else
					{
						if (temp_curve=FIND_BY_IDENTIFIER_IN_MANAGER(Curve,name)(
							current_token,curve_manager))
						{
							if (*curve_address!=temp_curve)
							{
								DEACCESS(Curve)(curve_address);
								*curve_address=ACCESS(Curve)(temp_curve);
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_Curve.  Curve '%s' does not exist",
								current_token);
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Curve.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," CURVE_NAME|none");
				if (curve_address=(struct Curve **)curve_address_void)
				{
					if (temp_curve= *curve_address)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",temp_curve->name);
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[none]");
					}
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing curve name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Curve.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Curve */

int write_Curve(struct Curve *curve,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 16 April 2003

DESCRIPTION :
Writes <curve> to filename(s) stemming from the name of the curve,
eg. "name" -> name.curve.com name.curve.exnode name.curve.exelem
==============================================================================*/
{
	char *file_name;
	FILE *out_file;
	int comp_no,return_code;
	struct Cmiss_region *child_cmiss_region, *root_cmiss_region;

	ENTER(write_Curve);
	USE_PARAMETER(dummy_void);
	if (curve)
	{
		if (ALLOCATE(file_name,char,strlen(curve->name)+13))
		{
			return_code=1;
			sprintf(file_name,"%s.curve.com",curve->name);
			if (out_file=fopen(file_name,"w"))
			{
				fprintf(out_file,"gfx define curve %s %s number_of_components %d",
					curve->name,FE_basis_type_string(curve->fe_basis_type),
					curve->number_of_components);
				fprintf(out_file," %s",
					Curve_extend_mode_string(curve->extend_mode));
				fprintf(out_file," file %s",curve->name);
				fprintf(out_file," max_value");
				for (comp_no=0;comp_no<curve->number_of_components;comp_no++)
				{
					fprintf(out_file," %g",curve->max_value[comp_no]);
				}
				fprintf(out_file," min_value");
				for (comp_no=0;comp_no<curve->number_of_components;comp_no++)
				{
					fprintf(out_file," %g",curve->min_value[comp_no]);
				}
				fprintf(out_file," parameter_grid %g value_grid %g",
					curve->parameter_grid,curve->value_grid);
				fclose(out_file);
			}
			else
			{
				return_code=0;
			}
			/* current export code expects the FE_region to be in a child region */
			if ((root_cmiss_region = CREATE(Cmiss_region)()) &&
				(child_cmiss_region = CREATE(Cmiss_region)()) &&
				Cmiss_region_add_child_region(root_cmiss_region, child_cmiss_region,
					/*child_name*/curve->name, /*child_position*/0) &&
				Cmiss_region_attach_FE_region(child_cmiss_region, curve->fe_region))
			{
				sprintf(file_name,"%s.curve.exnode",curve->name);
				if (!write_exregion_file_of_name(file_name, root_cmiss_region,
					(char *)NULL, /*write_elements*/0, /*write_nodes*/1,
				  FE_WRITE_COMPLETE_GROUP, (struct FE_field_order_info *)NULL))
				{
					return_code = 0;
				}
				sprintf(file_name,"%s.curve.exelem",curve->name);
				if (!write_exregion_file_of_name(file_name, root_cmiss_region,
					(char *)NULL, /*write_elements*/1, /*write_nodes*/0,
				  FE_WRITE_COMPLETE_GROUP, (struct FE_field_order_info *)NULL))
				{
					return_code = 0;
				}
			}
			else
			{
				return_code=0;
			}
			if (root_cmiss_region)
			{
				DESTROY(Cmiss_region)(&root_cmiss_region);
			}
		}
		else
		{
			return_code=0;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,"write_Curve.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_Curve.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_Curve */

struct Curve *create_Curve_from_file(char *curve_name,
	char *file_name_stem, struct IO_stream_package *io_stream_package)
/*******************************************************************************
LAST MODIFIED : 3 September 2004

DESCRIPTION :
Appends extension '.curve.exnode' on to <file_name_stem> and reads in nodes from
it. Similarly reads elements in from <file_name_stem>.curve.exelem. Creates
temporary managers to use import_finite_element functions. Mesh is checked for
appropriateness to curve usage.
???RC Later autorange
==============================================================================*/
{
	char *file_name;
	enum FE_basis_type fe_basis_type;
	int number_of_components,return_code;
	struct Cmiss_region *child_cmiss_region, *cmiss_region, *element_region;
	struct Curve *curve;
	struct FE_basis *fe_basis;
	struct FE_region *fe_region;
	struct IO_stream *element_file,*node_file, *region_file;
	struct LIST(FE_element_shape) *element_shape_list;
	struct MANAGER(FE_basis) *basis_manager;

	ENTER(create_Curve_from_file);
	curve=(struct Curve *)NULL;
	if (curve_name&&file_name_stem)
	{
		if (curve=cc_create_blank(curve_name))
		{
			/* create group managers needed to use import_finite_element functions  */
			basis_manager = curve->basis_manager;
			element_shape_list = curve->element_shape_list;
			if (ALLOCATE(file_name,char,strlen(file_name_stem)+14))
			{
				cmiss_region = (struct Cmiss_region *)NULL;
				return_code=1;
				if (sprintf(file_name,"%s.curve.exnode",file_name_stem) &&
					(node_file=CREATE(IO_stream)(io_stream_package)) &&
					(IO_stream_open_for_read(node_file, file_name)) && 
					sprintf(file_name,"%s.curve.exelem",file_name_stem) &&
					(element_file=CREATE(IO_stream)(io_stream_package)) &&
					(IO_stream_open_for_read(element_file, file_name)))
				{
					element_region = (struct Cmiss_region *)NULL;
					if ((cmiss_region = read_exregion_file(node_file,
						basis_manager, element_shape_list, (struct FE_import_time_index *)NULL)) &&
						(element_region = read_exregion_file(element_file,
							basis_manager, element_shape_list, (struct FE_import_time_index *)NULL)) &&
						Cmiss_regions_FE_regions_can_be_merged(cmiss_region, element_region)
 						&&
						Cmiss_regions_merge_FE_regions(cmiss_region, element_region) &&
						Cmiss_region_get_child_region(cmiss_region, /*child_number*/0,
							&child_cmiss_region))
					{
						if (!(fe_region = Cmiss_region_get_FE_region(child_cmiss_region)))
						{
							return_code = 0;
						}
					}
					else
					{
						return_code = 0;
					}
					if (element_region)
					{
						DESTROY(Cmiss_region)(&element_region);
					}
					IO_stream_close(node_file);
					DESTROY(IO_stream)(&node_file);
					IO_stream_close(element_file);
					DESTROY(IO_stream)(&element_file);
				}
				else
				{
					/*???RC don't wish to use combined node & element files yet --
						format not agreed; better to wait for FieldML */
					if (sprintf(file_name,"%s.curve.exregion",file_name_stem) &&
						(region_file=CREATE(IO_stream)(io_stream_package)) &&
						(IO_stream_open_for_read(region_file, file_name)))
					{
						if (cmiss_region = read_exregion_file(region_file,
							basis_manager, element_shape_list, (struct FE_import_time_index *)NULL))
						{
							if (!(fe_region = Cmiss_region_get_FE_region(cmiss_region)))
							{
								return_code = 0;
							}
						}
						else
						{
							return_code = 0;
						}
						IO_stream_close(region_file);
						DESTROY(IO_stream)(&region_file);
					}
					else
					{
						display_message(ERROR_MESSAGE,"create_Curve_from_file.  "
							"Unable to read .exnode and .exelem or .exregion from template %s",
							file_name_stem);
						return_code = 0;
					}
				}
				if (return_code)
				{
					REACCESS(FE_region)(&(curve->fe_region), fe_region);
					/* now check mesh is appropriate for a Curve */
					if (curve->template_element=FE_region_get_first_FE_element_that(fe_region,
						(LIST_CONDITIONAL_FUNCTION(FE_element) *)NULL,(void *)NULL))
					{
						ACCESS(FE_element)(curve->template_element);
						get_FE_element_node(curve->template_element,0,
							&(curve->template_node));
						if (curve->template_node)
						{
							ACCESS(FE_node)(curve->template_node);
						}
						else
						{
							return_code=0;
						}
						if (!((curve->parameter_field=ACCESS(FE_field)(
							FE_region_get_FE_field_from_name(fe_region, "parameter")))&&
							(1==get_FE_field_number_of_components(curve->parameter_field))))
						{
							return_code=0;
						}
						if (!((curve->value_field=ACCESS(FE_field)(
							FE_region_get_FE_field_from_name(fe_region, "value")))&&
							(0<(number_of_components=
							get_FE_field_number_of_components(curve->value_field)))))
						{
							return_code=0;
						}
						if ((1==get_FE_element_dimension(curve->template_element))&&
							FE_element_field_is_standard_node_based(curve->template_element,
							curve->value_field) &&
							FE_element_field_get_component_FE_basis(curve->template_element,
							curve->value_field, /*component_number*/0, &fe_basis))
						{
						   FE_basis_get_xi_basis_type(fe_basis, /*xi_number*/0,
								&fe_basis_type);
						}
						else
						{
							return_code=0;
						}
						if (return_code)
						{
							if (!(cc_establish(curve,fe_basis_type,number_of_components)&&
								cc_build_parameter_table(curve)))
							{
								display_message(ERROR_MESSAGE,
									"create_Curve_from_file.  "
									"Could not set basis type and number of components");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_Curve_from_file.  Invalid curve file(s)");
						}
					}
					else
					{
						/* nothing in the mesh - leave behaviour to calling function */
						return_code=0;
					}
				}
				if (cmiss_region)
				{
					DESTROY(Cmiss_region)(&cmiss_region);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Curve_from_file.  Could not create managers");
				return_code=0;
			}
			if (!return_code)
			{
				DESTROY(Curve)(&curve);
			}
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Curve_from_file.  Could not create curve");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Curve_from_file.  Invalid argument(s)");
	}
	LEAVE;

	return (curve);
} /* create_Curve_from_file */
