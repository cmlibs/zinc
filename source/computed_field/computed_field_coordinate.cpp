/*******************************************************************************
FILE : computed_field_coordinate.c

LAST MODIFIED : 28 October 2004

DESCRIPTION :
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
#include <stdio.h>
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_wrappers.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"

int Computed_field_extract_rc(struct Computed_field *field,
	int element_dimension,FE_value *rc_coordinates,FE_value *rc_derivatives)
/*******************************************************************************
LAST MODIFIED : 9 February 1999

DESCRIPTION :
Takes the values in <field> and converts them from their current coordinate
system into rectangular cartesian, returning them in the 3 component
<rc_coordinates> array. If <rc_derivatives> is not NULL, the derivatives are
also converted to rc and returned in that 9-component FE_value array.
Note that odd coordinate systems, such as FIBRE are treated as if they are
RECTANGULAR_CARTESIAN, which just causes a copy of values.
If <element_dimension> or the number of components in <field> are less than 3,
the missing places in the <rc_coordinates> and <rc_derivatives> arrays are
cleared to zero.
???RC Uses type float for in-between values x,y,z and jacobian for future
compatibility with coordinate system transformation functions in geometry.c.
This causes a slight drop in performance.

Note the order of derivatives:
1. All the <element_dimension> derivatives of component 1.
2. All the <element_dimension> derivatives of component 2.
3. All the <element_dimension> derivatives of component 3.
==============================================================================*/
{
	FE_value *source;
	float coordinates[3],derivatives[9],*destination,x,y,z,*jacobian,temp[9];
	int field_components,i,j,return_code;
	
	ENTER(Computed_field_extract_rc);
	if (field&&rc_coordinates&&(((FE_value *)NULL==rc_derivatives)||
		((0<element_dimension)&&field->derivatives_valid)))
	{
		field_components=field->number_of_components;
		/* copy coordinates, padding to 3 components */
		for (i=0;i<3;i++)
		{
			if (i<field_components)
			{
				coordinates[i] = (float)field->values[i];
			}
			else
			{
				coordinates[i]=0.0;
			}
		}
		if (rc_derivatives)
		{
			/* copy derivatives, padding to 3 components x 3 dimensions */
			destination=derivatives;
			source=field->derivatives;
			for (i=0;i<3;i++)
			{
				for (j=0;j<3;j++)
				{
					if ((i<field_components)&&(j<element_dimension))
					{
						*destination = (float)(*source);
						source++;
					}
					else
					{
						*destination = 0.0;
					}
					destination++;
				}
			}
			/* make sure jacobian only calculated if rc_derivatives requested */
			jacobian=temp;
		}
		else
		{
			jacobian=(float *)NULL;
		}

		switch (field->coordinate_system.type)
		{
			case CYLINDRICAL_POLAR:
			{
				cylindrical_polar_to_cartesian(
					coordinates[0],coordinates[1],coordinates[2],&x,&y,&z,jacobian);
			} break;
			case SPHERICAL_POLAR:
			{
				spherical_polar_to_cartesian(
					coordinates[0],coordinates[1],coordinates[2],&x,&y,&z,jacobian);
			} break;
			case PROLATE_SPHEROIDAL:
			{
				prolate_spheroidal_to_cartesian(
					coordinates[0],coordinates[1],coordinates[2],
					field->coordinate_system.parameters.focus,&x,&y,&z,jacobian);
			} break;
			case OBLATE_SPHEROIDAL:
			{
				oblate_spheroidal_to_cartesian(
					coordinates[0],coordinates[1],coordinates[2],
					field->coordinate_system.parameters.focus,&x,&y,&z,jacobian);
			} break;
			default:
			{
				/* treat all others as RECTANGULAR_CARTESIAN; copy coordinates */
				x=coordinates[0];
				y=coordinates[1];
				z=coordinates[2];
				if (rc_derivatives)
				{
					for (i=0;i<9;i++)
					{
						rc_derivatives[i]=(FE_value)(derivatives[i]);
					}
					/* clear jacobian to avoid derivative conversion below */
					jacobian=(FE_value *)NULL;
				}
			} break;
		}
		rc_coordinates[0]=(FE_value)x;
		rc_coordinates[1]=(FE_value)y;
		rc_coordinates[2]=(FE_value)z;
		if (jacobian)
		{
			for (i=0;i<3;i++)
			{
				/* derivative of x with respect to xi[i] */
				rc_derivatives[i]=(FE_value)(
					jacobian[0]*derivatives[0+i]+
					jacobian[1]*derivatives[3+i]+
					jacobian[2]*derivatives[6+i]);
				/* derivative of y with respect to xi[i] */
				rc_derivatives[3+i]=(FE_value)(
					jacobian[3]*derivatives[0+i]+
					jacobian[4]*derivatives[3+i]+
					jacobian[5]*derivatives[6+i]);
				/* derivative of z with respect to xi[i] */
				rc_derivatives[6+i]=(FE_value)(
					jacobian[6]*derivatives[0+i]+
					jacobian[7]*derivatives[3+i]+
					jacobian[8]*derivatives[6+i]);
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_extract_rc.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_extract_rc */

struct Computed_field_coordinate_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
};

static char computed_field_coordinate_transformation_type_string[] = "coordinate_transformation";

int Computed_field_is_type_coordinate_transformation(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_coordinate_transformation);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_coordinate_transformation_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_coordinate_transformation.  Missing field.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_coordinate_transformation */

#define Computed_field_coordinate_transformation_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_coordinate_transformation_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_coordinate_transformation_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_coordinate_transformation_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_coordinate_transformation_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_coordinate_transformation_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_coordinate_transformation_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_coordinate_transformation_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_evaluate_coordinate_transformation(
	struct Computed_field *field, int element_dimension,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Function called by Computed_field_evaluate_cache_in_element/at_node to compute
values in the coordinate system of this field from the source_field values in an 
arbitrary coordinate system.
NOTE: Assumes that values and derivatives arrays are already allocated in
<field>, and that its source_fields are already computed (incl. derivatives if
calculate_derivatives set) for the same element, with the given
<element_dimension> = number of Xi coords.
==============================================================================*/
{
	FE_value *destination,*dx_dX,temp[9];
	int i,j,return_code;
	
	ENTER(Computed_field_evaluate_rc_coordinate);
	if (field&&Computed_field_is_type_coordinate_transformation(field))
	{
		if (calculate_derivatives)
		{
			dx_dX=temp;
		}
		else
		{
			dx_dX=(FE_value *)NULL;
		}
		if (return_code=convert_Coordinate_system(
			&(field->source_fields[0]->coordinate_system),
			field->source_fields[0]->number_of_components,
			field->source_fields[0]->values,
			&(field->coordinate_system),field->number_of_components,field->values,
			dx_dX))
		{
			if (calculate_derivatives)
			{
				destination=field->derivatives;
				for (i=0;i<field->number_of_components;i++)
				{
					for (j=0;j<element_dimension;j++)
					{
						*destination= dx_dX[0+i*3] * 
							field->source_fields[0]->
							derivatives[j+element_dimension*0]
							+dx_dX[1+i*3] * 
							field->source_fields[0]->
							derivatives[j+element_dimension*1]
							+dx_dX[2+i*3] * 
							field->source_fields[0]->
							derivatives[j+element_dimension*2];
						destination++;
					}
				}
				field->derivatives_valid = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_rc_coordinate.  Could not convert to RC");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_rc_coordinate.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_rc_coordinate */

static int Computed_field_coordinate_transformation_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_coordinate_transformation_evaluate_cache_at_node);
	if (field && node && (0 < field->number_of_source_fields))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			return_code=Computed_field_evaluate_coordinate_transformation(field,
				/*element_dimension*/0,/*calculate_derivatives*/0);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_coordinate_transformation_evaluate_cache_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_coordinate_transformation_evaluate_cache_at_node */

static int Computed_field_coordinate_transformation_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int element_dimension, return_code;

	ENTER(Computed_field_coordinate_transformation_evaluate_cache_in_element);
	if (field && element && xi && (0 < field->number_of_source_fields))
	{
		element_dimension=get_FE_element_dimension(element);
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			return_code=Computed_field_evaluate_coordinate_transformation(field,
				element_dimension,calculate_derivatives);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_coordinate_transformation_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_coordinate_transformation_evaluate_cache_in_element */

#define Computed_field_coordinate_transformation_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_coordinate_transformation_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

static int Computed_field_coordinate_transformation_set_values_at_node(
	struct Computed_field *field,struct FE_node *node,FE_value time,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 28 October 2004

DESCRIPTION :
Sets the <values> of the computed <field> at <node>.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_coordinate_transformation_set_values_at_node);
	if (field && node && values)
	{
		FE_value source_field_coordinates[3];
		
		/* convert this fields values back into source coordinate system */
		return_code=
			convert_Coordinate_system(&(field->coordinate_system),
			field->number_of_components,values,
			&(field->source_fields[0]->coordinate_system),
			field->source_fields[0]->number_of_components,
			source_field_coordinates,
			/*jacobian*/(float *)NULL)&&
			Computed_field_set_values_at_node(field->source_fields[0],
			node,time,source_field_coordinates);
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_coordinate_transformation_set_values_at_node.  "
				"Could not set coordinate_transformation field %s at node",field->name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_coordinate_transformation_set_values_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_coordinate_transformation_set_values_at_node */

static int Computed_field_coordinate_transformation_set_values_in_element(
	struct Computed_field *field, struct FE_element *element,int *number_in_xi,
	FE_value time, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 28 October 2004

DESCRIPTION :
Sets the <values> of the computed <field> over the <element>.
==============================================================================*/
{
	FE_value local_field_coordinates[3], source_field_coordinates[3],
		*source_values;
	int element_dimension,i,j,k,number_of_points,return_code;

	ENTER(Computed_field_coordinate_transformation_set_values_in_element);
	if (field && element && number_in_xi && values)
	{
		return_code=1;
		element_dimension=get_FE_element_dimension(element);
		number_of_points=1;
		for (i=0;(i<element_dimension)&&return_code;i++)
		{
			if (0<number_in_xi[i])
			{
				number_of_points *= (number_in_xi[i]+1);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_coordinate_transformation_set_values_in_element.  "
					"number_in_xi must be positive");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* 3 values for non-rc coordinate system */
			if (ALLOCATE(source_values,FE_value,number_of_points*3))
			{
				for (j=0;(j<number_of_points)&&return_code;j++)
				{
					for (k=0;k<3;k++)
					{
						local_field_coordinates[k]=values[k*number_of_points+j];
					}
					/* convert RC values back into source coordinate system */
					if (convert_Coordinate_system(&(field->coordinate_system),
						3,local_field_coordinates,&(field->source_fields[0]->coordinate_system),
						field->source_fields[0]->number_of_components,
						source_field_coordinates,/*jacobian*/(float *)NULL))
					{
						for (k=0;k<3;k++)
						{
							source_values[k*number_of_points+j]=source_field_coordinates[k];
						}
					}
					else
					{
						return_code=0;
					}
				}
				if (return_code)
				{
					return_code=Computed_field_set_values_in_element(
						field->source_fields[0],element,number_in_xi,time,source_values);
				}
				DEALLOCATE(source_values);
			}
			else
			{
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_coordinate_transformation_set_values_in_element.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_coordinate_transformation_set_values_in_element */

#define Computed_field_coordinate_transformation_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_coordinate_transformation_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

static int Computed_field_coordinate_transformation_find_element_xi(
	struct Computed_field *field,
	FE_value *values, int number_of_values, struct FE_element **element, 
	FE_value *xi, int element_dimension, struct Cmiss_region *search_region) 
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_coordinate_transformation_find_element_xi);
	if (field && element && xi && values && (field->number_of_components == number_of_values))
	{
		FE_value source_field_coordinates[3];
		
		/* convert this fields values back into source coordinate system */
		return_code=convert_Coordinate_system(&(field->coordinate_system),
			number_of_values,values, &(field->source_fields[0]->coordinate_system),
			field->source_fields[0]->number_of_components, source_field_coordinates,
			/*jacobian*/(float *)NULL) && Computed_field_find_element_xi(
			field->source_fields[0],source_field_coordinates,
			field->source_fields[0]->number_of_components,element,
			xi, element_dimension, search_region, /*propagate_field*/1,
			/*find_nearest_location*/0);
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_coordinate_transformation_find_element_xi.  "
				"Could not set coordinate_transformation field %s at node",field->name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_coordinate_transformation_find_element_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_coordinate_transformation_find_element_xi */

static int list_Computed_field_coordinate_transformation(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_coordinate_transformation);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_coordinate_transformation.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_coordinate_transformation */

static char *Computed_field_coordinate_transformation_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_coordinate_transformation_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_coordinate_transformation_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_coordinate_transformation_get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_coordinate_transformation_get_command_string */

#define Computed_field_coordinate_transformation_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_coordinate_transformation(
	struct Computed_field *field, struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_COORDINATE_TRANSFORMATION with the supplied
<source_field>.  Sets the number of components equal to the <source_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_coordinate_transformation);
	if (field&&source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_coordinate_transformation_type_string;
			field->number_of_components = 3;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(coordinate_transformation);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_coordinate_transformation.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_coordinate_transformation */

int Computed_field_get_type_coordinate_transformation(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
If the field is of type COMPUTED_FIELD_COORDINATE_TRANSFORMATION, the 
<source_field> used by it is returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_coordinate_transformation);
	if (field && (field->type_string == 
		computed_field_coordinate_transformation_type_string) && source_field)
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_coordinate_transformation.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_coordinate_transformation */

static int define_Computed_field_type_coordinate_transformation(struct Parse_state *state,
	void *field_void,void *computed_field_coordinate_package_void)
/*******************************************************************************
LAST MODIFIED : 18 December 2001

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_COORDINATE_TRANSFORMATION (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,*source_field;
	struct Computed_field_coordinate_package 
		*computed_field_coordinate_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_coordinate_transformation);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_coordinate_package=
		(struct Computed_field_coordinate_package *)
		computed_field_coordinate_package_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		if (computed_field_coordinate_transformation_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code =
				Computed_field_get_type_coordinate_transformation(field, &source_field);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			option_table = CREATE(Option_table)();
			/* field */
			set_source_field_data.computed_field_manager=
				computed_field_coordinate_package->computed_field_manager;
			set_source_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				&set_source_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_coordinate_transformation(field, source_field);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_coordinate_transformation.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_coordinate_transformation.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_coordinate_transformation */

static char computed_field_vector_coordinate_transformation_type_string[] = "vector_coordinate_transformation";

int Computed_field_is_type_vector_coordinate_transformation(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_vector_coordinate_transformation);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_vector_coordinate_transformation_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_vector_coordinate_transformation.  Missing field.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_vector_coordinate_transformation */

#define Computed_field_vector_coordinate_transformation_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_vector_coordinate_transformation_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_vector_coordinate_transformation_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_vector_coordinate_transformation_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_vector_coordinate_transformation_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_vector_coordinate_transformation_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_vector_coordinate_transformation_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_vector_coordinate_transformation_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
No special criteria.
==============================================================================*/

int Computed_field_evaluate_vector_coordinate_transformation(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Function called by Computed_field_evaluate_cache_in_element/at_node to compute
a vector in a different coordinate system from one that it may already be in.
The <field> must be of type COMPUTED_FIELD_VECTOR_COORDINATE_TRANSFORMATION, 
which gives it a source vector field - describing 1, 2 or 3 vectors - and a 
coordinate field which says where in space the vector is being converted. 
The function uses the jacobian between the old coordinate system and the new
one at the coordinate position to get the new vector. Hence, derivatives of 
the converted vectors are not available.
==============================================================================*/
{
	FE_value cx[3],jacobian[9],*source,sum,x[3];
	int coordinates_per_vector,i,j,k,number_of_vectors,return_code;

	ENTER(Computed_field_evaluate_vector_coordinate_transformation);
	if (field&&Computed_field_is_type_vector_coordinate_transformation(field))
	{
		if (return_code=(convert_Coordinate_system(
				&(field->source_fields[1]->coordinate_system),
				field->source_fields[1]->number_of_components,
				field->source_fields[1]->values,
				&(field->source_fields[0]->coordinate_system),3,cx,
				/*jacobian*/(float *)NULL)&&
			convert_Coordinate_system(&(field->source_fields[0]->coordinate_system),
				3,cx,&(field->coordinate_system),3,x,jacobian)))
		{
			number_of_vectors=field->number_of_components/3;
			coordinates_per_vector=
				field->source_fields[0]->number_of_components/number_of_vectors;
			source=field->source_fields[0]->values;
			for (i=0;i<number_of_vectors;i++)
			{
				for (j=0;j<3;j++)
				{
					sum=0.0;
					for (k=0;k<coordinates_per_vector;k++)
					{
						sum += jacobian[j*3+k]*source[i*coordinates_per_vector+k];
					}
					field->values[i*3+j]=sum;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_vector_coordinate_transformation.  Could not convert to RC");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_vector_coordinate_transformation.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_vector_coordinate_transformation */

static int Computed_field_vector_coordinate_transformation_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_vector_coordinate_transformation_evaluate_cache_at_node);
	if (field && node && (0 < field->number_of_source_fields))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			return_code=Computed_field_evaluate_vector_coordinate_transformation(
				field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_vector_coordinate_transformation_evaluate_cache_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_vector_coordinate_transformation_evaluate_cache_at_node */

static int Computed_field_vector_coordinate_transformation_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_vector_coordinate_transformation_evaluate_cache_in_element);
	if (field && element && xi && (0 < field->number_of_source_fields))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			return_code=Computed_field_evaluate_vector_coordinate_transformation(
				field);
			/* no derivatives for this type */
			field->derivatives_valid=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_vector_coordinate_transformation_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_vector_coordinate_transformation_evaluate_cache_in_element */

#define Computed_field_vector_coordinate_transformation_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_vector_coordinate_transformation_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

static int Computed_field_vector_coordinate_transformation_set_values_at_node(
	struct Computed_field *field,struct FE_node *node,FE_value time,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 28 October 2004

DESCRIPTION :
Sets the <values> of the computed <field> at <node>.
==============================================================================*/
{
	FE_value jacobian[9],source_field_coordinates[3],*source_values,sum;
	int coordinates_per_vector,i,j,k,number_of_vectors,return_code;

	ENTER(Computed_field_vector_coordinate_transformation_set_values_at_node);
	if (field && node && values)
	{
		/* need jacobian at current coordinate position for converting to
			coordinate system of source vector field (=source_fields[0]) */
		if (Computed_field_evaluate_cache_at_node(field->source_fields[1],node,time)
			&&
			convert_Coordinate_system(&(field->source_fields[1]->coordinate_system),
				field->source_fields[1]->number_of_components,
				field->source_fields[1]->values,
				&(field->source_fields[0]->coordinate_system),
				field->source_fields[0]->number_of_components,source_field_coordinates,
				jacobian))
		{
			if (ALLOCATE(source_values,FE_value,field->number_of_components))
			{
				number_of_vectors=field->number_of_components/3;
				coordinates_per_vector=
					field->source_fields[0]->number_of_components/number_of_vectors;
				for (i=0;i<number_of_vectors;i++)
				{
					for (j=0;j<coordinates_per_vector;j++)
					{
						sum=0.0;
						for (k=0;k<3;k++)
						{
							sum += jacobian[j*3+k]*values[i*3+k];
						}
						source_values[i*coordinates_per_vector+j]=sum;
					}
				}
				return_code=Computed_field_set_values_at_node(
					field->source_fields[0],node,time,source_values);
				DEALLOCATE(source_values);
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			return_code=0;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_vector_coordinate_transformation_set_values_at_node.  "
				"Could not set vector_coordinate_transformation field %s at node",field->name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_vector_coordinate_transformation_set_values_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_vector_coordinate_transformation_set_values_at_node */

static int Computed_field_vector_coordinate_transformation_set_values_in_element(
	struct Computed_field *field, struct FE_element *element,int *number_in_xi,
	FE_value time, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 28 October 2004

DESCRIPTION :
Sets the <values> of the computed <field> over the <element>.
==============================================================================*/
{
	FE_value *source_values;
	int element_dimension,i,j,k,number_of_points,return_code;
	FE_value jacobian[9],non_rc_coordinates[3],
		rc_coordinates[3],*rc_coordinate_values,sum;
	int coordinates_per_vector,m,number_of_vectors;
	struct Computed_field *rc_coordinate_field;
	
	ENTER(Computed_field_vector_coordinate_transformation_set_values_in_element);
	if (field && element && number_in_xi && values)
	{
		return_code=1;
		element_dimension=get_FE_element_dimension(element);
		number_of_points=1;
		for (i=0;(i<element_dimension)&&return_code;i++)
		{
			if (0<number_in_xi[i])
			{
				number_of_points *= (number_in_xi[i]+1);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_vector_coordinate_transformation_set_values_in_element.  "
					"number_in_xi must be positive");
				return_code=0;
			}
		}

		if (return_code)
		{
			if (rc_coordinate_field=Computed_field_begin_wrap_coordinate_field(
				field->source_fields[1]))
			{
				if (Computed_field_get_values_in_element(rc_coordinate_field,
					element,number_in_xi, time, &rc_coordinate_values))
				{
					/* 3 values for non-rc coordinate system */
					if (ALLOCATE(source_values,FE_value,
						number_of_points*field->source_fields[0]->number_of_components))
					{
						for (j=0;(j<number_of_points)&&return_code;j++)
						{
							for (k=0;k<3;k++)
							{
								rc_coordinates[k]=
									rc_coordinate_values[k*number_of_points+j];
							}
							/* need jacobian at current coordinate position for
								converting to coordinate system of source vector field
								(=source_fields[0]) */
							if (convert_Coordinate_system(&(field->coordinate_system),
								3,rc_coordinates,&field->source_fields[0]->coordinate_system,
								3,non_rc_coordinates,jacobian))
							{
								number_of_vectors=field->number_of_components/3;
								coordinates_per_vector=
									field->source_fields[0]->number_of_components/
									number_of_vectors;
								for (i=0;i<number_of_vectors;i++)
								{
									for (m=0;m<coordinates_per_vector;m++)
									{
										sum=0.0;
										for (k=0;k<3;k++)
										{
											sum +=
												jacobian[m*3+k]*values[(i*3+k)*number_of_points+j];
										}
										source_values[(i*coordinates_per_vector+m)*
											number_of_points+j]=sum;
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
							return_code=Computed_field_set_values_in_element(
								field->source_fields[0],element,number_in_xi,time,source_values);
						}
						DEALLOCATE(source_values);
					}
					DEALLOCATE(rc_coordinate_values);
				}
				else
				{
					return_code=0;
				}
				Computed_field_end_wrap(&rc_coordinate_field);
			}
			else
			{
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_vector_coordinate_transformation_set_values_in_element.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_vector_coordinate_transformation_set_values_in_element */

#define Computed_field_vector_coordinate_transformation_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_vector_coordinate_transformation_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_vector_coordinate_transformation_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

static int list_Computed_field_vector_coordinate_transformation(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_vector_coordinate_transformation);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    vector field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    coordinate field : %s\n",field->source_fields[1]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_vector_coordinate_transformation.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_vector_coordinate_transformation */

static char *Computed_field_vector_coordinate_transformation_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_vector_coordinate_transformation_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_vector_coordinate_transformation_type_string, &error);
		append_string(&command_string, " vector ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " coordinate ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_vector_coordinate_transformation_get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_vector_coordinate_transformation_get_command_string */

#define Computed_field_vector_coordinate_transformation_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_vector_coordinate_transformation(
	struct Computed_field *field,
	struct Computed_field *vector_field,struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_RC_VECTOR, combining a vector field
supplying a single vector (1,2 or 3 components), two vectors (4 or 6 components)
or three vectors (9 components) with a coordinate field. This field type ensures
that each source vector is converted to RC coordinates at the position given by
the coordinate field - as opposed to RC_COORDINATE which assumes the
transformation is always based at the origin.
Sets the number of components to 3 times the number of vectors expected from
the source vector_field.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_vector_coordinate_transformation);
	if (field&&vector_field&&coordinate_field&&
		Computed_field_is_orientation_scale_capable(vector_field,(void *)NULL)&&
		Computed_field_has_up_to_3_numerical_components(coordinate_field,
			(void *)NULL))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_vector_coordinate_transformation_type_string;
			if (3 >= vector_field->number_of_components)
			{
				field->number_of_components=3;
			}
			else if (6 >= vector_field->number_of_components)
			{
				field->number_of_components=6;
			}
			else
			{
				field->number_of_components=9;
			}
			source_fields[0]=ACCESS(Computed_field)(vector_field);
			source_fields[1]=ACCESS(Computed_field)(coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(vector_coordinate_transformation);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_vector_coordinate_transformation.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_vector_coordinate_transformation */

int Computed_field_get_type_vector_coordinate_transformation(struct Computed_field *field,
	struct Computed_field **vector_field,
	struct Computed_field **coordinate_field)
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
If the field is of type COMPUTED_FIELD_VECTOR_COORDINATE_TRANSFORMATION, the 
<vector_field> and <coordinate_field> used by it is returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_vector_coordinate_transformation);
	if (field && (field->type_string == 
		computed_field_vector_coordinate_transformation_type_string) &&
		vector_field && coordinate_field)
	{
		*vector_field = field->source_fields[0];
		*coordinate_field = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_vector_coordinate_transformation.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_vector_coordinate_transformation */

static int define_Computed_field_type_vector_coordinate_transformation(struct Parse_state *state,
	void *field_void,void *computed_field_coordinate_package_void)
/*******************************************************************************
LAST MODIFIED : 18 December 2001

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_VECTOR_COORDINATE_TRANSFORMATION (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *coordinate_field,*field,*vector_field;
	struct Computed_field_coordinate_package 
		*computed_field_coordinate_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_vector_field_data;

	ENTER(define_Computed_field_type_vector_coordinate_transformation);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_coordinate_package=
		(struct Computed_field_coordinate_package *)
		computed_field_coordinate_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		coordinate_field = (struct Computed_field *)NULL;
		vector_field = (struct Computed_field *)NULL;
		if (computed_field_vector_coordinate_transformation_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_vector_coordinate_transformation(field, &vector_field, &coordinate_field);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (coordinate_field)
			{
				ACCESS(Computed_field)(coordinate_field);
			}
			if (vector_field)
			{
				ACCESS(Computed_field)(vector_field);
			}
			option_table = CREATE(Option_table)();
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				computed_field_coordinate_package->computed_field_manager;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* vector */
			set_vector_field_data.computed_field_manager=
				computed_field_coordinate_package->computed_field_manager;
			set_vector_field_data.conditional_function=
				Computed_field_is_orientation_scale_capable;
			set_vector_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"vector",&vector_field,
				&set_vector_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_vector_coordinate_transformation(
					field, vector_field, coordinate_field);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_vector_coordinate_transformation.  Failed");
				}
			}
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (vector_field)
			{
				DEACCESS(Computed_field)(&vector_field);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_vector_coordinate_transformation.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_vector_coordinate_transformation */

int Computed_field_register_types_coordinate(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 8 November 2001

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_coordinate_package 
		computed_field_coordinate_package;

	ENTER(Computed_field_register_types_coordinate);
	if (computed_field_package)
	{
		computed_field_coordinate_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_coordinate_transformation_type_string,
			define_Computed_field_type_coordinate_transformation,
			&computed_field_coordinate_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_vector_coordinate_transformation_type_string,
			define_Computed_field_type_vector_coordinate_transformation,
			&computed_field_coordinate_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_coordinate.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_coordinate */
