/*******************************************************************************
FILE : computed_field_finite_element.c

LAST MODIFIED : 28 October 2004

DESCRIPTION :
Implements a number of basic component wise operations on computed fields.
==============================================================================*/
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_time.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_finite_element.h"

#if defined (DEBUG)
/* SAB This field is useful for debugging when things don't clean up properly
	but has to be used carefully, especially as operations such as caching
	accesses the node or element being considered so you get effects like 
	the first point evaluated in an element having a count one less than 
	all the others */
#define COMPUTED_FIELD_ACCESS_COUNT
#endif /* defined (DEBUG) */

/*
Global types
------------
*/
struct Computed_field_finite_element_package
/*******************************************************************************
LAST MODIFIED : 21 March 2003

DESCRIPTION :
==============================================================================*/
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Cmiss_region *cmiss_region;
	struct FE_region *fe_region;
}; /* struct Computed_field_finite_element_package */

/*
Module types
------------
*/
struct Computed_field_finite_element_type_specific_data
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
==============================================================================*/
{
	struct FE_field *fe_field;
	struct FE_element_field_values *fe_element_field_values;
	/* need pointer to fe_field_manager so can call MANAGED_OBJECT_NOT_IN_USE in
		 Computed_field_finite_element_not_in_use */
	struct FE_region *fe_region;
}; /* struct Computed_field_finite_element_type_specific_data */

/*******************************************************************************
COMPUTED_FIELD_TYPE: node_array_value_at_time

LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns the values for the given <nodal_value_type> and <version_number>
of <fe_field> at a node.
Makes the number of components the same as in the <fe_field>.
==============================================================================*/

struct Computed_field_node_value_type_specific_data
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	struct FE_field *fe_field;
	enum FE_nodal_value_type nodal_value_type;
	int version_number;
}; /* struct Computed_field_node_value_type_specific_data */

/*******************************************************************************
COMPUTED_FIELD_TYPE: node_array_value_at_time

LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns the values for the given <nodal_value_type> and <version_number> 
of <fe_field> at a node.
Makes the number of components the same as in the <fe_field>.
==============================================================================*/

struct Computed_field_node_array_value_at_time_type_specific_data
{
	struct FE_field *fe_field;
	enum FE_nodal_value_type nodal_value_type;
	int version_number;
};

/*******************************************************************************
COMPUTED_FIELD_TYPE: embedded

LAST MODIFIED : 21 January 2002

DESCRIPTION :
Evaluates another computed field at the element:xi position returned by the
element_xi_fe_field.
==============================================================================*/

struct Computed_field_embedded_type_specific_data
{
	struct FE_field *element_xi_fe_field;
	struct FE_element_field_values *fe_element_field_values;
};

/*
Module constants
----------------
*/
static char computed_field_finite_element_type_string[]="finite_element";
static char computed_field_cmiss_number_type_string[]="cmiss_number";
#if defined (COMPUTED_FIELD_ACCESS_COUNT)
static char computed_field_access_count_type_string[]="access_count";
#endif /* defined (COMPUTED_FIELD_ACCESS_COUNT) */
static char computed_field_node_value_type_string[]="node_value";
static char computed_field_embedded_type_string[]="embedded";

/*
Module functions
----------------
*/
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
	if (field && (data = 
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
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_finite_element_type_specific_data *destination,
		*source;

	ENTER(Computed_field_finite_element_copy_type_specific);
	if (source_field && destination_field && (source = 
		(struct Computed_field_finite_element_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_finite_element_type_specific_data, 1))
		{
			if (source->fe_field)
			{
				destination->fe_field=ACCESS(FE_field)(source->fe_field);
			}
			destination->fe_element_field_values = 
				(struct FE_element_field_values *)NULL;
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

static int Computed_field_finite_element_clear_cache_type_specific(
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
		if(data->fe_element_field_values)
		{
			clear_FE_element_field_values(data->fe_element_field_values);
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

static int Computed_field_finite_element_is_defined_in_element(struct Computed_field *field,
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

static int Computed_field_finite_element_is_defined_at_node(struct Computed_field *field,
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

static int Computed_field_finite_element_has_multiple_times(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Check the fe_field
==============================================================================*/
{
	int return_code;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_finite_element_has_multiple_times);
	if (field && (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		return_code=FE_field_has_multiple_times(data->fe_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element_has_multiple_times.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_finite_element_has_multiple_times */

static int Computed_field_finite_element_has_numerical_components(
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

static int Computed_field_finite_element_not_in_use(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 13 May 2003

DESCRIPTION :
The FE_field must also not be in use.
==============================================================================*/
{
	int return_code;
	struct Computed_field_finite_element_type_specific_data *data;
	struct FE_region *fe_region;

	ENTER(Computed_field_finite_element_not_in_use);
	if (field && (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		/* check the fe_field can be destroyed */
		if (fe_region = FE_field_get_FE_region(data->fe_field))
		{
			/* ask owning FE_region if fe_field is used in nodes and elements */
			if (FE_region_is_FE_field_in_use(fe_region, data->fe_field))
			{
				return_code = 0;
			}
			else
			{
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_finite_element_not_in_use.  Missing FE_region");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element_not_in_use.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_finite_element_not_in_use */

static int Computed_field_calculate_FE_element_field_values_for_element(
	struct Computed_field *field,int calculate_derivatives,
	struct FE_element *element,FE_value time,struct FE_element *top_level_element)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

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
	if (field&&element&&
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
			(!FE_element_field_values_are_for_element_and_time(
				data->fe_element_field_values,element,time,top_level_element))||
			(calculate_derivatives&&
				(!FE_element_field_values_have_derivatives_calculated(data->fe_element_field_values))))
		{
			if (!data->fe_element_field_values)
			{
				if (!(data->fe_element_field_values = CREATE(FE_element_field_values)()))
				{
					return_code=0;
				}
			}
			else
			{
				/* following clears fe_element_field_values->element */
				clear_FE_element_field_values(data->fe_element_field_values);
			}
			if (return_code)
			{
				/* note that FE_element_field_values accesses the element */
				if (!calculate_FE_element_field_values(element,data->fe_field,
					time,calculate_derivatives,data->fe_element_field_values,
					top_level_element))
				{
					/* clear element to indicate that values are clear */
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
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	double double_value;
	enum Value_type value_type;
	float float_value;
	int i, int_value, return_code;
	short short_value;
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
						/*nodal_value_type*/FE_NODAL_VALUE,time,&double_value);
					field->values[i] = (FE_value)double_value;
				} break;
				case FE_VALUE_VALUE:
				{
					return_code=get_FE_nodal_FE_value_value(node,
						&fe_field_component,/*version_number*/0,
						/*nodal_value_type*/FE_NODAL_VALUE,time,&(field->values[i]));
				} break;
				case FLT_VALUE:
				{
					return_code=get_FE_nodal_float_value(node,&fe_field_component,
						/*version_number*/0,/*nodal_value_type*/FE_NODAL_VALUE,
						time,&float_value);
					field->values[i] = (FE_value)float_value;
				} break;
				case INT_VALUE:
				{
					return_code=get_FE_nodal_int_value(node,&fe_field_component,
						/*version_number*/0,/*nodal_value_type*/FE_NODAL_VALUE,
						time,&int_value);
					field->values[i] = (FE_value)int_value;
				} break;
				case SHORT_VALUE:
				{
					return_code=get_FE_nodal_short_value(node,&fe_field_component,
						/*version_number*/0,/*nodal_value_type*/FE_NODAL_VALUE,
						time,&short_value);
					field->values[i] = (FE_value)short_value;
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
	FE_value time,struct FE_element *top_level_element,int calculate_derivatives)
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
				field,calculate_derivatives,element,time,top_level_element))
		{
			/* 2. Calculate the field */
			value_type=get_FE_field_value_type(data->fe_field);
			/* component number -1 = calculate all components */
			switch (value_type)
			{
				case FE_VALUE_VALUE:
				case SHORT_VALUE:
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

static char *Computed_field_finite_element_evaluate_as_string_at_node(struct Computed_field *field,
	int component_number, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/
{
	char *return_string, start_component_number, stop_component_number,
		*temp_string;
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
				field, component_number, node, time);
		}
		else
		{
			if (-1 == component_number)
			{
				start_component_number = 0;
				stop_component_number = field->number_of_components - 1;
			}
			else
			{
				start_component_number = stop_component_number = component_number;
			}
			error = 0;
			for (i = start_component_number; i <= stop_component_number; i++)
			{
				if (get_FE_nodal_value_as_string(node, data->fe_field,
					i, /*version_number*/0, /*nodal_value_type*/FE_NODAL_VALUE,
					time, &temp_string))
				{
					append_string(&return_string, temp_string, &error);
					if (i < stop_component_number)
					{
						append_string(&return_string, ",", &error);
					}
					DEALLOCATE(temp_string);
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

static char *Computed_field_finite_element_evaluate_as_string_in_element(
	struct Computed_field *field,int component_number,struct FE_element *element,
	FE_value *xi,FE_value time,struct FE_element *top_level_element)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

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
				field, component_number, element, xi, time, top_level_element);
		}
		else
		{
			/* ensure we have FE_element_field_values for element, without
				requiring derivatives to be calculated  */
			if (Computed_field_calculate_FE_element_field_values_for_element(
				field,/*calculate_derivatives*/0,element,time,top_level_element))
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

static int Computed_field_finite_element_set_values_at_node(struct Computed_field *field,
	struct FE_node *node, FE_value time, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 28 October 2004

DESCRIPTION :
Sets the <values> of the computed <field> at <node>.
==============================================================================*/
{
	double double_value;
	enum Value_type value_type;
	float float_value;
	int i,int_value,j,k,return_code;
	short short_value;
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
							j,/*nodal_value_type*/FE_NODAL_VALUE,time,double_value);
					} break;
					case FE_VALUE_VALUE:
					{
						return_code=set_FE_nodal_FE_value_value(node,&fe_field_component,
							j,/*nodal_value_type*/FE_NODAL_VALUE,time,values[i]);
					} break;
					case FLT_VALUE:
					{
						float_value=(float)values[i];
						return_code=set_FE_nodal_float_value(node,&fe_field_component,
							j,/*nodal_value_type*/FE_NODAL_VALUE,time,float_value);
					} break;
					case INT_VALUE:
					{
						int_value=(int)floor(values[i]+0.5);
						return_code=set_FE_nodal_int_value(node,&fe_field_component,
							j,/*nodal_value_type*/FE_NODAL_VALUE,time,int_value);
					} break;
					case SHORT_VALUE:
					{
						short_value=(short)floor(values[i]+0.5);
						return_code=set_FE_nodal_short_value(node,&fe_field_component,
							j,/*nodal_value_type*/FE_NODAL_VALUE,time,short_value);
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

static int Computed_field_finite_element_set_values_in_element(
	struct Computed_field *field, struct FE_element *element,int *number_in_xi,
	FE_value time, FE_value *values)
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
	if (field&&element&&number_in_xi&&values && (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		return_code=1;
		if (time != 0)
		{
			display_message(WARNING_MESSAGE,
				"Computed_field_finite_element_set_values_in_element.  "
				"This function is not implemented for time.");
			return_code = 0;
		}
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
				Computed_field_get_type_string(field));
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
	
static int Computed_field_finite_element_get_native_discretization_in_element(
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
	if (field&&element&&number_in_xi&&
		(MAXIMUM_ELEMENT_XI_DIMENSIONS>=get_FE_element_dimension(element)) && (data = 
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

static int Computed_field_finite_element_find_element_xi(
	struct Computed_field *field, 
	FE_value *values, int number_of_values, struct FE_element **element,
	FE_value *xi, int element_dimension, struct Cmiss_region *search_region)
/*******************************************************************************
LAST MODIFIED : 11 March 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_find_element_xi);
	if (field && values && (number_of_values == field->number_of_components) &&
		element && xi && search_region)
	{
		return_code = Computed_field_perform_find_element_xi(field,
			values, number_of_values, element, xi, element_dimension, search_region);	
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
LAST MODIFIED : 2 May 2002

DESCRIPTION :
==============================================================================*/
{
	char *field_name;
	int return_code;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(List_Computed_field_finite_element);
	if (field && (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		if (return_code=GET_NAME(FE_field)(data->fe_field,&field_name))
		{
			display_message(INFORMATION_MESSAGE,"    fe_field : %s\n",field_name);
			DEALLOCATE(field_name);
		}
		display_message(INFORMATION_MESSAGE,"    CM field type : %s\n",
			ENUMERATOR_STRING(CM_field_type)(get_FE_field_CM_field_type(data->fe_field)));
		display_message(INFORMATION_MESSAGE,"    Value type : %s\n",
			Value_type_string(get_FE_field_value_type(data->fe_field)));
		return_code = 1;
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

static char *Computed_field_finite_element_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 16 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *component_name, temp_string[40];
	int error, i, number_of_components;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_finite_element_get_command_string);
	command_string = (char *)NULL;
	if (field && (data = 
		(struct Computed_field_finite_element_type_specific_data *)
		field->type_specific_data))
	{
		error = 0;
		append_string(&command_string,
			computed_field_finite_element_type_string, &error);
		number_of_components = get_FE_field_number_of_components(data->fe_field);
		sprintf(temp_string, " number_of_components %d ", number_of_components);
		append_string(&command_string, temp_string, &error);
		append_string(&command_string, ENUMERATOR_STRING(CM_field_type)(
			get_FE_field_CM_field_type(data->fe_field)), &error);
		append_string(&command_string, " ", &error);
		append_string(&command_string,
			Value_type_string(get_FE_field_value_type(data->fe_field)), &error);
		append_string(&command_string, " component_names", &error);
		for (i = 0; i < number_of_components; i++)
		{
			if (component_name = get_FE_field_component_name(data->fe_field, i))
			{
				make_valid_token(&component_name);
				append_string(&command_string, " ", &error);
				append_string(&command_string, component_name, &error);
				DEALLOCATE(component_name);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_finite_element_get_command_string */

static int define_Computed_field_type_finite_element(struct Parse_state *state,
	void *field_void,void *computed_field_finite_element_package_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

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
	struct Computed_field *field;
	struct Computed_field_finite_element_package *computed_field_finite_element_package;
	struct Coordinate_system *coordinate_system;
	struct FE_field *existing_fe_field,*fe_field;
	struct Option_table *option_table;

	ENTER(define_Computed_field_type_finite_element);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_finite_element_package=
		(struct Computed_field_finite_element_package *)
		computed_field_finite_element_package_void))
	{
		return_code = 1;
		existing_fe_field = (struct FE_field *)NULL;
		if (computed_field_finite_element_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code =
				Computed_field_get_type_finite_element(field, &existing_fe_field);
		}
		if (return_code)
		{
			if (existing_fe_field)
			{
				number_of_components=
					get_FE_field_number_of_components(existing_fe_field);
				cm_field_type = get_FE_field_CM_field_type(existing_fe_field);
				value_type = get_FE_field_value_type(existing_fe_field);
			}
			else
			{
				/* default to real, 3-component coordinate field */
				number_of_components = 3;
				cm_field_type = CM_COORDINATE_FIELD;
				value_type = FE_VALUE_VALUE;
			}
			if (return_code)
			{
				cm_field_type_string = ENUMERATOR_STRING(CM_field_type)(cm_field_type);
				value_type_string = Value_type_string(value_type);
			}
			if (ALLOCATE(component_names, char *, number_of_components))
			{
				for (i = 0; i < number_of_components; i++)
				{
					if (existing_fe_field)
					{
						component_names[i] =
							get_FE_field_component_name(existing_fe_field, i);
					}
					else
					{
						component_names[i] = (char *)NULL;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_finite_element.  Not enough memory");
				return_code = 0;
			}
			original_number_of_components = number_of_components;
			/* try to handle help first */
			if (return_code && (current_token = state->current_token))
			{
				if (!(strcmp(PARSER_HELP_STRING, current_token) &&
					strcmp(PARSER_RECURSIVE_HELP_STRING, current_token)))
				{
					option_table = CREATE(Option_table)();
					/* cm_field_type */
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(CM_field_type)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(CM_field_type) *)NULL,
						(void *)NULL);
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
			if (return_code && state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* cm_field_type */
				valid_strings = ENUMERATOR_GET_VALID_STRINGS(CM_field_type)(
					&number_of_valid_strings,
					(ENUMERATOR_CONDITIONAL_FUNCTION(CM_field_type) *)NULL,
					(void *)NULL);
				Option_table_add_enumerator(option_table, number_of_valid_strings,
					valid_strings, &cm_field_type_string);
				DEALLOCATE(valid_strings);
				/* component_names */
				Option_table_add_entry(option_table, "component_names", component_names,
					&number_of_components, set_names);
				/* value_type */
				valid_strings = Value_type_get_valid_strings_simple(
					&number_of_valid_strings);
				Option_table_add_enumerator(option_table, number_of_valid_strings,
					valid_strings, &value_type_string);
				DEALLOCATE(valid_strings);
				return_code = Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			if (return_code)
			{
				value_type = Value_type_from_string(value_type_string);
				STRING_TO_ENUMERATOR(CM_field_type)(cm_field_type_string,
					&cm_field_type);
				/* now make an FE_field to match the options entered */
				if (fe_field = CREATE(FE_field)(field->name,
					computed_field_finite_element_package->fe_region))
				{
					ACCESS(FE_field)(fe_field);
					if (return_code&&existing_fe_field)
					{
						/* copy existing field to get as much in common with it as
							 possible */
						return_code=FE_field_copy_without_identifier(fe_field,
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
						if ((!existing_fe_field) ||
							FE_fields_match_fundamental(fe_field, existing_fe_field))
						{
							if (FE_region_merge_FE_field(
								computed_field_finite_element_package->fe_region, fe_field))
							{
								return_code = 1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"define_Computed_field_type_finite_element.  "
									"Unable to merge FE_field");
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cannot fundamentally redefine finite_element field "
								"while it is in use");
							display_parse_state_location(state);
							return_code = 0;
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
						if (return_code =
							Computed_field_set_type_finite_element(field, fe_field,
								computed_field_finite_element_package->fe_region))
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
				DEALLOCATE(component_names);
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

#define Computed_field_cmiss_number_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_cmiss_number_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_cmiss_number_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_cmiss_number_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_cmiss_number_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_cmiss_number_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_cmiss_number_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_cmiss_number_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_cmiss_number_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_cmiss_number_evaluate_cache_at_node);
	USE_PARAMETER(time);
	if (field && node)
	{
		/* simply convert the node number into an FE_value */
		field->values[0]=(FE_value)get_FE_node_identifier(node);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cmiss_number_evaluate_cache_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cmiss_number_evaluate_cache_at_node */

static int Computed_field_cmiss_number_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Evaluate the fields cache in the element.
==============================================================================*/
{
	int element_dimension, i, return_code;
	struct CM_element_information cm_identifier;

	ENTER(Computed_field_cmiss_number_evaluate_cache_in_element);
	USE_PARAMETER(time);
	USE_PARAMETER(top_level_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi)
	{
		/* simply convert the element number into an FE_value */
		if (get_FE_element_identifier(element, &cm_identifier))
		{
			field->values[0] = cm_identifier.number;
			return_code = 1;
		}
		else
		{
			field->values[0] = 0;
			return_code = 0;
		}
		/* derivatives are always zero for this type, hence always calculated */
		element_dimension = get_FE_element_dimension(element);
		for (i = 0; i < element_dimension; i++)
		{
			field->derivatives[i] = 0.0;
		}
		field->derivatives_valid = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cmiss_number_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cmiss_number_evaluate_cache_in_element */

static char *Computed_field_cmiss_number_evaluate_as_string_at_node(
	struct Computed_field *field, int component_number, struct FE_node *node,
	FE_value time)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/
{
	char *return_string,tmp_string[50];
	int error;

	ENTER(Computed_field_cmiss_number_evaluate_as_string_at_node);
	USE_PARAMETER(time);
	return_string=(char *)NULL;
	if (field&&node&&(component_number >= -1)&&
		(component_number<field->number_of_components))
	{
		error=0;
		/* put out the cmiss number as a string */
		sprintf(tmp_string,"%d",get_FE_node_identifier(node));
		append_string(&return_string,tmp_string,&error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cmiss_number_evaluate_as_string_at_node.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_string);
} /* Computed_field_cmiss_number_evaluate_as_string_at_node */

static char *Computed_field_cmiss_number_evaluate_as_string_in_element(
	struct Computed_field *field,int component_number,struct FE_element *element,
	FE_value *xi,FE_value time,struct FE_element *top_level_element)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/
{
	char *return_string,tmp_string[50];
	struct CM_element_information cm_identifier;

	ENTER(Computed_field_cmiss_number_evaluate_as_string_in_element);
	USE_PARAMETER(top_level_element);
	USE_PARAMETER(time);
	return_string=(char *)NULL;
	if (field&&element&&xi&&(-1<=component_number)&&
		(component_number < field->number_of_components))
	{
	  /* put out the cmiss number as a string */
		if (get_FE_element_identifier(element, &cm_identifier))
		{
			sprintf(tmp_string, "%d", cm_identifier.number);
			return_string=duplicate_string(tmp_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cmiss_number_evaluate_as_string_in_element.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_string);
} /* Computed_field_cmiss_number_evaluate_as_string_in_element */

#define Computed_field_cmiss_number_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_cmiss_number_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_cmiss_number_get_native_discretization_in_element \
	(Computed_field_get_native_discretization_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 16 August 2000

DESCRIPTION :
A property of the element = not grid-based.
==============================================================================*/

#define Computed_field_cmiss_number_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_cmiss_number(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_cmiss_number);
	if (field)
	{
		/* no extra parameters */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_cmiss_number.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_cmiss_number */

static char *Computed_field_cmiss_number_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string;

	ENTER(Computed_field_cmiss_number_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		/* no command options */
		command_string = duplicate_string(computed_field_cmiss_number_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cmiss_number_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_cmiss_number_get_command_string */

static int define_Computed_field_type_cmiss_number(struct Parse_state *state,
	void *field_void,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CMISS_NUMBER.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field;

	ENTER(define_Computed_field_type_cmiss_number);
	USE_PARAMETER(dummy_void);
	if (state&&(field=(struct Computed_field *)field_void))
	{
		if (!state->current_token)
		{
			return_code=Computed_field_set_type_cmiss_number(field);
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
			"define_Computed_field_type_cmiss_number.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_cmiss_number */

#if defined (COMPUTED_FIELD_ACCESS_COUNT)
#define Computed_field_access_count_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_access_count_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_access_count_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_access_count_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_access_count_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_access_count_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_access_count_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_access_count_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_access_count_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_access_count_evaluate_cache_at_node);
	if (field && node)
	{
		/* simply convert the node number into an FE_value */
		field->values[0]=get_FE_node_access_count(node);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_access_count_evaluate_cache_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_access_count_evaluate_cache_at_node */

static int Computed_field_access_count_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_access_count_evaluate_cache_in_element);
	USE_PARAMETER(top_level_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi)
	{
	  /* simply convert the element number into an FE_value */
	  field->values[0]=(FE_value)element->access_count;
	  /* no derivatives for this type */
	  field->derivatives_valid=0;
	  return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_access_count_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_access_count_evaluate_cache_in_element */

static char *Computed_field_access_count_evaluate_as_string_at_node(
	struct Computed_field *field, int component_number, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/
{
	char *return_string,tmp_string[50];
	int error;

	ENTER(Computed_field_access_count_evaluate_as_string_at_node);
	return_string=(char *)NULL;
	if (field&&node&&(component_number >= -1)&&
		(component_number<field->number_of_components))
	{
		error=0;
		/* put out the cmiss number as a string */
		sprintf(tmp_string,"%d",get_FE_node_access_count(node));
		append_string(&return_string,tmp_string,&error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_access_count_evaluate_as_string_at_node.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_string);
} /* Computed_field_access_count_evaluate_as_string_at_node */

static char *Computed_field_access_count_evaluate_as_string_in_element(
	struct Computed_field *field,int component_number,
	struct FE_element *element,FE_value *xi,struct FE_element *top_level_element)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/
{
	char *return_string,tmp_string[50];

	ENTER(Computed_field_access_count_evaluate_as_string_in_element);
	USE_PARAMETER(top_level_element);
	return_string=(char *)NULL;
	if (field&&element&&xi&&(-1<=component_number)&&
		(component_number < field->number_of_components))
	{
	  /* put out the cmiss number as a string */
	  sprintf(tmp_string,"%d",element->access_count);
	  return_string=duplicate_string(tmp_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_access_count_evaluate_as_string_in_element.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_string);
} /* Computed_field_access_count_evaluate_as_string_in_element */

#define Computed_field_access_count_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_access_count_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_access_count_get_native_discretization_in_element \
	(Computed_field_get_native_discretization_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 16 August 2000

DESCRIPTION :
A property of the element = not grid-based.
==============================================================================*/

#define Computed_field_access_count_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_access_count(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_access_count);
	if (field)
	{
		/* no extra parameters */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_access_count.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_access_count */

static char *Computed_field_access_count_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string;

	ENTER(Computed_field_access_count_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		/* no command options */
		command_string = duplicate_string(computed_field_access_count_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_access_count_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_access_count_get_command_string */

int Computed_field_is_type_access_count(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_access_count);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_access_count_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_access_count.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_type_access_count */

#define Computed_field_access_count_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_access_count(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ACCESS_COUNT with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_set_type_access_count);
	if (field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		/* none */
		/* 2. free current type-specific data */
		Computed_field_clear_type(field);
		/* 3. establish the new type */
		field->type=COMPUTED_FIELD_NEW_TYPES;
		field->type_string = computed_field_access_count_type_string;
		field->number_of_components = 1;
		field->type_specific_data = (void *)1;
		
		/* Set all the methods */
		COMPUTED_FIELD_ESTABLISH_METHODS(access_count);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_access_count.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_access_count */

static int define_Computed_field_type_access_count(struct Parse_state *state,
	void *field_void,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_ACCESS_COUNT.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field;

	ENTER(define_Computed_field_type_access_count);
	USE_PARAMETER(dummy_void);
	if (state&&(field=(struct Computed_field *)field_void))
	{
		if (!state->current_token)
		{
			return_code=Computed_field_set_type_access_count(field);
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
			"define_Computed_field_type_access_count.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_access_count */
#endif /* defined (COMPUTED_FIELD_ACCESS_COUNT) */

static char computed_field_xi_coordinates_type_string[] = "xi_coordinates";

int Computed_field_is_type_xi_coordinates(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_xi_coordinates);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_xi_coordinates_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_xi_coordinates.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_type_xi_coordinates */

#define Computed_field_xi_coordinates_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_xi_coordinates_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_xi_coordinates_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_xi_coordinates_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_xi_coordinates_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

static int Computed_field_xi_coordinates_is_defined_at_node(
	struct Computed_field *field, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_xi_coordinates_is_defined_at_node);
	if (field && node)
	{
		/* can not be evaluated at nodes */
		return_code = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_xi_coordinates_is_defined_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_xi_coordinates_is_defined_at_node */

#define Computed_field_xi_coordinates_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_xi_coordinates_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
No special criteria.
==============================================================================*/

#define Computed_field_xi_coordinates_evaluate_cache_at_node \
	(Computed_field_evaluate_cache_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Cannot evaluate xi at nodes
==============================================================================*/

static int Computed_field_xi_coordinates_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value *temp;
	int element_dimension, i, j, return_code;

	ENTER(Computed_field_xi_coordinates_evaluate_cache_in_element);
	USE_PARAMETER(time);
	USE_PARAMETER(top_level_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi)
	{
		/* returns the values in xi, up to the element_dimension and padded
			with zeroes */
		element_dimension=get_FE_element_dimension(element);
		temp=field->derivatives;
		for (i=0;i<field->number_of_components;i++)
		{
			if (i<element_dimension)
			{
				field->values[i]=xi[i];
			}
			else
			{
				field->values[i]=0.0;
			}
			for (j=0;j<element_dimension;j++)
			{
				if (i==j)
				{
					*temp = 1.0;
				}
				else
				{
					*temp = 0.0;
				}
				temp++;
			}
		}
		/* derivatives are always calculated since they are merely part of
			the identity matrix */
		field->derivatives_valid=1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_xi_coordinates_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_xi_coordinates_evaluate_cache_in_element */

#define Computed_field_xi_coordinates_evaluate_as_string_at_node \
	(Computed_field_evaluate_as_string_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_xi_coordinates_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_xi_coordinates_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_xi_coordinates_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_xi_coordinates_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_xi_coordinates_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_xi_coordinates(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_xi_coordinates);
	if (field)
	{
		/* no extra parameters */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_xi_coordinates.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_xi_coordinates */

static char *Computed_field_xi_coordinates_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string;

	ENTER(Computed_field_xi_coordinates_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		/* no command options */
		command_string =
			duplicate_string(computed_field_xi_coordinates_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_xi_coordinates_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_xi_coordinates_get_command_string */

static int define_Computed_field_type_xi_coordinates(struct Parse_state *state,
	void *field_void,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_XI_COORDINATES.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field;

	ENTER(define_Computed_field_type_xi_coordinates);
	USE_PARAMETER(dummy_void);
	if (state&&(field=(struct Computed_field *)field_void))
	{
		if (!state->current_token)
		{
			return_code=Computed_field_set_type_xi_coordinates(field);
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
			"define_Computed_field_type_xi_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_xi_coordinates */

int Computed_field_is_type_node_value(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_node_value);
	if (field)
	{
		return_code = (field->type_string == computed_field_node_value_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_node_value.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_type_node_value */

static int Computed_field_node_value_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_node_value_type_specific_data *data;

	ENTER(Computed_field_node_value_clear_type_specific);
	if (field&& (data = 
		(struct Computed_field_node_value_type_specific_data *)
		field->type_specific_data))
	{
		if (data->fe_field)
		{
			DEACCESS(FE_field)(&(data->fe_field));
		}
		
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_node_value_clear_type_specific */

static void *Computed_field_node_value_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_node_value_type_specific_data *destination,
		*source;

	ENTER(Computed_field_node_value_copy_type_specific);
	if (source_field && destination_field && (source = 
		(struct Computed_field_node_value_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_node_value_type_specific_data, 1))
		{
			if (source->fe_field)
			{
				destination->fe_field=ACCESS(FE_field)(source->fe_field);
			}
			destination->nodal_value_type=source->nodal_value_type;
			destination->version_number=source->version_number;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_node_value_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_node_value_copy_type_specific */

#define Computed_field_node_value_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
==============================================================================*/

static int Computed_field_node_value_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_node_value_type_specific_data *data, *other_data;

	ENTER(Computed_field_node_value_type_specific_contents_match);
	if (field && other_computed_field && (data = 
		(struct Computed_field_node_value_type_specific_data *)
		field->type_specific_data) && (other_data = 
		(struct Computed_field_node_value_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		return_code = ((data->fe_field == other_data->fe_field)
			&& (data->nodal_value_type == other_data->nodal_value_type)
			&& (data->version_number == other_data->version_number));
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_node_value_type_specific_contents_match */

static int Computed_field_node_value_is_defined_in_element(struct Computed_field *field,
	struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_node_value_is_defined_in_element);
	if (field && element)
	{
	  return_code=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value_is_defined_in_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_node_value_is_defined_in_element */

static int Computed_field_node_value_is_defined_at_node(struct Computed_field *field,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Check the fe_field
==============================================================================*/
{
	int comp_no,return_code;
	struct Computed_field_node_value_type_specific_data *data;
	struct FE_field_component fe_field_component;

	ENTER(Computed_field_node_value_is_defined_at_node);
	if (field && node && (data = 
		(struct Computed_field_node_value_type_specific_data *)
		field->type_specific_data))
	{
		if (FE_field_is_defined_at_node(data->fe_field,node))
		{
			/* must ensure at least one component of version_number,
				nodal_value_type defined at node */
			return_code=0;
			fe_field_component.field=data->fe_field;
			for (comp_no=0;(comp_no<field->number_of_components)&&(!return_code);
				  comp_no++)
			{
				fe_field_component.number=comp_no;
				if (FE_nodal_value_version_exists(node,&fe_field_component,
					data->version_number,data->nodal_value_type))
				{
					return_code=1;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value_is_defined_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_node_value_is_defined_at_node */

static int Computed_field_node_value_has_numerical_components(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_node_value_type_specific_data *data;

	ENTER(Computed_field_node_value_has_numerical_components);
	if (field && (data = 
		(struct Computed_field_node_value_type_specific_data *)
		field->type_specific_data))
	{
		return_code=Value_type_is_numeric_simple(
			get_FE_field_value_type(data->fe_field));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value_has_numerical_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_node_value_has_numerical_components */

#define Computed_field_node_value_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
==============================================================================*/

static int Computed_field_node_value_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	double double_value;
	enum Value_type value_type;
	float float_value;
	int i, int_value, return_code;
	short short_value;
	struct Computed_field_node_value_type_specific_data *data;
	struct FE_field_component fe_field_component;

	ENTER(Computed_field_node_value_evaluate_cache_at_node);
	if (field && node && (data = 
		(struct Computed_field_node_value_type_specific_data *)
		field->type_specific_data))
	{
		return_code = 1;
		fe_field_component.field=data->fe_field;
		value_type=get_FE_field_value_type(data->fe_field);
		for (i=0;(i<field->number_of_components)&&return_code;i++)
		{
			fe_field_component.number=i;
			if (FE_nodal_value_version_exists(node,&fe_field_component,
				data->version_number,data->nodal_value_type))
			{
				switch (value_type)
				{
					case DOUBLE_VALUE:
					{
						return_code=get_FE_nodal_double_value(node,
							&fe_field_component,data->version_number,
							data->nodal_value_type,time,&double_value);
						field->values[i] = (FE_value)double_value;
					} break;
					case FE_VALUE_VALUE:
					{
						return_code=get_FE_nodal_FE_value_value(node,
							&fe_field_component,data->version_number,
							data->nodal_value_type,time,&(field->values[i]));
					} break;
					case FLT_VALUE:
					{
						return_code=get_FE_nodal_float_value(node,
							&fe_field_component,data->version_number,
							data->nodal_value_type,time,&float_value);
						field->values[i] = (FE_value)float_value;
					} break;
					case INT_VALUE:
					{
						return_code=get_FE_nodal_int_value(node,&fe_field_component,
							data->version_number,data->nodal_value_type,time,&int_value);
						field->values[i] = (FE_value)int_value;
					} break;
					case SHORT_VALUE:
					{
						return_code=get_FE_nodal_short_value(node,&fe_field_component,
							data->version_number,data->nodal_value_type,time,&short_value);
						field->values[i] = (FE_value)short_value;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_node_value_evaluate_cache_at_node.  "
							"Unsupported value type %s in node_value field",
							Value_type_string(value_type));
						return_code=0;
					}
				}
			}
			else
			{
				/* use 0 for all undefined components */
				field->values[i]=0.0;
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_node_value_evaluate_cache_at_node.  "
				"Error evaluating node_value field at node");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value_evaluate_cache_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_node_value_evaluate_cache_at_node */

#define Computed_field_node_value_evaluate_cache_in_element \
   (Computed_field_evaluate_cache_in_element_function)NULL;
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Cannot evaluate node_value in elements
==============================================================================*/

static char *Computed_field_node_value_evaluate_as_string_at_node(
	struct Computed_field *field, int component_number, struct FE_node *node, 
	FE_value time)
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/
{
	char *return_string, start_component_number, stop_component_number,
		*temp_string;
	int error, i;
	struct Computed_field_node_value_type_specific_data *data;

	ENTER(Computed_field_node_value_evaluate_as_string_at_node);
	return_string = (char *)NULL;
	if (field && node && (data = 
		(struct Computed_field_node_value_type_specific_data *)
		field->type_specific_data))
	{
		if (FE_VALUE_VALUE == get_FE_field_value_type(data->fe_field))
		{
			/* Then we can just use the default function and therefore use the
				values if they are already in the cache */
			return_string = Computed_field_default_evaluate_as_string_at_node(
				field, component_number, node, time);
		}
		else
		{
			if (-1 == component_number)
			{
				start_component_number = 0;
				stop_component_number = field->number_of_components - 1;
			}
			else
			{
				start_component_number = stop_component_number = component_number;
			}
			error = 0;
			for (i = start_component_number; i <= stop_component_number; i++)
			{
				if (get_FE_nodal_value_as_string(node, data->fe_field,
					i, data->version_number, data->nodal_value_type,
					time, &temp_string))
				{
					append_string(&return_string, temp_string, &error);
					if (i < stop_component_number)
					{
						append_string(&return_string, ",", &error);
					}
					DEALLOCATE(temp_string);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value_evaluate_as_string_at_node.  "
			"Invalid arguments.");
		return_string = (char *)NULL;
	}
	LEAVE;

	return (return_string);
} /* Computed_field_node_value_evaluate_as_string_at_node */

#define Computed_field_node_value_evaluate_as_string_in_element \
   (Computed_field_evaluate_as_string_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
==============================================================================*/

static int Computed_field_node_value_set_values_at_node(struct Computed_field *field,
	struct FE_node *node, FE_value time, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 28 October 2004

DESCRIPTION :
Sets the <values> of the computed <field> at <node>.
==============================================================================*/
{
	double double_value;
	enum Value_type value_type;
	float float_value;
	int i,int_value,return_code;
	struct Computed_field_node_value_type_specific_data *data;
	struct FE_field_component fe_field_component;

	ENTER(Computed_field_node_value_set_values_at_node);
	if (field&&node&&values && (data = 
		(struct Computed_field_node_value_type_specific_data *)
		field->type_specific_data))
	{
		return_code=1;

		fe_field_component.field=data->fe_field;
		value_type=get_FE_field_value_type(data->fe_field);
		for (i=0;(i<field->number_of_components)&&return_code;i++)
		{
			fe_field_component.number=i;
			/* only set nodal value/versions that exist */
			if (FE_nodal_value_version_exists(node,&fe_field_component,
				data->version_number,data->nodal_value_type))
			{
				switch (value_type)
				{
					case DOUBLE_VALUE:
					{
						double_value=(double)values[i];
						return_code=set_FE_nodal_double_value(node,&fe_field_component,
							data->version_number,data->nodal_value_type,time,double_value);
					} break;
					case FE_VALUE_VALUE:
					{
						return_code=set_FE_nodal_FE_value_value(node,&fe_field_component,
							data->version_number,data->nodal_value_type,time,values[i]);
					} break;
					case FLT_VALUE:
					{
						float_value=(float)values[i];
						return_code=set_FE_nodal_float_value(node,&fe_field_component,
							data->version_number,data->nodal_value_type,time,float_value);
					} break;
					case INT_VALUE:
					{
						int_value=(int)floor(values[i]+0.5);
						return_code=set_FE_nodal_float_value(node,&fe_field_component,
							data->version_number,data->nodal_value_type,time,int_value);
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_set_values_at_node.  "
							"Could not set finite_element field %s at node",field->name);
						return_code=0;
					}
				}
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_values_at_node.  "
				"Could not set node_value field at node",field->name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value_set_values_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_node_value_set_values_at_node */

#define Computed_field_node_value_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
==============================================================================*/
	
#define Computed_field_node_value_get_native_discretization_in_element \
   (Computed_field_get_native_discretization_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
==============================================================================*/

#define Computed_field_node_value_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
==============================================================================*/

static int list_Computed_field_node_value(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
==============================================================================*/
{
	char *field_name;
	int return_code;
	struct Computed_field_node_value_type_specific_data *data;

	ENTER(List_Computed_field_node_value);
	if (field && (data = 
		(struct Computed_field_node_value_type_specific_data *)
		field->type_specific_data))
	{
		if (return_code=GET_NAME(FE_field)(data->fe_field,&field_name))
		{
			display_message(INFORMATION_MESSAGE,"    fe_field : %s\n",field_name);
			display_message(INFORMATION_MESSAGE,"    nodal value type : %s\n",
				ENUMERATOR_STRING(FE_nodal_value_type)(data->nodal_value_type));
			display_message(INFORMATION_MESSAGE,"    version : %d\n",
				data->version_number+1);
			DEALLOCATE(field_name);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_node_value.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_node_value */

static char *Computed_field_node_value_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;
	struct Computed_field_node_value_type_specific_data *data;

	ENTER(Computed_field_node_value_get_command_string);
	command_string = (char *)NULL;
	if (field && (data = 
		(struct Computed_field_node_value_type_specific_data *)
		field->type_specific_data))
	{
		error = 0;
		append_string(&command_string,
			computed_field_node_value_type_string, &error);
		append_string(&command_string, " fe_field ", &error);
		if (GET_NAME(FE_field)(data->fe_field, &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " ", &error);
		append_string(&command_string,
			ENUMERATOR_STRING(FE_nodal_value_type)(data->nodal_value_type), &error);
		sprintf(temp_string, " version %d", data->version_number + 1);
		append_string(&command_string, temp_string, &error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_node_value_get_command_string */

static int Computed_field_node_value_has_multiple_times(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Check the fe_field
==============================================================================*/
{
	int return_code;
	struct Computed_field_node_value_type_specific_data *data;

	ENTER(Computed_field_node_value_has_multiple_times);
	if (field && (data = 
		(struct Computed_field_node_value_type_specific_data *)
		field->type_specific_data))
	{
		return_code=FE_field_has_multiple_times(data->fe_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value_has_multiple_times.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_node_value_has_multiple_times */

int Computed_field_set_type_node_value(struct Computed_field *field,
	struct FE_field *fe_field,enum FE_nodal_value_type nodal_value_type,
	int version_number)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_NODE_VALUE, returning the values for the
given <nodal_value_type> and <version_number> of <fe_field> at a node.
Makes the number of components the same as in the <fe_field>.
Field automatically takes the coordinate system of the source fe_field. See note
at start of this file about changing use of coordinate systems.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	char **component_names;
	int i, number_of_components, return_code;
	struct Computed_field_node_value_type_specific_data *data;

	ENTER(Computed_field_set_type_node_value);
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
			ALLOCATE(data,struct Computed_field_node_value_type_specific_data, 1))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_node_value_type_string;
			field->number_of_components = number_of_components;
			field->component_names = component_names;
			field->type_specific_data = (void *)data;
			data->fe_field = ACCESS(FE_field)(fe_field);
			data->nodal_value_type=nodal_value_type;
			data->version_number=version_number;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(node_value);
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
			"Computed_field_set_type_node_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_node_value */

static int Computed_field_get_type_node_value(struct Computed_field *field,
	struct FE_field **fe_field,enum FE_nodal_value_type *nodal_value_type,
	int *version_number)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_NODE_VALUE, the FE_field being
"wrapped" by it is returned - otherwise an error is reported.
==============================================================================*/
{
	int return_code;
	struct Computed_field_node_value_type_specific_data *data;

	ENTER(Computed_field_get_type_node_value);
	if (field&&(field->type_string==computed_field_node_value_type_string) 
		&& (data = (struct Computed_field_node_value_type_specific_data *)
		field->type_specific_data))
	{
		*fe_field=data->fe_field;
		*nodal_value_type=data->nodal_value_type;
		*version_number=data->version_number;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_node_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_node_value */

static int define_Computed_field_type_node_value(struct Parse_state *state,
	void *field_void,void *computed_field_finite_element_package_void)
/*******************************************************************************
LAST MODIFIED : 16 April 2003

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_NODE_VALUE (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	char *current_token, *nodal_value_type_string;
	enum FE_nodal_value_type nodal_value_type;
	int return_code,version_number;
	static char *nodal_value_type_strings[] =
	{
	  "value",
	  "d/ds1",
	  "d/ds2",
	  "d/ds3",
	  "d2/ds1ds2",
	  "d2/ds1ds3",
	  "d2/ds2ds3",
	  "d3/ds1ds2ds3"
	};
	struct Computed_field *field;
	struct Computed_field_finite_element_package 
		*computed_field_finite_element_package;
	struct FE_field *fe_field;
	struct Option_table *option_table;

	ENTER(define_Computed_field_type_node_value);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_finite_element_package=
		(struct Computed_field_finite_element_package *)
		computed_field_finite_element_package_void))
	{
		return_code=1;
		fe_field=(struct FE_field *)NULL;
		nodal_value_type=FE_NODAL_UNKNOWN;
		/* user enters version number starting at 1; field stores it as 0 */
		version_number=1;
		if (computed_field_node_value_type_string ==
		  Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_node_value(field,&fe_field,
				&nodal_value_type,&version_number);
			version_number++;
		}
		if (return_code)
		{
			/* have ACCESS/DEACCESS because set_FE_field does */
			if (fe_field)
			{
				ACCESS(FE_field)(fe_field);
			}
			/* try to handle help first */
			if (current_token=state->current_token)
			{
				if (!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
				{
					option_table=CREATE(Option_table)();
					/* fe_field */
					Option_table_add_set_FE_field_from_FE_region(
						option_table, "fe_field" ,&fe_field,
					  computed_field_finite_element_package->fe_region);
					/* nodal_value_type */
					nodal_value_type_string=nodal_value_type_strings[0];
					Option_table_add_enumerator(option_table,
					  sizeof(nodal_value_type_strings)/sizeof(char *),
					  nodal_value_type_strings,&nodal_value_type_string);
					/* version_number */
					Option_table_add_entry(option_table,"version", &version_number,
					  NULL, set_int_positive);
					return_code=Option_table_multi_parse(option_table,state);
					DESTROY(Option_table)(&option_table);
				}
			}
			/* parse the fe_field if the "fe_field" token is next */
			if (return_code)
			{
				if ((current_token=state->current_token)&&
					fuzzy_string_compare(current_token,"fe_field"))
				{
					option_table=CREATE(Option_table)();
					/* fe_field */
					Option_table_add_set_FE_field_from_FE_region(
						option_table, "fe_field" ,&fe_field,
					  computed_field_finite_element_package->fe_region);
					if (return_code=Option_table_parse(option_table,state))
					{
						if (!fe_field)
						{
							display_parse_state_location(state);
							display_message(ERROR_MESSAGE,"Missing or invalid fe_field");
							return_code=0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
				else
				{
					display_parse_state_location(state);
					display_message(ERROR_MESSAGE,
						"Must specify fe_field before other options");
					return_code=0;
				}
			}
			/* parse the value_type/version number */
			if (return_code&&state->current_token)
			{
			  option_table=CREATE(Option_table)();
			  /* nodal_value_type */
			  nodal_value_type_string=nodal_value_type_strings[0];
			  Option_table_add_enumerator(option_table,
				 sizeof(nodal_value_type_strings)/sizeof(char *),
				 nodal_value_type_strings,&nodal_value_type_string);
			  /* version_number */
			  Option_table_add_entry(option_table,"version", &version_number,
				 NULL, set_int_positive);
			  return_code=Option_table_multi_parse(option_table,state);
			  DESTROY(Option_table)(&option_table);
			}
			if (return_code)
			{
				if (nodal_value_type_string == nodal_value_type_strings[0])
				{
					nodal_value_type = FE_NODAL_VALUE;
				}
				else if (nodal_value_type_string == nodal_value_type_strings[1])
				{
					nodal_value_type = FE_NODAL_D_DS1;
				}
				else if (nodal_value_type_string == nodal_value_type_strings[2])
				{
					nodal_value_type = FE_NODAL_D_DS2;
				}
				else if (nodal_value_type_string == nodal_value_type_strings[3])
				{
					nodal_value_type = FE_NODAL_D_DS3;
				}
				else if (nodal_value_type_string == nodal_value_type_strings[4])
				{
					nodal_value_type = FE_NODAL_D2_DS1DS2;
				}
				else if (nodal_value_type_string == nodal_value_type_strings[5])
				{
					nodal_value_type = FE_NODAL_D2_DS1DS3;
				}
				else if (nodal_value_type_string == nodal_value_type_strings[6])
				{
					nodal_value_type = FE_NODAL_D2_DS2DS3;
				}
				else if (nodal_value_type_string == nodal_value_type_strings[7])
				{
					nodal_value_type = FE_NODAL_D3_DS1DS2DS3;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_node_value.  "
						"Unknown nodal value string.");
					return_code = 0;
				}
			}
			if (return_code)
			{
				/* user enters version number starting at 1; field stores it as 0 */
				if (return_code=Computed_field_set_type_node_value(field,fe_field,
					nodal_value_type,version_number-1))
				{
					Computed_field_set_coordinate_system(field,
						get_FE_field_coordinate_system(fe_field));
				}
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_node_value.  Failed");
				}
			}
			if (fe_field)
			{
				DEACCESS(FE_field)(&fe_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_node_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_node_value */

int Computed_field_is_type_embedded(struct Computed_field *field, void *dummy)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_embedded);
	USE_PARAMETER(dummy);
	if (field)
	{
		return_code = (field->type_string == computed_field_embedded_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_embedded.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_type_embedded */

static int Computed_field_embedded_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_embedded_type_specific_data *data;

	ENTER(Computed_field_embedded_clear_type_specific);
	if (field&& (data = 
		(struct Computed_field_embedded_type_specific_data *)
		field->type_specific_data))
	{
		if (data->element_xi_fe_field)
		{
			DEACCESS(FE_field)(&(data->element_xi_fe_field));
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
			"Computed_field_embedded_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_embedded_clear_type_specific */

static void *Computed_field_embedded_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_embedded_type_specific_data *destination,
		*source;

	ENTER(Computed_field_embedded_copy_type_specific);
	if (source_field && destination_field && (source = 
		(struct Computed_field_embedded_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_embedded_type_specific_data, 1))
		{
			if (source->element_xi_fe_field)
			{
				destination->element_xi_fe_field =
					ACCESS(FE_field)(source->element_xi_fe_field);
			}
			/*???RC Following could be a leak? */
			destination->fe_element_field_values = 
				(struct FE_element_field_values *)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_embedded_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_embedded_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_embedded_copy_type_specific */

static int Computed_field_embedded_clear_cache_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

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
		if(data->fe_element_field_values)
		{
			clear_FE_element_field_values(data->fe_element_field_values);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_embedded_clear_cache_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_embedded_clear_cache_type_specific */

static int Computed_field_embedded_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_embedded_type_specific_data *data, *other_data;

	ENTER(Computed_field_embedded_type_specific_contents_match);
	if (field && other_computed_field && (data = 
		(struct Computed_field_embedded_type_specific_data *)
		field->type_specific_data) && (other_data = 
		(struct Computed_field_embedded_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		return_code = (data->element_xi_fe_field == other_data->element_xi_fe_field);
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_embedded_type_specific_contents_match */

static int Computed_field_embedded_is_defined_in_element(struct Computed_field *field,
	struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_embedded_is_defined_in_element);
	if (field && element)
	{
		return_code=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_embedded_is_defined_in_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_embedded_is_defined_in_element */

static int Computed_field_embedded_is_defined_at_node(struct Computed_field *field,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Check the fe_field
==============================================================================*/
{
	int comp_no,i,number_of_components,return_code;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct Computed_field_embedded_type_specific_data *data;
	struct FE_element *element;
	struct FE_field_component fe_field_component;

	ENTER(Computed_field_embedded_is_defined_at_node);
	if (field && node && (data = 
		(struct Computed_field_embedded_type_specific_data *)
		field->type_specific_data))
	{
		return_code = 0;
		if (FE_field_is_defined_at_node(data->element_xi_fe_field,node))
		{
			return_code=1;
			fe_field_component.field=data->element_xi_fe_field;
			number_of_components=
				get_FE_field_number_of_components(data->element_xi_fe_field);
			for (comp_no=0;(comp_no<number_of_components)&&return_code;comp_no++)
			{
				fe_field_component.number=comp_no;
				if (FE_nodal_value_version_exists(node,&fe_field_component,
					/*version_number*/0,FE_NODAL_VALUE)&&
					get_FE_nodal_element_xi_value(node,data->element_xi_fe_field,comp_no,
					 /*version_number*/0,FE_NODAL_VALUE,&element,xi))
				{
					for (i=0;(i<field->number_of_source_fields)&&return_code;i++)
					{
						if (!Computed_field_is_defined_in_element(
							field->source_fields[i],element))
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
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_embedded_is_defined_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_embedded_is_defined_at_node */

static int Computed_field_embedded_has_numerical_components(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_embedded_has_numerical_components);
	if (field)
	{
		return_code=Computed_field_has_numerical_components(
			field->source_fields[0],(void *)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_embedded_has_numerical_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_embedded_has_numerical_components */

#define Computed_field_embedded_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
No special criteria.
Does not matter if the element_xi_fe_field is in use or not.
==============================================================================*/

static int Computed_field_embedded_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int i, return_code;
	struct Computed_field_embedded_type_specific_data *data;
	struct FE_element *element;

	ENTER(Computed_field_embedded_evaluate_cache_at_node);
	if (field && node && (data = 
		(struct Computed_field_embedded_type_specific_data *)
		field->type_specific_data))
	{
		return_code = 1;
		if (get_FE_nodal_element_xi_value(node,
			data->element_xi_fe_field, /* component */ 0,
			/* version */ 0, FE_NODAL_VALUE, &element, xi))
		{
			/* now calculate source_fields[0] */
			if(Computed_field_evaluate_cache_in_element(
				field->source_fields[0],element,xi,time,(struct FE_element *)NULL,0))
			{
				for (i=0;i<field->number_of_components;i++)
				{
					field->values[i] = field->source_fields[0]->values[i];
				}
			}
			else
			{
				return_code = 0;
			}
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_embedded_evaluate_cache_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_embedded_evaluate_cache_at_node */

#define Computed_field_embedded_evaluate_cache_in_element \
   (Computed_field_evaluate_cache_in_element_function)NULL;
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Cannot evaluate embedded in elements
==============================================================================*/

#define Computed_field_embedded_evaluate_as_string_at_node \
   Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_embedded_evaluate_as_string_in_element \
   (Computed_field_evaluate_as_string_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
==============================================================================*/

static int Computed_field_embedded_set_values_at_node(struct Computed_field *field,
	struct FE_node *node, FE_value time, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 28 October 2004

DESCRIPTION :
Sets the <values> of the computed <field> at <node>.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_embedded_set_values_at_node);
	USE_PARAMETER(time);
	if (field&&node&&values)
	{
		return_code=0;

		display_message(ERROR_MESSAGE,"Computed_field_embedded_set_values_at_node. "
			" %s not implemented yet. Write the code!",field->name);
		/* If we ever need to write the code for this, make it so that we set the array */
		/* values based upon the field->time ONLY if the time correspondes EXACTLY to an*/
		/* array index. No "taking the nearest one" or anything nasty like that */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_embedded_set_values_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_embedded_set_values_at_node */

#define Computed_field_embedded_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
==============================================================================*/
	
#define Computed_field_embedded_get_native_discretization_in_element \
   (Computed_field_get_native_discretization_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
==============================================================================*/

#define Computed_field_embedded_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
==============================================================================*/

static int list_Computed_field_embedded(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
==============================================================================*/
{
	char *field_name;
	int return_code;
	struct Computed_field_embedded_type_specific_data *data;

	ENTER(List_Computed_field_embedded);
	if (field && (data = 
		(struct Computed_field_embedded_type_specific_data *)
		field->type_specific_data))
	{
		if (return_code=GET_NAME(FE_field)(data->element_xi_fe_field, &field_name))
		{
			display_message(INFORMATION_MESSAGE,"    element_xi fe_field: %s\n",field_name);
			DEALLOCATE(field_name);
			display_message(INFORMATION_MESSAGE,
				"    field : %s\n",field->source_fields[0]->name);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_embedded.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_embedded */

static char *Computed_field_embedded_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;
	struct Computed_field_embedded_type_specific_data *data;

	ENTER(Computed_field_embedded_get_command_string);
	command_string = (char *)NULL;
	if (field && (data = 
		(struct Computed_field_embedded_type_specific_data *)
		field->type_specific_data))
	{
		error = 0;
		append_string(&command_string,
			computed_field_embedded_type_string, &error);
		append_string(&command_string, " element_xi ", &error);
		if (GET_NAME(FE_field)(data->element_xi_fe_field, &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
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
			"Computed_field_embedded_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_embedded_get_command_string */

#define Computed_field_embedded_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

static int Computed_field_set_type_embedded(struct Computed_field *field,
	struct FE_field *element_xi_fe_field, struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_EMBEDDED, wrapping the given
<fe_field>. Makes the number of components the same as in the <fe_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **source_fields;
	struct Computed_field_embedded_type_specific_data *data;

	ENTER(Computed_field_set_type_embedded);
	if (field && element_xi_fe_field && 
		(ELEMENT_XI_VALUE == get_FE_field_value_type(element_xi_fe_field)) &&
		source_field)
	{
		return_code=1;
		number_of_source_fields=1;
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *,
			number_of_source_fields) &&
			ALLOCATE(data,struct Computed_field_embedded_type_specific_data, 1))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_embedded_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			field->type_specific_data = (void *)data;
			data->element_xi_fe_field = ACCESS(FE_field)(element_xi_fe_field);
			data->fe_element_field_values = (struct FE_element_field_values *)NULL;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(embedded);
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
			"Computed_field_set_type_embedded.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_embedded */

static int Computed_field_get_type_embedded(struct Computed_field *field,
	struct FE_field **element_xi_fe_field, struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
If the field is of type COMPUTED_FIELD_EMBEDDED, the FE_field being
"wrapped" by it is returned - otherwise an error is reported.
==============================================================================*/
{
	int return_code;
	struct Computed_field_embedded_type_specific_data *data;

	ENTER(Computed_field_get_type_embedded);
	if (field&&(field->type_string==computed_field_embedded_type_string) 
		&& (data = (struct Computed_field_embedded_type_specific_data *)
		field->type_specific_data) && element_xi_fe_field &&source_field)
	{
		*element_xi_fe_field = data->element_xi_fe_field;
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_embedded.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_embedded */

static int define_Computed_field_type_embedded(struct Parse_state *state,
	void *field_void, void *computed_field_finite_element_package_void)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_EMBEDDED (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *source_field, *field;
	struct Computed_field_finite_element_package
		*computed_field_finite_element_package;
	struct FE_field *element_xi_fe_field;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(define_Computed_field_type_embedded);
	if (state && (field = (struct Computed_field *)field_void) &&
		(computed_field_finite_element_package =
			(struct Computed_field_finite_element_package *)
			computed_field_finite_element_package_void))
	{
		return_code = 1;
		element_xi_fe_field = (struct FE_field *)NULL;
		source_field = (struct Computed_field *)NULL;
		if (computed_field_embedded_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_embedded(field,
				&element_xi_fe_field, &source_field);
		}
		if (return_code)
		{
			if (element_xi_fe_field)
			{
				ACCESS(FE_field)(element_xi_fe_field);
			}
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			option_table = CREATE(Option_table)();
			/* element_xi FE_field */
			Option_table_add_set_FE_field_from_FE_region(option_table, "element_xi", 
				&element_xi_fe_field,
				computed_field_finite_element_package->fe_region);
			set_field_data.conditional_function =
				Computed_field_has_numerical_components;
			set_field_data.conditional_function_user_data = (void *)NULL;
			set_field_data.computed_field_manager =
				computed_field_finite_element_package->computed_field_manager;
			Option_table_add_entry(option_table, "field", &source_field,
				&set_field_data, set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				if (element_xi_fe_field && source_field)
				{
					return_code = Computed_field_set_type_embedded(field,
						element_xi_fe_field, source_field);
					Computed_field_set_coordinate_system_from_sources(field);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_embedded.  "
						"element_xi field or source field not specified");
					return_code = 0;
				}
			}
			if (element_xi_fe_field)
			{
				DEACCESS(FE_field)(&element_xi_fe_field);
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_embedded.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_embedded */

static int Computed_field_manager_update_wrapper(
	struct MANAGER(Computed_field) *computed_field_manager,
	struct Computed_field *wrapper, enum CHANGE_LOG_CHANGE(FE_field) change)
/*******************************************************************************
LAST MODIFIED : 2 May 2003

DESCRIPTION :
Searches the <computed_field_manager> for an existing field which matches
<wrapper> in name, or if not, in content.
If none found then <wrapper> is added to the manager.
If an existing wrapper is found then depending on state of <identifier_changed>
and <contents_changed>, it is modified appropriately to match the new <wrapper>.
???RC Review Manager Messages Here
==============================================================================*/
{
	int return_code;
	struct Computed_field *existing_wrapper;

	ENTER(Computed_field_manager_update_wrapper);
	if (computed_field_manager && wrapper)
	{
		return_code = 1;
		/* find existing wrapper first of same name, then matching contents */
		if (!(existing_wrapper = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
			wrapper->name, computed_field_manager)))
		{
			existing_wrapper = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_contents_match, (void *)wrapper, computed_field_manager);
		}
		if (existing_wrapper)
		{
			if (change & CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_field))
			{
				if (change & CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field))
				{
					return_code = MANAGER_MODIFY(Computed_field,name)(
						existing_wrapper, wrapper, computed_field_manager);
				}
				else
				{
					return_code = MANAGER_MODIFY_IDENTIFIER(Computed_field,name)(
						existing_wrapper, wrapper->name, computed_field_manager);
				}
			}
			else if (change & CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field))
			{
				return_code = MANAGER_MODIFY_NOT_IDENTIFIER(Computed_field,name)(
					existing_wrapper, wrapper, computed_field_manager);
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_manager_update_wrapper.  Unable to modify wrapper");
			}
		}
		else
		{
			if (!ADD_OBJECT_TO_MANAGER(Computed_field)(wrapper,
				computed_field_manager))
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_manager_update_wrapper.  Unable to add wrapper");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_manager_update_wrapper.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_manager_update_wrapper */

struct FE_field_to_Computed_field_change_data
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct FE_region_changes *changes;
	struct FE_region *fe_region;
};

static int FE_field_to_Computed_field_change(struct FE_field *fe_field,
	void *field_change_data_void)
/*******************************************************************************
LAST MODIFIED : 24 March 2003

DESCRIPTION :
From <fe_field>, creates one, and for certain types of fields more than one
Computed_field wrapper(s) for evaluating it in various ways, then calls
Computed_field_manager_update_wrapper to ensure that the new wrappers are
added to the manager, or the existing wrappers modified to match the new ones.
The computed_field_manager should be cached around several calls to this
function.
==============================================================================*/
{
	char *extra_field_name, *field_name;
	enum CHANGE_LOG_CHANGE(FE_field) change;
	enum Value_type value_type;
	int return_code;
	struct Computed_field *default_coordinate_field, *wrapper;
	struct FE_field_to_Computed_field_change_data *field_change_data;

	ENTER(FE_field_to_Computed_field_change);
	if (fe_field && (field_change_data =
		(struct FE_field_to_Computed_field_change_data *)field_change_data_void))
	{
		if (CHANGE_LOG_QUERY(FE_field)(field_change_data->changes->fe_field_changes,
			fe_field, &change))
		{
			return_code = 1;
			if (change != CHANGE_LOG_OBJECT_UNCHANGED(FE_field))
			{
				if (GET_NAME(FE_field)(fe_field, &field_name))
				{
					/* establish up-to-date wrapper for the fe_field */
					if (wrapper = CREATE(Computed_field)(field_name))
					{
						ACCESS(Computed_field)(wrapper);
						if (Computed_field_set_coordinate_system(wrapper,
							get_FE_field_coordinate_system(fe_field)) &&
							Computed_field_set_type_finite_element(wrapper, fe_field,
								field_change_data->fe_region) &&
							Computed_field_set_read_only(wrapper))
						{
							if (!Computed_field_manager_update_wrapper(
								field_change_data->computed_field_manager, wrapper,
								change))
							{
								return_code = 0;
							}
						}
						else
						{
							return_code = 0;
						}
						DEACCESS(Computed_field)(&wrapper);
					}
					else
					{
						return_code = 0;
					}

					/* For ELEMENT_XI_VALUE also make a default embedded coordinate field */
					value_type = get_FE_field_value_type(fe_field);
					switch(value_type)
					{
						case ELEMENT_XI_VALUE:
						{
							if (ALLOCATE(extra_field_name, char, strlen(field_name) + 15))
							{
								sprintf(extra_field_name, "%s_coordinate", field_name);
								/* establish up-to-date wrapper for the fe_field */
								if (wrapper = CREATE(Computed_field)(extra_field_name))
								{
									ACCESS(Computed_field)(wrapper);
									if ((default_coordinate_field =
										FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
											Computed_field_has_coordinate_fe_field,
											(void *)NULL, field_change_data->computed_field_manager)))
									{
										if (Computed_field_set_coordinate_system(wrapper,
											Computed_field_get_coordinate_system(
												default_coordinate_field)) &&
											Computed_field_set_type_embedded(wrapper, fe_field,
												default_coordinate_field) &&
											Computed_field_set_read_only(wrapper))
										{
											if (!Computed_field_manager_update_wrapper(
												field_change_data->computed_field_manager, wrapper,
												change))
											{
												return_code = 0;
											}
										}
										else
										{
											return_code = 0;
										}
									}
									else
									{
										return_code = 0;
									}
									DEACCESS(Computed_field)(&wrapper);
								}
								else
								{
									return_code = 0;
								}
								DEALLOCATE(extra_field_name);
							}
							else
							{
								return_code = 0;
							}
						} break;
					}
					DEALLOCATE(field_name);
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
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"FE_field_to_Computed_field_change.  Could not propagate change(s)");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_to_Computed_field_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_to_Computed_field_change */

static void Computed_field_FE_region_change(struct FE_region *fe_region,
	struct FE_region_changes *changes,
	void *computed_field_finite_element_package_void)
/*******************************************************************************
LAST MODIFIED : 3 June 2003

DESCRIPTION :
Updates definitions of Computed_field wrappers for changed FE_fields in the
manager by calling FE_field_to_Computed_field_change for every FE_field in the
changed_object_list. Caches computed_field_manager to consolidate messages
that may result from this process.
???RC Review Manager Messages Here
==============================================================================*/
{
	enum CHANGE_LOG_CHANGE(FE_field) change_summary;
	struct Computed_field_finite_element_package
		*computed_field_finite_element_package;
	struct FE_field_to_Computed_field_change_data field_change_data;

	ENTER(Computed_field_FE_region_change);
	if (changes && (computed_field_finite_element_package=
		(struct Computed_field_finite_element_package *)
		computed_field_finite_element_package_void) &&
		(fe_region == computed_field_finite_element_package->fe_region))
	{
		field_change_data.computed_field_manager = 
			computed_field_finite_element_package->computed_field_manager;
		field_change_data.changes = changes;
		field_change_data.fe_region = fe_region;
		CHANGE_LOG_GET_CHANGE_SUMMARY(FE_field)(changes->fe_field_changes,
			&change_summary);
		if ((change_summary & CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_field)) || 
			(change_summary & CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field)) || 
			(change_summary & CHANGE_LOG_OBJECT_ADDED(FE_field)))
		{
			/* have begin/end cache because more than one field may have been added
				 or modified */
			MANAGER_BEGIN_CACHE(Computed_field)(
				computed_field_finite_element_package->computed_field_manager);
			/* Ensure there is an updated Computed_field for each FE_field */
			FE_region_for_each_FE_field(field_change_data.fe_region,
				FE_field_to_Computed_field_change, (void *)&field_change_data);
			MANAGER_END_CACHE(Computed_field)(
				computed_field_finite_element_package->computed_field_manager);
		}
		if (change_summary & CHANGE_LOG_OBJECT_REMOVED(FE_field))
		{
			/* Currently we do nothing as the computed field wrapper is destroyed
				before the FE_field is removed from the manager.  This is not necessary
				and this response could be to delete the wrapper. */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_FE_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Computed_field_FE_region_change */

/*
Global functions
----------------
*/

struct Computed_field_finite_element_package *
Computed_field_register_types_finite_element(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *cmiss_region)
/*******************************************************************************
LAST MODIFIED : 21 March 2003

DESCRIPTION :
This function registers the finite_element related types of Computed_fields and
also attached to the <cmiss_region> so that any fe_fields are
automatically wrapped in corresponding computed_fields.
==============================================================================*/
{
	struct Computed_field_finite_element_package *return_ptr;
	struct FE_region *fe_region;
	static struct Computed_field_finite_element_package 
		computed_field_finite_element_package;

	ENTER(Computed_field_register_types_finite_element);
	if (computed_field_package && cmiss_region &&
		(fe_region = Cmiss_region_get_FE_region(cmiss_region)))
	{
		computed_field_finite_element_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(computed_field_package);
		computed_field_finite_element_package.cmiss_region =
			ACCESS(Cmiss_region)(cmiss_region);
		computed_field_finite_element_package.fe_region =
			ACCESS(FE_region)(fe_region);
		Computed_field_package_add_type(computed_field_package,
			computed_field_finite_element_type_string,
			define_Computed_field_type_finite_element,
			&computed_field_finite_element_package);
		Computed_field_package_add_type(computed_field_package,
			computed_field_cmiss_number_type_string,
			define_Computed_field_type_cmiss_number,
			&computed_field_finite_element_package);
#if defined (COMPUTED_FIELD_ACCESS_COUNT)
		Computed_field_package_add_type(computed_field_package,
			computed_field_access_count_type_string,
			define_Computed_field_type_access_count,
			&computed_field_finite_element_package);
#endif /* defined (COMPUTED_FIELD_ACCESS_COUNT) */
		Computed_field_package_add_type(computed_field_package,
			computed_field_xi_coordinates_type_string,
			define_Computed_field_type_xi_coordinates,
			&computed_field_finite_element_package);
		Computed_field_package_add_type(computed_field_package,
			computed_field_node_value_type_string,
			define_Computed_field_type_node_value,
			&computed_field_finite_element_package);
		Computed_field_package_add_type(computed_field_package,
			computed_field_embedded_type_string,
			define_Computed_field_type_embedded,
			&computed_field_finite_element_package);
		if (FE_region_add_callback(fe_region, Computed_field_FE_region_change, 
			(void *)&computed_field_finite_element_package))
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
LAST MODIFIED : 30 April 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_deregister_types_finite_element);
	if (computed_field_finite_element_package)
	{
		FE_region_remove_callback(computed_field_finite_element_package->fe_region, 
			Computed_field_FE_region_change, 
			(void *)computed_field_finite_element_package);
		DEACCESS(FE_region)(&(computed_field_finite_element_package->fe_region));
		DEACCESS(Cmiss_region)(
			&(computed_field_finite_element_package->cmiss_region));
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

int Computed_field_set_type_finite_element(struct Computed_field *field,
	struct FE_field *fe_field, struct FE_region *fe_region)
/*******************************************************************************
LAST MODIFIED : 11 March 2003

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_FINITE_ELEMENT, wrapping the given
<fe_field>. Makes the number of components the same as in the <fe_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
Need pointer to fe_field_manager so can call MANAGED_OBJECT_NOT_IN_USE in
Computed_field_finite_element_not_in_use.
==============================================================================*/
{
	char **component_names;
	int i, number_of_components, return_code;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_set_type_finite_element);
	if (field && fe_field && fe_region)
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
			field->type_string = computed_field_finite_element_type_string;
			field->number_of_components = number_of_components;
			field->component_names = component_names;
			field->type_specific_data = (void *)data;
			data->fe_field = ACCESS(FE_field)(fe_field);
			data->fe_element_field_values = (struct FE_element_field_values *)NULL;
			data->fe_region = fe_region;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(finite_element);
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
	if (field&&(field->type_string==computed_field_finite_element_type_string) 
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

int Computed_field_contains_changed_FE_field(
	struct Computed_field *field, void *fe_field_change_log_void)
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Returns true if <field> directly contains an FE_field and it is listed as
changed, added or removed in <fe_field_change_log>.
<fe_field_change_log_void> must point at a struct CHANGE_LOG<FE_field>.
==============================================================================*/
{
	enum CHANGE_LOG_CHANGE(FE_field) fe_field_change;
	enum FE_nodal_value_type nodal_value_type;
	int return_code, version_number;
	struct CHANGE_LOG(FE_field) *fe_field_change_log;
	struct Computed_field *source_field;
	struct FE_field *fe_field;

	ENTER(Computed_field_contains_changed_FE_field);
	if (field && (fe_field_change_log =
		(struct CHANGE_LOG(FE_field) *)fe_field_change_log_void))
	{
		if (field->type_string == computed_field_finite_element_type_string)
		{
			return_code = Computed_field_get_type_finite_element(field, &fe_field);
		}
		else if (field->type_string == computed_field_embedded_type_string)
		{
			return_code = Computed_field_get_type_embedded(field, &fe_field,
				&source_field);
		}
		else if (field->type_string == computed_field_node_value_type_string)
		{
			return_code = Computed_field_get_type_node_value(field, &fe_field,
				&nodal_value_type, &version_number);
		}
		else
		{
			return_code = 0;
		}
		if (return_code)
		{
			return_code = CHANGE_LOG_QUERY(FE_field)(fe_field_change_log, fe_field,
				&fe_field_change) &&
				(fe_field_change != CHANGE_LOG_OBJECT_UNCHANGED(FE_field));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_contains_changed_FE_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_contains_changed_FE_field */

static int Computed_field_add_source_FE_field_to_list(
	struct Computed_field *field, void *fe_field_list_void)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
If <field> has a source FE_field, ensures it is in <fe_field_list>.
==============================================================================*/
{
	enum FE_nodal_value_type nodal_value_type;
	int return_code, version_number;
	struct Computed_field *source_field;
	struct FE_field *fe_field;
	struct LIST(FE_field) *fe_field_list;

	ENTER(Computed_field_add_source_FE_field_to_list);
	if (field && (fe_field_list = (struct LIST(FE_field) *)fe_field_list_void))
	{
		fe_field = (struct FE_field *)NULL;
		if (field->type_string == computed_field_finite_element_type_string)
		{
			return_code = Computed_field_get_type_finite_element(field, &fe_field);
		}
		else if (field->type_string == computed_field_embedded_type_string)
		{
			return_code = Computed_field_get_type_embedded(field, &fe_field,
				&source_field);
		}
		else if (field->type_string == computed_field_node_value_type_string)
		{
			return_code = Computed_field_get_type_node_value(field, &fe_field,
				&nodal_value_type, &version_number);
		}
		else
		{
			return_code = 1;
		}
		if (return_code && fe_field)
		{
			if (!IS_OBJECT_IN_LIST(FE_field)(fe_field, fe_field_list))
			{
				return_code = ADD_OBJECT_TO_LIST(FE_field)(fe_field, fe_field_list);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_add_source_FE_field_to_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_add_source_FE_field_to_list */

struct LIST(FE_field)
	*Computed_field_get_defining_FE_field_list(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Returns the list of FE_fields that <field> depends on.
==============================================================================*/
{
	struct LIST(FE_field) *fe_field_list;

	ENTER(Computed_field_get_defining_FE_field_list);
	fe_field_list = (struct LIST(FE_field) *)NULL;
	if (field)
	{
		if (fe_field_list = CREATE(LIST(FE_field))())
		{
			if (!Computed_field_for_each_ancestor(field,
				Computed_field_add_source_FE_field_to_list, (void *)fe_field_list))
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_get_defining_FE_field_list.  Failed");
				DESTROY(LIST(FE_field))(&fe_field_list);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_defining_FE_field_list.  Invalid argument(s)");
	}
	LEAVE;

	return (fe_field_list);
} /* Computed_field_get_defining_FE_field_list */

int Computed_field_is_type_cmiss_number(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_cmiss_number);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_cmiss_number_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_cmiss_number.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_type_cmiss_number */

#define Computed_field_cmiss_number_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_cmiss_number(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CMISS_NUMBER with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_set_type_cmiss_number);
	if (field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		/* none */
		/* 2. free current type-specific data */
		Computed_field_clear_type(field);
		/* 3. establish the new type */
		field->type_string = computed_field_cmiss_number_type_string;
		field->number_of_components = 1;
		field->type_specific_data = (void *)1;
		
		/* Set all the methods */
		COMPUTED_FIELD_ESTABLISH_METHODS(cmiss_number);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_cmiss_number.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_cmiss_number */

int Computed_field_is_read_only_with_fe_field(
	struct Computed_field *field,void *fe_field_void)
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is read only and a
wrapper for <fe_field>.
???RC This looks ready for an overhaul since other fields in this module can
contain FE_fields.
???RC Note however that this function is used to find the finite_element
wrapper for a given FE_field, so must make sure this still works!
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

int Computed_field_has_coordinate_fe_field(struct Computed_field *field,
	void *dummy)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Iterator/conditional function returning true if <field> is a wrapper for a
coordinate type fe_field.
==============================================================================*/
{
	int return_code;
	struct Computed_field_finite_element_type_specific_data *data;

	ENTER(Computed_field_has_coordinate_fe_field);
	USE_PARAMETER(dummy);
	if (field)
	{
		return_code = 0;
		if (field->type_string == computed_field_finite_element_type_string)
		{
			data = (struct Computed_field_finite_element_type_specific_data *)
				field->type_specific_data;
			if (data->fe_field && FE_field_is_coordinate_field(data->fe_field, NULL))
			{
				return_code = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_coordinate_fe_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_has_coordinate_fe_field */

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

int Computed_field_has_string_value_type(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2002

DESCRIPTION :
Returns true if <field> is of string value type.
Currently only FE_fields can return string value type, hence this function is
restricted to this module.
Eventually, other computed fields will be of string type and this function will
belong elsewhere.
Note: not returning possible true result for embedded field as evaluate
at node function does not allow for string value either.
==============================================================================*/
{
	enum FE_nodal_value_type nodal_value_type;
	int return_code;
	int version_number;
	struct FE_field *fe_field;

	ENTER(Computed_field_has_string_value_type);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		fe_field = (struct FE_field *)NULL;
		if (field->type_string == computed_field_finite_element_type_string)
		{
			Computed_field_get_type_finite_element(field, &fe_field);
		}
		else if (field->type_string == computed_field_node_value_type_string)
		{
			Computed_field_get_type_node_value(field, &fe_field,
				&nodal_value_type, &version_number);
		}
		return_code = ((struct FE_field *)NULL != fe_field) &&
			(STRING_VALUE == get_FE_field_value_type(fe_field));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_string_value_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_has_string_value_type */

int Computed_field_depends_on_embedded_field(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns true if the field is of an embedded type or depends on any computed
fields which are or an embedded type.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_depends_on_embedded_field);
	if (field)
	{
		if (Computed_field_is_type_embedded(field, NULL))
		{
			return_code = 1;
		}
		else
		{
			return_code=0;
			for (i=0;(i<field->number_of_source_fields)&&(!return_code);i++)
			{
				return_code=Computed_field_depends_on_embedded_field(
					field->source_fields[i]);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_depends_on_embedded_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_depends_on_embedded_field */

int Computed_field_manager_destroy_FE_field(
	struct MANAGER(Computed_field) *computed_field_manager,
	struct FE_field *fe_field)
/*******************************************************************************
LAST MODIFIED : 13 May 2003

DESCRIPTION :
Cleans up <fe_field> and its Computed_field wrapper if each are not in use.
==============================================================================*/
{
	int return_code;
	struct Computed_field *computed_field;
	struct FE_region *fe_region;

	ENTER(Computed_field_manager_destroy_FE_field);
	if (computed_field_manager && fe_field &&
		(fe_region = FE_field_get_FE_region(fe_field)))
	{
		return_code = 1;
		if (FE_region_is_FE_field_in_use(fe_region, fe_field))
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_manager_destroy_FE_field.  "
				"FE_field %s (%p) is in use (accessed %d times)",
				get_FE_field_name(fe_field), fe_field, get_FE_field_access_count(fe_field));
			return_code = 0;
		}
		else
		{
			/* Computed_field wrapper does not need to exist to remove FE_field */
			if (computed_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_is_read_only_with_fe_field, (void *)(fe_field),
				computed_field_manager))
			{
				if (MANAGED_OBJECT_NOT_IN_USE(Computed_field)(computed_field,
					computed_field_manager))
				{
					if (!REMOVE_OBJECT_FROM_MANAGER(Computed_field)(
						computed_field, computed_field_manager))
					{
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_manager_destroy_FE_field.  "
						"Computed_field %s is in use (accessed %d times)",
						computed_field->name, computed_field->access_count);
					return_code = 0;
				}
			}
			if (return_code)
			{
				return_code = FE_region_remove_FE_field(fe_region, fe_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_manager_destroy_FE_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return(return_code);
} /* Computed_field_manager_destroy_FE_field */

#define Computed_field_xi_coordinates_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_xi_coordinates(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_XI_COORDINATES with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_set_type_xi_coordinates);
	if (field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		/* none */
		/* 2. free current type-specific data */
		Computed_field_clear_type(field);
		/* 3. establish the new type */
		field->type_string = computed_field_xi_coordinates_type_string;
		field->number_of_components = 3;
		field->type_specific_data = (void *)1;
		
		/* Set all the methods */
		COMPUTED_FIELD_ESTABLISH_METHODS(xi_coordinates);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_xi_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_xi_coordinates */
