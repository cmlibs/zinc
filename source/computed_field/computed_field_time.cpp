/*******************************************************************************
FILE : computed_field_time.c

LAST MODIFIED : 01 October 2003

DESCRIPTION :
Implements a number of basic component wise operations on computed fields.
==============================================================================*/
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "time/time.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_time.h"

struct Computed_field_time_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Time_keeper *time_keeper;
};

static char computed_field_time_lookup_type_string[] = "time_lookup";

int Computed_field_is_type_time_lookup(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_time_lookup);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_time_lookup_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_time_lookup.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_time_lookup */

#define Computed_field_time_lookup_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_time_lookup_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_time_lookup_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_time_lookup_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_time_lookup_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_time_lookup_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_time_lookup_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_time_lookup_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_time_lookup_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_time_lookup_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 2) && 
		(field->number_of_components == field->source_fields[0]->number_of_components) && 
		(1 == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		/* only calculate the time_field at this time and then use the
		   results of that field for the time of the source_field */
		if ((return_code=Computed_field_evaluate_cache_at_node(
			field->source_fields[1],node,time)) &&
			(return_code=Computed_field_evaluate_cache_at_node(
			field->source_fields[0],node,field->source_fields[1]->values[0])))
		{
			/* 2. Copy the results */
			for (i=0;i<field->number_of_components;i++)
			{
				field->values[i] = field->source_fields[0]->values[i];
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_time_lookup_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_time_lookup_evaluate_cache_at_node */

static int Computed_field_time_lookup_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value *temp, *temp1;
	int element_dimension, i, return_code;

	ENTER(Computed_field_time_lookup_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 2) && 
		(field->number_of_components == field->source_fields[0]->number_of_components) && 
		(1 == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		element_dimension=get_FE_element_dimension(element);
		if ((return_code=Computed_field_evaluate_cache_in_element(
			field->source_fields[1],element,xi,time,top_level_element,
			/*calculate_derivatives*/0)) &&
			(return_code=Computed_field_evaluate_cache_in_element(
			field->source_fields[0],element,xi,field->source_fields[1]->values[0],
			top_level_element,calculate_derivatives)))
		{
			/* 2. Copy the results */
			for (i=0;i<field->number_of_components;i++)
			{
				field->values[i] = field->source_fields[0]->values[i];
			}
			if (calculate_derivatives && field->source_fields[0]->derivatives_valid)
			{
				temp=field->derivatives;
				temp1=field->source_fields[0]->derivatives;
				for (i=(field->number_of_components*element_dimension);
					  0<i;i--)
				{
					(*temp)=(*temp1);
					temp++;
					temp1++;
				}
				field->derivatives_valid = 1;
			}
			else
			{
				field->derivatives_valid = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_time_lookup_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_time_lookup_evaluate_cache_in_element */

#define Computed_field_time_lookup_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_time_lookup_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_time_lookup_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_time_lookup_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_time_lookup_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_time_lookup_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_time_lookup(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_time_lookup);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    time field : %s\n",
			field->source_fields[1]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_time_lookup.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_time_lookup */

static char *Computed_field_time_lookup_get_command_string(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_time_lookup_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_time_lookup_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " time_field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, " ", &error);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_time_lookup_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_time_lookup_get_command_string */

int Computed_field_time_lookup_has_multiple_times (struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Returns 1 if the time_field source_field has multiple times.  The times of the
actual source field are controlled by this time field and so changes in the
global time do not matter.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_default_has_multiple_times);
	if (field && (2 == field->number_of_source_fields))
	{
		return_code=0;
		if (Computed_field_has_multiple_times(field->source_fields[1]))
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_time_lookup_has_multiple_times.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_time_lookup_has_multiple_times */

int Computed_field_set_type_time_lookup(struct Computed_field *field,
	struct Computed_field *source_field, struct Computed_field *time_field)
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_TIME_LOOKUP with the supplied
fields, <source_field> is the field the values are returned from but rather
than using the current time, the scalar <time_field> is evaluated and its value
is used for the time.
==============================================================================*/
{
	int number_of_source_fields,number_of_source_values,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_time_lookup);
	if (field&&source_field&&time_field&&(1 == time_field->number_of_components))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		number_of_source_values=0;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_time_lookup_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			source_fields[1]=ACCESS(Computed_field)(time_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->source_values=(FE_value *)NULL;
			field->number_of_source_values=number_of_source_values;
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(time_lookup);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_time_lookup.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_time_lookup */

int Computed_field_get_type_time_lookup(struct Computed_field *field,
	struct Computed_field **source_field, struct Computed_field **time_field)
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
If the field is of type COMPUTED_FIELD_TIME_LOOKUP, the 
<source_field> and <time_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_time_lookup);
	if (field&&(field->type_string==computed_field_time_lookup_type_string))
	{
		*source_field = field->source_fields[0];
		*time_field = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_time_lookup.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_time_lookup */

static int define_Computed_field_type_time_lookup(struct Parse_state *state,
	void *field_void,void *computed_field_time_package_void)
/*******************************************************************************
LAST MODIFIED : 16 December 2002

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_TIME_LOOKUP (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_time_package 
		*computed_field_time_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data,
		set_time_field_data;

	ENTER(define_Computed_field_type_time_lookup);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_time_package=
		(struct Computed_field_time_package *)
		computed_field_time_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_time_lookup_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_time_lookup(field, 
					source_fields, source_fields + 1);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}
				if (source_fields[1])
				{
					ACCESS(Computed_field)(source_fields[1]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					computed_field_time_package->computed_field_manager;
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				Option_table_add_entry(option_table,"field",&source_fields[0],
					&set_source_field_data,set_Computed_field_conditional);
				set_time_field_data.computed_field_manager=
					computed_field_time_package->computed_field_manager;
				set_time_field_data.conditional_function=Computed_field_is_scalar;
				set_time_field_data.conditional_function_user_data=(void *)NULL;
				Option_table_add_entry(option_table,"time_field",&source_fields[1],
					&set_time_field_data,set_Computed_field_conditional);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_time_lookup(field,
						source_fields[0], source_fields[1]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_time_lookup.  Failed");
					}
				}
				if (source_fields[0])
				{
					DEACCESS(Computed_field)(&source_fields[0]);
				}
				if (source_fields[1])
				{
					DEACCESS(Computed_field)(&source_fields[1]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_time_lookup.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_time_lookup.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_time_lookup */

struct Computed_field_time_value_type_specific_data
{
	struct Time_object *time_object;
};

static char computed_field_time_value_type_string[] = "time_value";

int Computed_field_is_type_time_value(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_time_value);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_time_value_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_time_value.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_time_value */

#define Computed_field_time_value_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_time_value_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_time_value_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_time_value_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_time_value_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_time_value_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_time_value_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_time_value_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_time_value_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_time_value_type_specific_data *data;

	ENTER(Computed_field_time_value_evaluate_cache_at_node);
	USE_PARAMETER(node);
	USE_PARAMETER(time);
	if (field && (data = 
		(struct Computed_field_time_value_type_specific_data *)
		field->type_specific_data))
	{
		field->values[0] = Time_object_get_current_time(data->time_object);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_time_value_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_time_value_evaluate_cache_at_node */

static int Computed_field_time_value_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_time_value_type_specific_data *data;

	ENTER(Computed_field_time_value_evaluate_cache_in_element);
	USE_PARAMETER(element);
	USE_PARAMETER(xi);
	USE_PARAMETER(time);
	USE_PARAMETER(top_level_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && (data = 
		(struct Computed_field_time_value_type_specific_data *)
		field->type_specific_data))
	{
		field->values[0] = Time_object_get_current_time(data->time_object);
		field->derivatives_valid = 0;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_time_value_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_time_value_evaluate_cache_in_element */

#define Computed_field_time_value_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_time_value_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_time_value_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_time_value_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_time_value_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_time_value_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_time_value(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_time_value);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_time_value.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_time_value */

static char *Computed_field_time_value_get_command_string(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string;
	int error;

	ENTER(Computed_field_time_value_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_time_value_type_string, &error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_time_value_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_time_value_get_command_string */

int Computed_field_time_value_has_multiple_times (struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
Always has multiple times.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_time_value_has_multiple_times);
	if (field)
	{
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_time_value_has_multiple_times.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_time_value_has_multiple_times */

int Computed_field_set_type_time_value(struct Computed_field *field,
	struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_TIME_VALUE.  It always returns the time
from the default time keeper.
==============================================================================*/
{
	int return_code;
	struct Computed_field_time_value_type_specific_data *data;
	struct Time_object *time_object;

	ENTER(Computed_field_set_type_time_value);
	if (field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		if ((time_object = CREATE(Time_object)(field->name)) &&
			ALLOCATE(data, struct Computed_field_time_value_type_specific_data, 1))
		{
			Time_object_set_time_keeper(time_object, time_keeper);
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_time_value_type_string;
			field->number_of_components = 1;
			data->time_object = time_object;
			field->type_specific_data = data;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(time_value);
		}
		else
		{
			DESTROY(Time_object)(&time_object);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_time_value.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_time_value */

/* Computed_field_get_type_time_value(struct Computed_field *field) */
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
There are no fields to fetch from a time value field.
==============================================================================*/

static int define_Computed_field_type_time_value(struct Parse_state *state,
	void *field_void,void *computed_field_time_package_void)
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_TIME_VALUE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field;
	struct Computed_field_time_package *computed_field_time_package;

	ENTER(define_Computed_field_type_time_value);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_time_package=
		(struct Computed_field_time_package *)
		computed_field_time_package_void))
	{
    if ((!(state->current_token)) ||
		(strcmp(PARSER_HELP_STRING,state->current_token)&&
		strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
		{
			return_code = Computed_field_set_type_time_value(field,
				computed_field_time_package->time_keeper);
		}
		else
		{
			/* Help */
			return_code = 0;
		}
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_time_value */

int Computed_field_register_types_time(
	struct Computed_field_package *computed_field_package,
	struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 19 September 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_time_package 
		computed_field_time_package;

	ENTER(Computed_field_register_types_time);
	if (computed_field_package)
	{
		computed_field_time_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_time_package.time_keeper =
			time_keeper;
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_time_lookup_type_string,
			define_Computed_field_type_time_lookup,
			&computed_field_time_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_time_value_type_string,
			define_Computed_field_type_time_value,
			&computed_field_time_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_time.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_time */

