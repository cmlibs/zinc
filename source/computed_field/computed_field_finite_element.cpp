/*******************************************************************************
FILE : computed_field_finite_element.c

LAST MODIFIED : 17 July 2000

DESCRIPTION :
Implements a number of basic component wise operations on computed fields.
==============================================================================*/
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_private.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_finite_element.h"

struct Computed_field_finite_element_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	void *fe_field_manager_callback_id;
};

struct Computed_field_finite_element_type_specific_data
{
	struct FE_field *fe_field;
	struct FE_element_field_values *fe_element_field_values;
};

static char computed_field_finite_element_type_string[] = "finite_element";

int Computed_field_is_type_finite_element(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_finite_element);
	if (field)
	{
		return_code = (field->type_string == computed_field_finite_element_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_finite_element.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_type_finite_element */

static int Computed_field_finite_element_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_finite_element_clear_type_specific);
	if (field&& (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		if (data->fe_field)
		{
			DEACCESS(FE_field)(&(data->fe_field));
		}
		if (data->fe_element_field_values)
		{
			DEALLOCATE(data->fe_element_field_values);
		}
		
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element_clear_type_specific */

static void *Computed_field_finite_element_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_finite_element_type_specific_data *destination,
		*source;

	ENTER(Computed_field_finite_element_copy_type_specific);
	if (field && (source = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_finite_element_type_specific_data, 1))
		{
			if (source->fe_field)
			{
				destination->fe_field=ACCESS(FE_field)(source->fe_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_finite_element_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_finite_element_copy_type_specific */

int Computed_field_finite_element_clear_cache_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_finite_element_clear_cache_type_specific);
	if (field && (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		if(data->fe_element_field_values&&data->fe_element_field_values->element)
		{
			clear_FE_element_field_values(data->fe_element_field_values);
			/* clear element to indicate that values are clear */
			data->fe_element_field_values->element=(struct FE_element *)NULL;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element_clear_cache_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element_clear_cache_type_specific */

static int Computed_field_finite_element_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_finite_element_type_specific_data *data, *other_data;

	ENTER(Computed_field_finite_element_type_specific_contents_match);
	if (field && other_computed_field && (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data) && (other_data = 
		(struct Computed_field_finite_element_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		return_code = (data->fe_field == other_data->fe_field);
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element_type_specific_contents_match */

int Computed_field_finite_element_is_defined_in_element(struct Computed_field *field,
	struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_finite_element_is_defined_in_element);
	if (field && element && (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		return_code=1;
		if (data->fe_field)
		{
			if (!FE_field_is_defined_in_element(data->fe_field,element))
			{
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element_is_defined_in_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element_is_defined_in_element */

int Computed_field_finite_element_is_defined_at_node(struct Computed_field *field,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Check the fe_field
==============================================================================*/
{
	int return_code;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_finite_element_is_defined_at_node);
	if (field && node && (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		return_code=FE_field_is_defined_at_node(data->fe_field,node);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element_is_defined_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element_is_defined_at_node */

int Computed_field_finite_element_has_numerical_components(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_finite_element_has_numerical_components);
	if (field && (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		return_code=Value_type_is_numeric_simple(
			get_FE_field_value_type(data->fe_field));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element_has_numerical_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_finite_element_has_numerical_components */

int Computed_field_finite_element_can_be_destroyed(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_finite_element_type_specific_data *data;
	struct FE_field *temp_fe_field;

	ENTER(Computed_field_finite_element_can_be_destroyed);
	if (field && (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		/* check the fe_field can be destroyed */
		temp_fe_field=data->fe_field;
		DEACCESS(FE_field)(&temp_fe_field);
		return_code=FE_field_can_be_destroyed(data->fe_field);
		ACCESS(FE_field)(data->fe_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element_can_be_destroyed.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_finite_element_can_be_destroyed */

static int Computed_field_calculate_FE_element_field_values_for_element(
	struct Computed_field *field,int calculate_derivatives,
	struct FE_element *element,struct FE_element *top_level_element)
/*******************************************************************************
LAST MODIFIED : 24 May 2000

DESCRIPTION :
Establishes the FE_element_field values necessary for evaluating <field>, which
must be of type COMPUTED_FIELD_FINITE_ELEMENT. <calculate_derivatives> flag
controls whether basis functions for derivatives are also evaluated. If <field>
is already set up in the correct way, does nothing.
==============================================================================*/
{
	int return_code;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_calculate_FE_element_field_values_for_element);
	if (field&&element&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_finite_element_type_string)&&
		(data = (struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		return_code=1;
		/* clear the cache if values already cached for a node */
		if (field->node)
		{
			Computed_field_clear_cache(field);
		}
		/* ensure we have FE_element_field_values for element, with
			 derivatives_calculated if requested */
		if ((!data->fe_element_field_values)||
			(!FE_element_field_values_are_for_element(
				data->fe_element_field_values,element,top_level_element))||
			(calculate_derivatives&&
				(!data->fe_element_field_values->derivatives_calculated)))
		{
			if (!data->fe_element_field_values)
			{
				if (ALLOCATE(data->fe_element_field_values,
					struct FE_element_field_values,1))
				{
					/* clear element to indicate that values are clear */
					data->fe_element_field_values->element=
						(struct FE_element *)NULL;
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				if (data->fe_element_field_values->element)
				{
					/* following clears fe_element_field_values->element */
					clear_FE_element_field_values(data->fe_element_field_values);
				}
			}
			if (return_code)
			{
				/* note that FE_element_field_values accesses the element */
				if (!calculate_FE_element_field_values(element,data->fe_field,
					calculate_derivatives,data->fe_element_field_values,
					top_level_element))
				{
					/* clear element to indicate that values are clear */
					data->fe_element_field_values->element=(struct FE_element *)NULL;
					return_code=0;
				}
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_calculate_FE_element_field_values_for_element.  "
				"Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_calculate_FE_element_field_values_for_element.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_calculate_FE_element_field_values_for_element */

static int Computed_field_finite_element_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	double double_value;
	enum Value_type value_type;
	float float_value;
	int i, int_value, return_code;
	struct Computed_field_finite_element_type_specific_data *data;
	struct FE_field_component fe_field_component;

	ENTER(Computed_field_finite_element_evaluate_cache_at_node);
	if (field && node && (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		return_code = 1;
		/* not very efficient - should cache FE_node_field or similar */
		fe_field_component.field=data->fe_field;
		value_type=get_FE_field_value_type(data->fe_field);
		for (i=0;(i<field->number_of_components)&&return_code;i++)
		{
			fe_field_component.number=i;
			switch (value_type)
			{
				case DOUBLE_VALUE:
				{
					return_code=get_FE_nodal_double_value(node,
						&fe_field_component,/*version_number*/0,
						/*nodal_value_type*/FE_NODAL_VALUE, &double_value);
					field->values[i] = (FE_value)double_value;
				} break;
				case FE_VALUE_VALUE:
				{
					return_code=get_FE_nodal_FE_value_value(node,
						&fe_field_component,/*version_number*/0,
						/*nodal_value_type*/FE_NODAL_VALUE,&(field->values[i]));
				} break;
				case FLT_VALUE:
				{
					return_code=get_FE_nodal_float_value(node,&fe_field_component,
						/*version_number*/0,/*nodal_value_type*/FE_NODAL_VALUE,&float_value);
					field->values[i] = (FE_value)float_value;
				} break;
				case INT_VALUE:
				{
					return_code=get_FE_nodal_int_value(node,&fe_field_component,
						/*version_number*/0,/*nodal_value_type*/FE_NODAL_VALUE,&int_value);
					field->values[i] = (FE_value)int_value;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_finite_element_evaluate_cache_at_node.  "
						"Unsupported value type %s in finite_element field",
						Value_type_string(value_type));
					return_code=0;
				}
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_finite_element_evaluate_cache_at_node.  "
				"Error evaluating finite_element field at node");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element_evaluate_cache_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element_evaluate_cache_at_node */

static int Computed_field_finite_element_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	enum Value_type value_type;
	int i, *int_values, return_code;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_finite_element_evaluate_cache_in_element);
	if (field && element && xi && (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		/* 1. Precalculate any source fields that this field depends on.
			For type COMPUTED_FIELD_FINITE_ELEMENT, this means getting
			FE_element_field_values.*/
		if (return_code=
			Computed_field_calculate_FE_element_field_values_for_element(
				field,calculate_derivatives,element,top_level_element))
		{
			/* 2. Calculate the field */
			value_type=get_FE_field_value_type(data->fe_field);
			/* component number -1 = calculate all components */
			switch (value_type)
			{
				case FE_VALUE_VALUE:
				{
					if (calculate_derivatives)
					{
						return_code=calculate_FE_element_field(-1,
							data->fe_element_field_values,xi,field->values,
							field->derivatives);
						field->derivatives_valid = 1;
					}
					else
					{
						return_code=calculate_FE_element_field(-1,
							data->fe_element_field_values,xi,field->values,
							(FE_value *)NULL);
					}
				} break;
				case INT_VALUE:
				{
					/* no derivatives for this value_type */
					field->derivatives_valid=0;
					if (calculate_derivatives)
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_finite_element_evaluate_cache_in_element.  "
							"Derivatives not defined for integer fields");
						return_code=0;
					}
					else
					{
						if (ALLOCATE(int_values,int,field->number_of_components))
						{
							return_code=calculate_FE_element_field_int_values(-1,
								data->fe_element_field_values,xi,int_values);
							for (i=0;i<field->number_of_components;i++)
							{
								field->values[i]=(FE_value)int_values[i];
							}
							DEALLOCATE(int_values);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_finite_element_evaluate_cache_in_element.  "
								"Not enough memory for int_values");
							return_code=0;
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_finite_element_evaluate_cache_in_element.  "
						"Unsupported value type %s in finite_element field",
						Value_type_string(value_type));
					return_code=0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element_evaluate_cache_in_element */

char *Computed_field_finite_element_evaluate_as_string_at_node(struct Computed_field *field,
	int component_number, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/
{
	char *return_string, *temp_string;
	int error, i;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_finite_element_evaluate_as_string_at_node);
	return_string = (char *)NULL;
	if (field && node && (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		if (FE_VALUE_VALUE == get_FE_field_value_type(data->fe_field))
		{
			/* Then we can just use the default function and therefore use the
				values if they are already in the cache */
			return_string = Computed_field_default_evaluate_as_string_at_node(
				field, component_number, node);
		}
		else
		{
			if (-1 == component_number)
			{
				if (get_FE_nodal_value_as_string(node,data->fe_field,
					/*component_number*/0,/*version_number*/0,
					/*nodal_value_type*/FE_NODAL_VALUE,&return_string))
				{
					error=0;
					for (i=1;i<field->number_of_components;i++)
					{
						if (get_FE_nodal_value_as_string(node,data->fe_field,
							i,/*version_number*/0,/*nodal_value_type*/FE_NODAL_VALUE,
							&temp_string))
						{
							append_string(&return_string,",",&error);
							append_string(&return_string,temp_string,&error);
							DEALLOCATE(temp_string);
						}
					}
				}
			}
			else
			{
				if (get_FE_nodal_value_as_string(node,data->fe_field,
					component_number,/*version_number*/0,
					/*nodal_value_type*/FE_NODAL_VALUE,&return_string))
				{
					error=0;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element_evaluate_as_string_at_node.  "
			"Invalid arguments.");
		return_string = (char *)NULL;
	}
	LEAVE;

	return (return_string);
} /* Computed_field_finite_element_evaluate_as_string_at_node */

char *Computed_field_finite_element_evaluate_as_string_in_element(
	struct Computed_field *field,int component_number,
	struct FE_element *element,FE_value *xi,struct FE_element *top_level_element)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/
{
	char *return_string;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_finite_element_evaluate_as_string_in_element);
	return_string = (char *)NULL;
	if (field && element && xi && (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		if (FE_VALUE_VALUE == get_FE_field_value_type(data->fe_field))
		{
			/* Then we can just use the default function and therefore use the
				values if they are already in the cache */
			return_string = Computed_field_default_evaluate_as_string_in_element(
				field, component_number, element, xi, top_level_element);
		}
		else
		{
			/* ensure we have FE_element_field_values for element, without
				requiring derivatives to be calculated  */
			if (Computed_field_calculate_FE_element_field_values_for_element(
				field,/*calculate_derivatives*/0,element,top_level_element))
			{
				calculate_FE_element_field_as_string(component_number,
					data->fe_element_field_values,xi,&return_string);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element_evaluate_as_string_in_element.  "
			"Invalid arguments.");
		return_string = (char *)NULL;
	}
	LEAVE;

	return (return_string);
} /* Computed_field_finite_element_evaluate_as_string_in_element */

int Computed_field_finite_element_set_values_at_node(struct Computed_field *field,
	struct FE_node *node,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Sets the <values> of the computed <field> at <node>.
==============================================================================*/
{
	double double_value;
	enum Value_type value_type;
	float float_value;
	int i,int_value,j,k,return_code;
	struct Computed_field_finite_element_type_specific_data *data;
	struct FE_field_component fe_field_component;

	ENTER(Computed_field_finite_element_set_values_at_node);
	if (field&&node&&values && (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		return_code=1;

		fe_field_component.field=data->fe_field;
		value_type=get_FE_field_value_type(data->fe_field);
		for (i=0;(i<field->number_of_components)&&return_code;i++)
		{
			fe_field_component.number=i;
			/* set values all versions; to set values for selected version only,
				use COMPUTED_FIELD_NODE_VALUE instead */
			k=get_FE_node_field_component_number_of_versions(node,
				data->fe_field,i);
			for (j=0;(j<k)&&return_code;j++)
			{
				switch (value_type)
				{
					case DOUBLE_VALUE:
					{
						double_value=(double)values[i];
						return_code=set_FE_nodal_double_value(node,&fe_field_component,
							j,/*nodal_value_type*/FE_NODAL_VALUE,double_value);
					} break;
					case FE_VALUE_VALUE:
					{
						return_code=set_FE_nodal_FE_value_value(node,&fe_field_component,
							j,/*nodal_value_type*/FE_NODAL_VALUE,values[i]);
					} break;
					case FLT_VALUE:
					{
						float_value=(float)values[i];
						return_code=set_FE_nodal_float_value(node,&fe_field_component,
							j,/*nodal_value_type*/FE_NODAL_VALUE,float_value);
					} break;
					case INT_VALUE:
					{
						int_value=(int)floor(values[i]+0.5);
						return_code=set_FE_nodal_float_value(node,&fe_field_component,
							j,/*nodal_value_type*/FE_NODAL_VALUE,int_value);
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_finite_element_set_values_at_node.  "
							"Could not set finite_element field %s at node",field->name);
						return_code=0;
					}
				}
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_finite_element_set_values_at_node.  "
				"Could not set finite_element field %s at node",field->name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element_set_values_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element_set_values_at_node */

int Computed_field_finite_element_set_values_in_element(
	struct Computed_field *field, struct FE_element *element,int *number_in_xi,
	FE_value *values)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Sets the <values> of the computed <field> over the <element>.
==============================================================================*/
{
	enum Value_type value_type;
	int element_dimension,grid_map_number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],
		i,*int_values,j,k,number_of_points,offset,return_code;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_finite_element_set_values_in_element);
	if (field&&element&&element->shape&&number_in_xi&&values && (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		return_code=1;
		element_dimension=element->shape->dimension;
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
					"Computed_field_finite_element_set_values_in_element.  "
					"number_in_xi must be positive");
				return_code=0;
			}
		}
		if (return_code)
		{
			if (FE_element_field_is_grid_based(element,data->fe_field)&&
				get_FE_element_field_grid_map_number_in_xi(element,
					data->fe_field,grid_map_number_in_xi))
			{
				for (i=0;(i<element_dimension)&&return_code;i++)
				{
					if (number_in_xi[i] != grid_map_number_in_xi[i])
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_set_values_in_element.  "
							"Finite element has different grid number_in_xi");
						return_code=0;
					}
				}
				if (return_code)
				{
					value_type=get_FE_field_value_type(data->fe_field);
					switch (value_type)
					{
						case FE_VALUE_VALUE:
						{
							for (k=0;(k<field->number_of_components)&&return_code;k++)
							{
								if (!set_FE_element_field_component_grid_FE_value_values(
									element,data->fe_field,k,values+k*number_of_points))
								{
									display_message(ERROR_MESSAGE,
										"Computed_field_finite_element_set_values_in_element.  "
										"Unable to set finite element grid FE_value values");
									return_code=0;
								}
							}
						} break;
						case INT_VALUE:
						{
							if (ALLOCATE(int_values,int,number_of_points))
							{
								for (k=0;(k<field->number_of_components)&&return_code;k++)
								{
									offset=k*number_of_points;
									for (j=0;j<number_of_points;j++)
									{
										/*???RC this conversion could be a little dodgy */
										int_values[j]=(int)(values[offset+j]);
									}
									if (!set_FE_element_field_component_grid_int_values(
										element,data->fe_field,k,int_values))
									{
										display_message(ERROR_MESSAGE,
											"Computed_field_finite_element_set_values_in_element.  "
											"Unable to set finite element grid int values");
										return_code=0;
									}
								}
								DEALLOCATE(int_values);
							}
							else
							{
								return_code=0;
							}
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_finite_element_set_values_in_element.  "
								"Unsupported value_type for finite_element field");
							return_code=0;
						} break;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_finite_element_set_values_in_element.  "
					"Finite element field %s is not grid based in element",
					field->name);
				return_code=0;
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_finite_element_set_values_in_element.  "
				"Failed for field %s.",field->name,
				Computed_field_type_to_string(field));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element_set_values_in_element.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element_set_values_in_element */
	
int Computed_field_finite_element_get_native_discretization_in_element(
	struct Computed_field *field,struct FE_element *element,int *number_in_xi)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
If the <field> ois grid-based in <element>, returns in
<number_in_xi> the numbers of finite difference cells in each xi-direction of
<element>. Note that this number is one less than the number of grid points in
each direction. <number_in_xi> should be allocated with at least as much space
as the number of dimensions in <element>, but is assumed to have no more than
MAXIMUM_ELEMENT_XI_DIMENSIONS so that
int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS] can be passed to this function.
Returns 0 with no errors if the field is not grid-based.
==============================================================================*/
{
	int return_code;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_get_native_discretization_in_element);
	if (field&&element&&number_in_xi&&element->shape&&
		(MAXIMUM_ELEMENT_XI_DIMENSIONS>=element->shape->dimension) && (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		if (FE_element_field_is_grid_based(element,data->fe_field))
		{
			return_code=get_FE_element_field_grid_map_number_in_xi(element,
				data->fe_field,number_in_xi);
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_native_discretization_in_element.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_native_discretization_in_element */

int Computed_field_finite_element_find_element_xi(struct Computed_field *field, 
	FE_value *values, int number_of_values, struct FE_element **element,
	FE_value *xi, struct GROUP(FE_element) *search_element_group)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
==============================================================================*/
{
	int i, number_of_xi, return_code;
	struct Computed_field_iterative_find_element_xi_data find_element_xi_data;

	ENTER(Computed_field_find_element_xi);
	if (field&&values&&(number_of_values==field->number_of_components)&&element&&xi&&
		search_element_group)
	{
		/* Attempt to find correct element by searching */
		find_element_xi_data.field = field;
		find_element_xi_data.values = values;
		find_element_xi_data.number_of_values = number_of_values;
		find_element_xi_data.found_number_of_xi = 0;
		find_element_xi_data.found_derivatives = (FE_value *)NULL;
		find_element_xi_data.tolerance = 1e-06;
		if (ALLOCATE(find_element_xi_data.found_values, FE_value, number_of_values))
		{
			*element = (struct FE_element *)NULL;
					
			/* Try the cached element first */
			if (!*element && field->element && Computed_field_iterative_element_conditional(
				field->element, (void *)&find_element_xi_data))
			{
				*element = field->element;
			}
			/* Now try every element */
			if (!*element)
			{
				*element = FIRST_OBJECT_IN_GROUP_THAT(FE_element)
					(Computed_field_iterative_element_conditional,
						&find_element_xi_data, search_element_group);
			}
			if (*element)
			{
				number_of_xi = get_FE_element_dimension(*element);
				for (i = 0 ; i < number_of_xi ; i++)
				{
						xi[i] = find_element_xi_data.xi[i];
				}
			}
			/* The search is valid even if the element wasn't found */
			return_code = 1;
			DEALLOCATE(find_element_xi_data.found_values);
			if (find_element_xi_data.found_derivatives)
			{
				DEALLOCATE(find_element_xi_data.found_derivatives);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_find_element_xi.  Unable to allocate value memory.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_find_element_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_find_element_xi */

static int list_Computed_field_finite_element(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
==============================================================================*/
{
	char *field_name;
	int return_code;

	ENTER(List_Computed_field_finite_element);
	if (field)
	{
		if (return_code=GET_NAME(FE_field)(field->fe_field,&field_name))
		{
			display_message(INFORMATION_MESSAGE,"    fe_field : %s\n",field_name);
			DEALLOCATE(field_name);
		}
		display_message(INFORMATION_MESSAGE,"    CM field type : %s\n",
			CM_field_type_string(get_FE_field_CM_field_type(field->fe_field)));
		display_message(INFORMATION_MESSAGE,"    Value type : %s\n",
			Value_type_string(get_FE_field_value_type(field->fe_field)));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_finite_element.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_finite_element */

static int list_Computed_field_finite_element_commands(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
==============================================================================*/
{
	char *field_name;
	int return_code;

	ENTER(list_Computed_field_finite_element_commands);
	if (field)
	{
		if (return_code=GET_NAME(FE_field)(field->fe_field,&field_name))
		{
			display_message(INFORMATION_MESSAGE," fe_field %s",field_name);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_finite_element_commands.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_finite_element_commands */

int Computed_field_set_type_finite_element(struct Computed_field *field,
	struct FE_field *fe_field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_FINITE_ELEMENT, wrapping the given
<fe_field>. Makes the number of components the same as in the <fe_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	char **component_names;
	int i, number_of_components, return_code;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_set_type_finite_element);
	if (field&&fe_field)
	{
		return_code=1;
		number_of_components = get_FE_field_number_of_components(fe_field);
		/* 1. make dynamic allocations for any new type-specific data */
		if (ALLOCATE(component_names, char *, number_of_components))
		{
			for (i = 0 ; i < number_of_components; i++)
			{
				if (!(component_names[i]=get_FE_field_component_name(fe_field,i)))
				{
					return_code = 0;
				}
			}
		}
		else
		{
			return_code = 0;
		}
		if (return_code &&
			ALLOCATE(data,struct Computed_field_finite_element_type_specific_data, 1))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = computed_field_finite_element_type_string;
			field->number_of_components = number_of_components;
			field->component_names = component_names;
			field->type_specific_data = (void *)data;
			data->fe_field = ACCESS(FE_field)(fe_field);
			data->fe_element_field_values = (struct FE_element_field_values *)NULL;

			/* Set all the methods */
			field->computed_field_clear_type_specific_function =
				Computed_field_finite_element_clear_type_specific;
			field->computed_field_copy_type_specific_function =
				Computed_field_finite_element_copy_type_specific;
			field->computed_field_clear_cache_type_specific_function =
				Computed_field_finite_element_clear_cache_type_specific;
			field->computed_field_type_specific_contents_match_function =
				Computed_field_finite_element_type_specific_contents_match;
			field->computed_field_is_defined_in_element_function =
				Computed_field_finite_element_is_defined_in_element;
			field->computed_field_is_defined_at_node_function =
				Computed_field_finite_element_is_defined_at_node;
			field->computed_field_has_numerical_components_function =
				Computed_field_finite_element_has_numerical_components;
			field->computed_field_can_be_destroyed_function =
				Computed_field_finite_element_can_be_destroyed;
			field->computed_field_evaluate_cache_at_node_function =
				Computed_field_finite_element_evaluate_cache_at_node;
			field->computed_field_evaluate_cache_in_element_function =
				Computed_field_finite_element_evaluate_cache_in_element;
			field->computed_field_evaluate_as_string_at_node_function =
				Computed_field_finite_element_evaluate_as_string_at_node;
			field->computed_field_evaluate_as_string_in_element_function =
				Computed_field_finite_element_evaluate_as_string_in_element;
			field->computed_field_set_values_at_node_function =
				Computed_field_finite_element_set_values_at_node;
			field->computed_field_set_values_in_element_function =
				Computed_field_finite_element_set_values_in_element;
			field->computed_field_get_native_discretization_in_element_function =
				Computed_field_finite_element_get_native_discretization_in_element;
			field->computed_field_find_element_xi_function =
				Computed_field_finite_element_find_element_xi;
			field->list_Computed_field_function = 
				list_Computed_field_finite_element;
			field->list_Computed_field_commands_function = 
				list_Computed_field_finite_element_commands;
		}
		else
		{
			for (i = 0 ; i < number_of_components ; i++)
			{
				if (component_names[i])
				{
					DEALLOCATE(component_names[i]);
				}
			}
			DEALLOCATE(component_names);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_finite_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_finite_element */

int Computed_field_get_type_finite_element(struct Computed_field *field,
	struct FE_field **fe_field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_FINITE_ELEMENT, the FE_field being
"wrapped" by it is returned - otherwise an error is reported.
==============================================================================*/
{
	int return_code;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_get_type_finite_element);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_finite_element_type_string) 
		&& (data = (struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		*fe_field=data->fe_field;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_finite_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_finite_element */

static int define_Computed_field_type_finite_element(struct Parse_state *state,
	void *field_void,void *computed_field_finite_element_package_void)
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Creates FE_fields with the given name, coordinate_system, value_type,
cm_field_type, number_of_components and component_names.
The actual finite_element wrapper is not made here but in response to the
FE_field being made and/or modified.
==============================================================================*/
{
	char *cm_field_type_string,**component_names,*current_token,
		**temp_component_names,**valid_strings,*value_type_string;
	enum CM_field_type cm_field_type;
	enum Value_type value_type;
	int i,number_of_components,number_of_valid_strings,
		original_number_of_components,return_code;
	struct Computed_field *existing_field,*field;
	struct Computed_field_finite_element_package *computed_field_finite_element_package;
	struct Coordinate_system *coordinate_system;
	struct FE_field *existing_fe_field,*fe_field,*temp_fe_field;
	struct Option_table *option_table;

	ENTER(define_Computed_field_type_finite_element);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_finite_element_package=
		(struct Computed_field_finite_element_package *)
		computed_field_finite_element_package_void))
	{
		return_code=1;
		existing_fe_field=(struct FE_field *)NULL;
		if (computed_field_finite_element_type_string ==
				Computed_field_get_type_string(field))
		{
			return_code=
				Computed_field_get_type_finite_element(field,&existing_fe_field);
		}
		if (return_code)
		{
			if (existing_fe_field)
			{
				value_type=get_FE_field_value_type(existing_fe_field);
				if (existing_fe_field)
				{
					number_of_components=
						get_FE_field_number_of_components(existing_fe_field);
					cm_field_type=get_FE_field_CM_field_type(existing_fe_field);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_finite_element.  "
						"Missing existing_fe_field");
					return_code=0;
				}
			}
			else
			{
				/* default to real, 3-component coordinate field */
				value_type=FE_VALUE_VALUE;
				number_of_components=3;
				cm_field_type=CM_COORDINATE_FIELD;
			}
			if (return_code)
			{
				cm_field_type_string=CM_field_type_string(cm_field_type);
				value_type_string=Value_type_string(value_type);
			}
			if (ALLOCATE(component_names,char *,number_of_components))
			{
				for (i=0;i<number_of_components;i++)
				{
					component_names[i]=(char *)NULL;
					if (existing_fe_field)
					{
						component_names[i]=get_FE_field_component_name(existing_fe_field,i);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_finite_element.  Not enough memory");
				return_code=0;
			}
			original_number_of_components=number_of_components;
			/* try to handle help first */
			if (return_code&&(current_token=state->current_token))
			{
				if (!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
				{
					option_table=CREATE(Option_table)();
					/* cm_field_type */
					valid_strings=
						CM_field_type_get_valid_strings(&number_of_valid_strings);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&cm_field_type_string);
					DEALLOCATE(valid_strings);
					/* component_names */
					Option_table_add_entry(option_table,"component_names",component_names,
						&original_number_of_components,set_names);
					/* value_type */
					valid_strings=Value_type_get_valid_strings_simple(
						&number_of_valid_strings);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&value_type_string);
					DEALLOCATE(valid_strings);
					/* number_of_components */
					Option_table_add_entry(option_table,"number_of_components",
						&number_of_components,NULL,set_int_positive);
					return_code=Option_table_multi_parse(option_table,state);
					DESTROY(Option_table)(&option_table);
				}
			}
			/* parse the number_of_components */
			if (return_code&&(current_token=state->current_token))
			{
				/* ... only if the "number_of_components" token is next */
				if (fuzzy_string_compare(current_token,"number_of_components"))
				{
					option_table=CREATE(Option_table)();
					Option_table_add_entry(option_table,"number_of_components",
						&number_of_components,NULL,set_int_positive);
					if (return_code=Option_table_parse(option_table,state))
					{
						if (number_of_components != original_number_of_components)
						{
							/* deallocate any names that will no longer be with the field */
							for (i=number_of_components;i<original_number_of_components;i++)
							{
								if (component_names[i])
								{
									DEALLOCATE(component_names[i]);
								}
							}
							if (REALLOCATE(temp_component_names,component_names,char *,
								number_of_components))
							{
								component_names=temp_component_names;
								/* clear any new names */
								for (i=original_number_of_components;i<number_of_components;i++)
								{
									component_names[i]=(char *)NULL;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"define_Computed_field_type_finite_element.  "
									"Not enough memory");
								number_of_components=original_number_of_components;
								return_code=0;
							}
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			/* parse the remainder */
			if (return_code&&state->current_token)
			{
				option_table=CREATE(Option_table)();
				/* cm_field_type */
				valid_strings=
					CM_field_type_get_valid_strings(&number_of_valid_strings);
				Option_table_add_enumerator(option_table,number_of_valid_strings,
					valid_strings,&cm_field_type_string);
				DEALLOCATE(valid_strings);
				/* component_names */
				Option_table_add_entry(option_table,"component_names",component_names,
					&number_of_components,set_names);
				/* value_type */
				valid_strings=Value_type_get_valid_strings_simple(
					&number_of_valid_strings);
				Option_table_add_enumerator(option_table,number_of_valid_strings,
					valid_strings,&value_type_string);
				DEALLOCATE(valid_strings);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			if (return_code)
			{
				value_type=Value_type_from_string(value_type_string);
				cm_field_type=CM_field_type_from_string(cm_field_type_string);
				/* now make an FE_field to match the options entered */
				if (fe_field=CREATE(FE_field)())
				{
					/* get the name from the computed field */
					return_code=set_FE_field_name(fe_field,field->name);
					ACCESS(FE_field)(fe_field);
					if (return_code&&existing_fe_field)
					{
						/* copy existing field to get as much in common with it as
							 possible */
						return_code=MANAGER_COPY_WITHOUT_IDENTIFIER(FE_field,name)(fe_field,
							existing_fe_field);
					}
					if (return_code&&set_FE_field_value_type(fe_field,value_type)&&
						set_FE_field_number_of_components(fe_field,number_of_components)&&
						set_FE_field_CM_field_type(fe_field,cm_field_type)&&
						(coordinate_system=Computed_field_get_coordinate_system(field))&&
						set_FE_field_coordinate_system(fe_field,coordinate_system))
					{
						for (i=0;i<number_of_components;i++)
						{
							if (component_names[i])
							{
								set_FE_field_component_name(fe_field,i,component_names[i]);
							}
						}
						if (existing_fe_field)
						{
							if ((existing_field=
								FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
									field->name,computed_field_finite_element_package->
									computed_field_manager))&&
								(!Computed_field_is_in_use(existing_field)))
							{
								/* since cannot modify contents of FE_field while it is in use,
									 deaccess temp_copy of it for duration of modify. Actually,
									 it is accessed twice, once by the wrapper field in the
									 manager, and once by "field", hence: */
								temp_fe_field=existing_fe_field;
								DEACCESS(FE_field)(&temp_fe_field);
								temp_fe_field=existing_fe_field;
								DEACCESS(FE_field)(&temp_fe_field);
								return_code=MANAGER_MODIFY_NOT_IDENTIFIER(FE_field,name)(
									existing_fe_field,fe_field,
									computed_field_finite_element_package->fe_field_manager);
								ACCESS(FE_field)(existing_fe_field);
								ACCESS(FE_field)(existing_fe_field);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Cannot redefine finite_element field while it is in use");
								display_parse_state_location(state);
								return_code=0;
							}
						}
						else
						{
							return_code=ADD_OBJECT_TO_MANAGER(FE_field)(fe_field,
								computed_field_finite_element_package->fe_field_manager);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_finite_element.  "
							"Could not set up fe_field");
						return_code=0;
					}
					if (return_code)
					{
						/* must make sure the computed field is set to type
							 FINITE_ELEMENT for correct handling in define_Computed_field */
						if (return_code=Computed_field_set_type_finite_element(field,fe_field))
						{
							Computed_field_set_coordinate_system(field,
								get_FE_field_coordinate_system(fe_field));
						}
					}
					DEACCESS(FE_field)(&fe_field);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_finite_element.  "
						"Could not create fe_field");
					return_code=0;
				}
			}
			/* clean up the component_names array */
			if (component_names)
			{
				for (i=0;i<number_of_components;i++)
				{
					if (component_names[i])
					{
						DEALLOCATE(component_names[i]);
					}
				}
			}
		}
		/* Always return 0 as this ensures the define_Computed_field function
			does not modify the Computed_field, the Computed_field is instead
			modified in response to the FE_field manager messages */
		return_code=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_finite_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_finite_element */

struct Computed_field_default_coordinates_type_specific_data
{
	struct MANAGER(Computed_field) *computed_field_manager;
};

static char computed_field_default_coordinate_type_string[] = "default_coordinate";

int Computed_field_is_type_default_coordinate(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_default_coordinate);
	if (field)
	{
		return_code = (field->type_string == computed_field_default_coordinate_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_default_coordinate.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_type_default_coordinate */

static int Computed_field_default_coordinate_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_default_coordinate_clear_type_specific);
	if (field)
	{
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_coordinate_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_coordinate_clear_type_specific */

static void *Computed_field_default_coordinate_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_default_coordinates_type_specific_data *destination,
		*source;

	ENTER(Computed_field_default_coordinate_copy_type_specific);
	if (field && (source = 
		(struct Computed_field_default_coordinates_type_specific_data *)
		field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_default_coordinates_type_specific_data, 1))
		{
			destination->computed_field_manager = source->computed_field_manager;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_default_coordinate_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_coordinate_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_default_coordinate_copy_type_specific */

int Computed_field_default_coordinate_clear_cache_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_clear_cache);
	if (field)
	{
		if (field->source_fields)
		{
			/* must deaccess any source_fields, since these act as a cache for type
				 COMPUTED_FIELD_DEFAULT_COORDINATE */
			for (i=0;i< field->number_of_source_fields;i++)
			{
				DEACCESS(Computed_field)(&(field->source_fields[i]));
			}
			DEALLOCATE(field->source_fields);
			field->number_of_source_fields=0;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clear_cache.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_clear_cache */

static int Computed_field_default_coordinate_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_default_coordinate_type_specific_contents_match);
	if (field && other_computed_field)
	{
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_coordinate_type_specific_contents_match */

int Computed_field_default_coordinate_is_defined_in_element(
	struct Computed_field *field, struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/
{
	int return_code;
	ENTER(Computed_field_default_coordinate_is_defined_in_element);
	if (field && element)
	{
		if (get_FE_element_default_coordinate_field(element))
		{
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
			"Computed_field_default_coordinate_is_defined_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_coordinate_is_defined_in_element */


static int Computed_field_default_coordinate_is_defined_at_node(
	struct Computed_field *field, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_default_coordinate_evaluate_cache_at_node);
	if (field && node)
	{
		if (get_FE_node_default_coordinate_field(node))
		{
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
			"Computed_field_default_coordinate_is_defined_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_coordinate_evaluate_cache_at_node */

#define Computed_field_default_coordinate_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_default_coordinate_can_be_destroyed \
	(Computed_field_can_be_destroyed_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
No special criteria on the destroy
==============================================================================*/

static int Computed_field_get_default_coordinate_source_field_in_element(
	struct Computed_field *field,struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 27 October 1999

DESCRIPTION :
For fields of type COMPUTED_FIELD_DEFAULT_COORDINATE, makes sure the <field>'s
source_fields are allocated to contain 1 field, the finite_element wrapper for
the first coordinate field defined at <node>. For efficiency, checks that the
currently cached field/node are not already correct before finding a new one.
==============================================================================*/
{
	int new_field, return_code;
	struct FE_field *fe_field;
	struct Computed_field_default_coordinates_type_specific_data *data;
	struct Computed_field_finite_element_type_specific_data *fe_data;

	ENTER(Computed_field_get_default_coordinate_source_field_in_element);
	if (field&&element&&(field->type_string==computed_field_default_coordinate_type_string)
		&& (data = (struct Computed_field_default_coordinates_type_specific_data *)
		field->type_specific_data))
	{
		return_code=1;
		/* get Computed_field wrapping first coordinate field of element */
		/* if the element is still pointed to by the cache, then already ok */
		if (element != field->element)
		{
			if (fe_field=get_FE_element_default_coordinate_field(element))
			{
				new_field = 1;
				if (field->source_fields)
				{
					if (Computed_field_is_type_finite_element(field->source_fields[0]))
					{
						fe_data = (struct Computed_field_finite_element_type_specific_data *)
							field->source_fields[0]->type_specific_data;
						if (fe_field == fe_data->fe_field)
						{
							new_field = 0;
						}
					}
				}
				if (new_field)
				{
					/* finding a new field, so must clear cache of current one */
					if (field->source_fields)
					{
						Computed_field_clear_cache(field->source_fields[0]);
						DEACCESS(Computed_field)(&(field->source_fields[0]));
					}
					else
					{
						field->source_fields=
							ALLOCATE(field->source_fields,struct Computed_field *,1);
					}
					if (field->source_fields)
					{
						if (field->source_fields[0]=
							FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
								Computed_field_is_read_only_with_fe_field,
								(void *)fe_field,data->computed_field_manager))
						{
							ACCESS(Computed_field)(field->source_fields[0]);
							field->number_of_source_fields=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_get_default_coordinate_source_field_in_element."
								"  No computed field for default coordinate field");
							/* don't want empty source_fields array left around */
							DEALLOCATE(field->source_fields);
							field->number_of_source_fields=0;
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_get_default_coordinate_source_field_in_element.  "
							"Could not allocate default coordinate source fields");
						field->number_of_source_fields=0;
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_get_default_coordinate_source_field_in_element.  "
					"No default coordinate field");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_default_coordinate_source_field_in_element.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_default_coordinate_source_field_in_element */

static int Computed_field_get_default_coordinate_source_field_at_node(
	struct Computed_field *field,struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
For fields of type COMPUTED_FIELD_DEFAULT_COORDINATE, makes sure the <field>'s
source_fields are allocated to contain 1 field, the finite_element wrapper for
the first coordinate field defined at <node>. For efficiency, checks that the
currently cached field/node are not already correct before finding a new one.
==============================================================================*/
{
	int new_field, return_code;
	struct FE_field *fe_field;
	struct Computed_field_default_coordinates_type_specific_data *data;
	struct Computed_field_finite_element_type_specific_data *fe_data;

	ENTER(Computed_field_get_default_coordinate_source_field_at_node);
	if (field&&node&&(field->type_string==computed_field_default_coordinate_type_string)
		&& (data = (struct Computed_field_default_coordinates_type_specific_data *)
		field->type_specific_data))
	{
		return_code=1;
		/* if node and field->node have equivalent fields then source field will
			 already be correct */
		if (!(field->node&&equivalent_FE_fields_at_nodes(node,field->node)))
		{
			if (fe_field=get_FE_node_default_coordinate_field(node))
			{
				new_field = 1;
				if (field->source_fields)
				{
					if (Computed_field_is_type_finite_element(field->source_fields[0]))
					{
						fe_data = (struct Computed_field_finite_element_type_specific_data *)
							field->source_fields[0]->type_specific_data;
						if (fe_field == fe_data->fe_field)
						{
							new_field = 0;
						}
					}
				}
				if (new_field)
				{
					/* finding a new field, so must clear cache of current one */
					if (field->source_fields)
					{
						Computed_field_clear_cache(field->source_fields[0]);
						DEACCESS(Computed_field)(&(field->source_fields[0]));
					}
					else
					{
						field->source_fields=
							ALLOCATE(field->source_fields,struct Computed_field *,1);
					}
					if (field->source_fields)
					{
						if (field->source_fields[0]=
							FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
								Computed_field_is_read_only_with_fe_field,
								(void *)fe_field,data->computed_field_manager))
						{
							ACCESS(Computed_field)(field->source_fields[0]);
							field->number_of_source_fields=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_get_default_coordinate_source_field_at_node.  "
								"No computed field for default coordinate field");
							/* don't want empty source_fields array left around */
							DEALLOCATE(field->source_fields);
							field->number_of_source_fields=0;
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_get_default_coordinate_source_field_at_node.  "
							"Could not allocate default coordinate source fields");
						field->number_of_source_fields=0;
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_get_default_coordinate_source_field_at_node.  "
					"No default coordinate field");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_default_coordinate_source_field_at_node.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_default_coordinate_source_field_at_node */

int Computed_field_default_coordinate_evaluate_cache_at_node(
	struct Computed_field *field,struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_default_coordinate_evaluate_cache_at_node);
	if (field&&node)
	{
		return_code=1;
		if (Computed_field_get_default_coordinate_source_field_at_node(
			field,node))
		{
			/* calculate values of source_field */
			if (return_code=Computed_field_evaluate_source_fields_cache_at_node(
				field,node))
			{
				/* once the default_coordinate source field is found, its
					calculation is the same as rc_coordinate */
				return_code=Computed_field_evaluate_rc_coordinate(field,
					/*element_dimension*/0,/*calculate_derivatives*/0);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_default_coordinate_evaluate_cache_at_node.  "
				"Could not get default coordinate source_field");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_coordinate_evaluate_cache_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_coordinate_evaluate_cache_at_node */

static int Computed_field_default_coordinate_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	struct FE_element *top_level_element, int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int element_dimension, return_code;

	ENTER(Computed_field_default_coordinate_evaluate_cache_in_element);
	if (field && Computed_field_has_at_least_2_components(field, NULL) && 
		element && xi)
	{
		element_dimension=element->shape->dimension;
		if (Computed_field_get_default_coordinate_source_field_in_element(
			field,element))
		{
			/* calculate values of source_field */
			if(return_code=Computed_field_evaluate_source_fields_cache_in_element(
				field,element,xi,top_level_element,
				calculate_derivatives))
			{
				/* once the default_coordinate source field is found, its
					calculation is the same as rc_coordinate */
				return_code=Computed_field_evaluate_rc_coordinate(field,
					element_dimension,calculate_derivatives);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_cache_in_element.  "
				"Could not get default coordinate source_field");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_coordinate_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_coordinate_evaluate_cache_in_element */

#define Computed_field_default_coordinate_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_default_coordinate_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

int Computed_field_default_coordinate_set_values_at_node(struct Computed_field *field,
	struct FE_node *node,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	FE_value non_rc_coordinates[3];
	int return_code;
	struct Coordinate_system rc_coordinate_system;
					
	ENTER(Computed_field_set_values_at_node);
	if (field&&node&&values)
	{
		if (Computed_field_get_default_coordinate_source_field_at_node(
			field,node))
		{
			/* convert RC values back into source coordinate system */
			rc_coordinate_system.type = RECTANGULAR_CARTESIAN;
			return_code=
				convert_Coordinate_system(&rc_coordinate_system,values,
					&(field->source_fields[0]->coordinate_system),non_rc_coordinates,
					/*jacobian*/(float *)NULL)&&
				Computed_field_set_values_at_node(field->source_fields[0],
					node,non_rc_coordinates);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_default_coordinate_set_values_at_node.  "
				"Could not get default coordinate source_field");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_coordinate_set_values_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_coordinate_set_values_at_node */

int Computed_field_default_coordinate_set_values_in_element(struct Computed_field *field,
	struct FE_element *element,int *number_in_xi,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	FE_value non_rc_coordinates[3],rc_coordinates[3],*source_values;
	int element_dimension,i,j,k,number_of_points,return_code;
	struct Coordinate_system rc_coordinate_system;

	ENTER(Computed_field_set_values_in_element);
	if (field&&element&&element->shape&&number_in_xi&&values)
	{
		return_code=1;
		element_dimension=element->shape->dimension;
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
					"Computed_field_scale_set_values_in_element.  "
					"number_in_xi must be positive");
				return_code=0;
			}
		}
		if (return_code)
		{
			if (Computed_field_get_default_coordinate_source_field_in_element(
				field,element))
			{
				rc_coordinate_system.type = RECTANGULAR_CARTESIAN;
				/* 3 values for non-rc coordinate system */
				if (ALLOCATE(source_values,FE_value,number_of_points*3))
				{
					for (j=0;(j<number_of_points)&&return_code;j++)
					{
						for (k=0;k<3;k++)
						{
							rc_coordinates[k]=values[k*number_of_points+j];
						}
						/* convert RC values back into source coordinate system */
						if (convert_Coordinate_system(&rc_coordinate_system,
							rc_coordinates,&(field->source_fields[0]->coordinate_system),
							non_rc_coordinates,/*jacobian*/(float *)NULL))
						{
							for (k=0;k<3;k++)
							{
								source_values[k*number_of_points+j]=non_rc_coordinates[k];
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
							field->source_fields[0],element,number_in_xi,source_values);
					}
					DEALLOCATE(source_values);
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_default_coordinate_set_values_in_element.  "
					"Could not get default coordinate source_field");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_coordinate_set_values_in_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_coordinate_set_values_in_element */

int Computed_field_default_coordinate_get_native_discretization_in_element(
	struct Computed_field *field,struct FE_element *element,int *number_in_xi)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_default_coordinate_get_native_discretization_in_element);
	if (field&&element&&number_in_xi&&element->shape&&
		(MAXIMUM_ELEMENT_XI_DIMENSIONS>=element->shape->dimension))
	{
		if (Computed_field_get_default_coordinate_source_field_in_element(
			field,element))
		{
			return_code=
				Computed_field_get_native_discretization_in_element(
					field->source_fields[0],element,number_in_xi);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_default_coordinate_get_native_discretization_in_element.  "
				"Could not get default coordinate source_field");
			return_code=0;
		}
		/* must clear the cache to remove default coordinate source field */
		Computed_field_clear_cache(field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_coordinate_get_native_discretization_in_element.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_coordinate_get_native_discretization_in_element */

#define Computed_field_default_coordinate_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_default_coordinate(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_default_coordinate);
	if (field)
	{
		/* no extra parameters */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_default_coordinate.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_default_coordinate */

static int list_Computed_field_default_coordinate_commands(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(list_Computed_field_default_coordinate_commands);
	if (field)
	{
		/* no extra parameters */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_default_coordinate_commands.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_default_coordinate_commands */

int Computed_field_set_type_default_coordinate(struct Computed_field *field,
	struct MANAGER(Computed_field) *computed_field_manager)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DEFAULT_COORDINATE, which returns the
values/derivatives of the first [coordinate] field defined for the element/node
in rectangular cartesian coordinates. This type is intended to replace the
NULL coordinate_field option in the calculate_FE_element_field_values function.
When a field of this type is calculated at and element/node, the evaluate
function finds the first FE_field (coordinate type) defined over it, then gets
its Computed_field wrapper from the manager and proceeds from there.
Consequences of this behaviour are:
- the field allocates its source_fields to point to the computed_field for the
actual coordinate field in the evaluate phase.
- when the source field changes the current one's cache is cleared and it is
deaccessed.
- when the cache is cleared, so is any reference to the source_field.
- always performs the conversion to RC since cannot predict the coordinate
system used by the eventual source_field. Coordinate_system of this type of
field need not be RC, although it usually will be.
Sets number of components to 3.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int return_code;
	struct Computed_field_default_coordinates_type_specific_data *data;

	ENTER(Computed_field_set_type_default_coordinate);
	if (field&&computed_field_manager)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,struct Computed_field_default_coordinates_type_specific_data, 1))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = computed_field_default_coordinate_type_string;
			field->number_of_components = 3;
			field->type_specific_data = (void *)data;
			data->computed_field_manager = computed_field_manager;

			/* Set all the methods */
			field->computed_field_clear_type_specific_function =
				Computed_field_default_coordinate_clear_type_specific;
			field->computed_field_copy_type_specific_function =
				Computed_field_default_coordinate_copy_type_specific;
			field->computed_field_clear_cache_type_specific_function =
				Computed_field_default_coordinate_clear_cache_type_specific;
			field->computed_field_type_specific_contents_match_function =
				Computed_field_default_coordinate_type_specific_contents_match;
			field->computed_field_is_defined_in_element_function =
				Computed_field_default_coordinate_is_defined_in_element;
			field->computed_field_is_defined_at_node_function =
				Computed_field_default_coordinate_is_defined_at_node;
			field->computed_field_has_numerical_components_function =
				Computed_field_default_coordinate_has_numerical_components;
			field->computed_field_can_be_destroyed_function =
				Computed_field_default_coordinate_can_be_destroyed;
			field->computed_field_evaluate_cache_at_node_function =
				Computed_field_default_coordinate_evaluate_cache_at_node;
			field->computed_field_evaluate_cache_in_element_function =
				Computed_field_default_coordinate_evaluate_cache_in_element;
			field->computed_field_evaluate_as_string_at_node_function =
				Computed_field_default_coordinate_evaluate_as_string_at_node;
			field->computed_field_evaluate_as_string_in_element_function =
				Computed_field_default_coordinate_evaluate_as_string_in_element;
			field->computed_field_set_values_at_node_function =
				Computed_field_default_coordinate_set_values_at_node;
			field->computed_field_set_values_in_element_function =
				Computed_field_default_coordinate_set_values_in_element;
			field->computed_field_get_native_discretization_in_element_function =
				Computed_field_default_coordinate_get_native_discretization_in_element;
			field->computed_field_find_element_xi_function =
				Computed_field_default_coordinate_find_element_xi;
			field->list_Computed_field_function = 
				list_Computed_field_default_coordinate;
			field->list_Computed_field_commands_function = 
				list_Computed_field_default_coordinate_commands;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_default_coordinate.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_default_coordinate */

int Computed_field_get_type_default_coordinate(struct Computed_field *field,
	struct MANAGER(Computed_field) **computed_field_manager)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_DEFAULT_COORDINATE, the 
<source_field> and <xi_index> used by it are returned.
==============================================================================*/
{
	int return_code;
	struct Computed_field_default_coordinates_type_specific_data *data;

	ENTER(Computed_field_get_type_default_coordinate);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_default_coordinate_type_string)
		&&(data = 
		(struct Computed_field_default_coordinates_type_specific_data *)
		field->type_specific_data)&&computed_field_manager)
	{
		*computed_field_manager=data->computed_field_manager;	
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_default_coordinate.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_default_coordinate */

static int define_Computed_field_type_default_coordinate(
	struct Parse_state *state,void *field_void,
	void *computed_field_finite_element_package_void)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_DEFAULT_COORDINATE.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field;
	struct Computed_field_finite_element_package 
		*computed_field_finite_element_package;

	ENTER(define_Computed_field_type_default_coordinate);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_finite_element_package=(struct 
			Computed_field_finite_element_package *)
			computed_field_finite_element_package_void))
	{
		if (!state->current_token)
		{
			return_code=Computed_field_set_type_default_coordinate(field,
				computed_field_finite_element_package->computed_field_manager);
		}
		else
		{
			if (strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
			{
				display_message(ERROR_MESSAGE,
					"Unknown option <%s>",state->current_token);
				display_parse_state_location(state);
			}
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_default_coordinate.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_default_coordinate */

int Computed_field_is_read_only_with_fe_field(
	struct Computed_field *field,void *fe_field_void)
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Iterator/conditional function returning true if <field> is read only and a
wrapper for <fe_field>.
==============================================================================*/
{
	int return_code;
	struct FE_field *fe_field;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_is_read_only_with_fe_field);
	if (field&&(fe_field=(struct FE_field *)fe_field_void))
	{
		return_code = 0;
		if (field->read_only)
		{
			if (field->type_string == computed_field_finite_element_type_string)
			{
				data = (struct Computed_field_finite_element_type_specific_data *)
					field->type_specific_data;
				return_code=(data->fe_field==fe_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_read_only_with_fe_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_read_only_with_fe_field */

int Computed_field_is_scalar_integer(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Returns true if <field> is a 1 integer component FINITE_ELEMENT wrapper.
==============================================================================*/
{
	int return_code;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_is_scalar_integer);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		if((1==field->number_of_components)&&
			(COMPUTED_FIELD_NEW_TYPES==field->type)&&
			(field->type_string==computed_field_finite_element_type_string) 
			&& (data = (struct Computed_field_finite_element_type_specific_data *)
			field->type_specific_data))
		{
			return_code = (INT_VALUE==get_FE_field_value_type(data->fe_field));
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_scalar_integer.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_scalar_integer */

int Computed_field_is_scalar_integer_grid_in_element(
	struct Computed_field *field,void *element_void)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Returns true if <field> is a 1 integer component FINITE_ELEMENT wrapper which
is defined in <element> AND is grid-based.
Used for choosing field suitable for identifying grid points.
==============================================================================*/
{
	int return_code;
	struct FE_element *element;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_is_scalar_integer_grid_in_element);
	if (field&&(element=(struct FE_element *)element_void))
	{
		if ((1==field->number_of_components)&&
			(COMPUTED_FIELD_NEW_TYPES==field->type)&&
			(field->type_string==computed_field_finite_element_type_string) 
			&& (data = (struct Computed_field_finite_element_type_specific_data *)
			field->type_specific_data))
		{
			return_code = (INT_VALUE==get_FE_field_value_type(data->fe_field))&&
				Computed_field_is_defined_in_element(field,element)&&
				FE_element_field_is_grid_based(element,data->fe_field);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_scalar_integer_grid_in_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_scalar_integer_grid_in_element */

static int Computed_field_update_fe_field_in_manager(
	struct Computed_field *computed_field, struct FE_field *fe_field,
	struct MANAGER(Computed_field) *computed_field_manager)
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Searches the Computed_field_manager for a Computed_field which has the exact
same contents.  If there is then the object is "MODIFIED" so that the name is
updated and MANAGER messages are passed to any clients. (i.e. the fe_field may
have changed).  If there isn't a matching field then the new one is added to
the manager.
==============================================================================*/
{
	int return_code;
	struct Computed_field *matching_field,*same_name_field;

	ENTER(Computed_field_update_fe_field_in_manager);
	if (computed_field&&fe_field&&computed_field_manager)
	{
		/* find any existing wrapper for fe_field */
		same_name_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
			computed_field->name,computed_field_manager);
		if (((matching_field=same_name_field)&&
			(!Computed_field_is_in_use(matching_field)))||
			(matching_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_contents_match,(void *)computed_field,
				computed_field_manager)))
		{
			/* has the field name changed? */
			if (strcmp(matching_field->name,computed_field->name))
			{
				return_code=MANAGER_MODIFY(Computed_field)(
					matching_field,computed_field,computed_field_manager);
			}
			else
			{
				return_code=MANAGER_MODIFY_NOT_IDENTIFIER(Computed_field,name)(
					matching_field,computed_field,computed_field_manager);
			}
		}
		else
		{
			if (same_name_field)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_update_fe_field_in_manager.  "
					"Cannot modify computed field wrapper %s",same_name_field->name);
				return_code=0;
			}
			else
			{
				return_code=ADD_OBJECT_TO_MANAGER(Computed_field)(
					computed_field,computed_field_manager);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_update_fe_field_in_manager.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_update_fe_field_in_manager */

static int FE_field_update_wrapper_Computed_field(struct FE_field *fe_field,
	void *computed_field_manager_void)
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
FE_field iterator function that performs the following:
1. Tries to find a wrapper Computed_field for <fe_field> in the
<computed_field_manager>.
2. If one is found, the contents are modified to match the changed values in
the fe_field, and if the name of the fields are the same, only a
MANAGER_MODIFY_NOT_IDENTIFIER is performed.
3. If no wrapper is found, a new one is created and added to the manager.
==============================================================================*/
{
	char *extra_field_name, *field_name;
	enum Value_type value_type;
	int return_code;
	struct Computed_field *default_coordinate_field,*temp_field;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(FE_field_update_wrapper_Computed_field);
	if (fe_field&&(computed_field_manager=
		(struct MANAGER(Computed_field) *)computed_field_manager_void))
	{
		if (GET_NAME(FE_field)(fe_field,&field_name))
		{
			/* establish up-to-date wrapper for the fe_field */
			if (temp_field=CREATE(Computed_field)(field_name))
			{
				ACCESS(Computed_field)(temp_field);
				if (Computed_field_set_coordinate_system(temp_field,
					get_FE_field_coordinate_system(fe_field))&&
					Computed_field_set_type_finite_element(temp_field,fe_field)&&
					Computed_field_set_read_only(temp_field))
				{
					return_code = Computed_field_update_fe_field_in_manager(temp_field,
						fe_field, computed_field_manager);
				}
				else
				{
					return_code=0;
				}
				DEACCESS(Computed_field)(&temp_field);
			}
			else
			{
				return_code=0;
			}

			/* For the ELEMENT_XI_VALUE also make a default embedded coordinate field */
			value_type = get_FE_field_value_type(fe_field);
			switch(value_type)
			{
				case ELEMENT_XI_VALUE:
				{
					if(ALLOCATE(extra_field_name, char, strlen(field_name) + 20)&&
						sprintf(extra_field_name, "%s_coordinate", field_name))
					{
						/* establish up-to-date wrapper for the fe_field */
						if (temp_field=CREATE(Computed_field)(extra_field_name))
						{
							ACCESS(Computed_field)(temp_field);
							if ((default_coordinate_field=
								FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
									"default_coordinate",computed_field_manager))||
								(default_coordinate_field=
									FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
										Computed_field_has_up_to_3_numerical_components,
										(void *)NULL,computed_field_manager)))
							{
								if (Computed_field_set_coordinate_system(temp_field,
									Computed_field_get_coordinate_system(
										default_coordinate_field))&&
									Computed_field_set_type_embedded(temp_field,fe_field,
							  		default_coordinate_field)&&
									Computed_field_set_read_only(temp_field))
								{
									return_code = Computed_field_update_fe_field_in_manager(
										temp_field, fe_field, computed_field_manager);
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
							DEACCESS(Computed_field)(&temp_field);
						}
						else
						{
							return_code=0;
						}
						DEALLOCATE(extra_field_name);
					}
					else
					{
						return_code=0;
					}
				} break;
			}
			DEALLOCATE(field_name);
		}
		else
		{
			return_code=0;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"FE_field_update_wrapper_Computed_field.  Unable to update wrapper");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_update_wrapper_Computed_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_update_wrapper_Computed_field */

static void Computed_field_FE_field_change(
	struct MANAGER_MESSAGE(FE_field) *message,void *computed_field_finite_element_package_void)
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Something has changed globally in the FE_field manager. This function ensures
that there is an appropriate (same name/#components/coordinate system)
Computed_field wrapper for each FE_field in the manager.
==============================================================================*/
{
	char *field_name;
	int return_code;
	struct Computed_field *field;
	struct Computed_field_finite_element_package *computed_field_finite_element_package;
	struct FE_field *fe_field;

	ENTER(Computed_field_FE_field_change);
	if (message&&(computed_field_finite_element_package=
		(struct Computed_field_finite_element_package *)computed_field_finite_element_package_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(FE_field):
			{
				/* establish/update all FE_field wrappers */
				/*???RC Note: does not handle deletion of fe_fields during manager
					change all. Should not happen really */
				MANAGER_BEGIN_CACHE(Computed_field)(
					computed_field_finite_element_package->computed_field_manager);
				return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_field)(
					FE_field_update_wrapper_Computed_field,
					(void *)computed_field_finite_element_package->computed_field_manager,
					computed_field_finite_element_package->fe_field_manager);
				MANAGER_END_CACHE(Computed_field)(
					computed_field_finite_element_package->computed_field_manager);
			} break;
			case MANAGER_CHANGE_ADD(FE_field):
			case MANAGER_CHANGE_OBJECT(FE_field):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(FE_field):
			{
				return_code=
					FE_field_update_wrapper_Computed_field(message->object_changed,
						(void *)computed_field_finite_element_package->computed_field_manager);
			} break;
			case MANAGER_CHANGE_DELETE(FE_field):
			{
				/* do nothing; this can never happen as must destroy Computed field before FE_field */
				return_code=1;
#if defined (OLD_CODE)
				fe_field=message->object_changed;
				if ((field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_is_read_only_with_fe_field,(void *)fe_field,
					computed_field_finite_element_package->computed_field_manager))&&
					GET_NAME(FE_field)(fe_field,&field_name))
				{
					if (Computed_field_is_in_use(field))
					{
						display_message(ERROR_MESSAGE,"Computed_field_FE_field_change."
							"  FE_field %s destroyed while Computed_field %s is in use",
							field_name,field->name);
						return_code=0;
					}
					else
					{
						return_code=REMOVE_OBJECT_FROM_MANAGER(Computed_field)(field,
							computed_field_finite_element_package->computed_field_manager);
					}
					DEALLOCATE(field_name);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_FE_field_change.  DELETE: Invalid field(s)");
					return_code=0;
				}
#endif /* defined (OLD_CODE) */
			} break;
			case MANAGER_CHANGE_IDENTIFIER(FE_field):
			{
				fe_field=message->object_changed;
				if ((field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_is_read_only_with_fe_field,(void *)fe_field,
					computed_field_finite_element_package->computed_field_manager))&&
					GET_NAME(FE_field)(fe_field,&field_name))
				{
					return_code=MANAGER_MODIFY_IDENTIFIER(Computed_field,name)(
						field,field_name,computed_field_finite_element_package->computed_field_manager);
					DEALLOCATE(field_name);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_FE_field_change.  IDENTIFIER: Invalid field(s)");
					return_code=0;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_FE_field_change.  Unknown manager message");
				return_code=0;
			} break;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_FE_field_change.  Unable to process changes");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_FE_field_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Computed_field_FE_field_change */

int destroy_computed_field_given_fe_field(
	struct MANAGER(Computed_field) *computed_field_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct FE_field *fe_field)
/*******************************************************************************
LAST MODIFIED : 17 May 2000

DESCRIPTION :
Given <fe_field>, destroys the associated computed field, and fe_field
==============================================================================*/
{
	int return_code;
	struct Computed_field *computed_field=(struct Computed_field *)NULL;

	ENTER(destroy_computed_field_given_fe_field);
	if(computed_field_manager&&fe_field_manager&&fe_field)
	{
		if(computed_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)
			(Computed_field_is_read_only_with_fe_field,(void *)(fe_field),
				computed_field_manager))
		{
			if (Computed_field_can_be_destroyed(computed_field))
			{
				/* also want to destroy (wrapped) FE_field */	
				if (return_code=REMOVE_OBJECT_FROM_MANAGER(Computed_field)(
					computed_field,computed_field_manager))
				{							
					return_code=REMOVE_OBJECT_FROM_MANAGER(FE_field)(
						fe_field,fe_field_manager);
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,"destroy_computed_field_given_fe_field. "
						" Could not destroy field");
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"destroy_computed_field_given_fe_field Cannot destroy field in use ");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"destroy_computed_field_given_fe_field."
				"Field does not exist");	
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"destroy_computed_field_given_fe_field "
			"invalid arguements ");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* destroy_computed_field_given_fe_field */

int remove_computed_field_from_manager_given_FE_field(
	struct MANAGER(Computed_field) *computed_field_manager,struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : August 27 1999

DESCRIPTION :
Frees the computed fields from the computed field manager, given the FE_field
==============================================================================*/
{
	int return_code;
	struct Computed_field *computed_field;

	ENTER(remove_computed_field_from_manager_given_FE_field);
	if(field&&computed_field_manager)
	{
		return_code=1;
		computed_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
			Computed_field_is_read_only_with_fe_field,
			(void *)(field),
			computed_field_manager);
		if(computed_field)
		{
			if (Computed_field_can_be_destroyed(computed_field))
			{
				REMOVE_OBJECT_FROM_MANAGER(Computed_field)(computed_field,
					computed_field_manager);			
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"remove_computed_field_from_manager_given_FE_field."
			" invalid arguments");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* remove_computed_field_from_manager_given_FE_field */

struct Computed_field_finite_element_package *
Computed_field_register_types_finite_element(
	struct Computed_field_package *computed_field_package,
	struct MANAGER(FE_field) *fe_field_manager)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
This function registers the finite_element related types of Computed_fields and
also registers with the <fe_field_manager> so that any fe_fields are
automatically wrapped in corresponding computed_fields.
==============================================================================*/
{
	struct Computed_field_finite_element_package *return_ptr;
	static struct Computed_field_finite_element_package 
		computed_field_finite_element_package;

	ENTER(Computed_field_register_types_finite_element);
	if (computed_field_package&&fe_field_manager)
	{
		computed_field_finite_element_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_finite_element_package.fe_field_manager =
			fe_field_manager;
		Computed_field_package_add_type(computed_field_package,
			computed_field_finite_element_type_string,
			define_Computed_field_type_finite_element,
			&computed_field_finite_element_package);
		Computed_field_package_add_type(computed_field_package,
			computed_field_default_coordinate_type_string,
			define_Computed_field_type_default_coordinate,
			&computed_field_finite_element_package);
		if (computed_field_finite_element_package.fe_field_manager_callback_id=
			MANAGER_REGISTER(FE_field)(Computed_field_FE_field_change,
				(void *)&computed_field_finite_element_package,fe_field_manager))
		{
			return_ptr = &computed_field_finite_element_package;
		}
		else
		{
			return_ptr = (struct Computed_field_finite_element_package *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_finite_element.  Invalid argument(s)");
		return_ptr = (struct Computed_field_finite_element_package *)NULL;
	}
	LEAVE;

	return (return_ptr);
} /* Computed_field_register_types_finite_element */

int Computed_field_deregister_types_finite_element(
	struct Computed_field_finite_element_package
	*computed_field_finite_element_package)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_deregister_types_finite_element);
	if (computed_field_finite_element_package)
	{
		MANAGER_DEREGISTER(FE_field)(
			computed_field_finite_element_package->fe_field_manager_callback_id,
			computed_field_finite_element_package->fe_field_manager);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_deregister_types_finite_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_deregister_types_finite_element */

