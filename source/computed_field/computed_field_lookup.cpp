/*******************************************************************************
FILE : computed_field_lookup.c

LAST MODIFIED : 10 March 2004

DESCRIPTION :
Defines fields for looking up values at given locations.
==============================================================================*/
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "finite_element/finite_element_region.h"
#include "region/cmiss_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_lookup.h"

struct Computed_field_lookup_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Cmiss_region *root_region;
};

struct Computed_field_nodal_lookup_type_specific_data
{
  struct Cmiss_region *nodal_lookup_region;
  int nodal_lookup_node_identifier;
	struct FE_node *nodal_lookup_node;
};

static char computed_field_nodal_lookup_type_string[] = "nodal_lookup";

int Computed_field_is_type_nodal_lookup(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 29 September 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_nodal_lookup);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_nodal_lookup_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_nodal_lookup.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_nodal_lookup */

static int Computed_field_nodal_lookup_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 October 2003

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_nodal_lookup_type_specific_data *data;

	ENTER(Computed_field_nodal_lookup_clear_type_specific);
	if (field && (data = 
		(struct Computed_field_nodal_lookup_type_specific_data *)
		field->type_specific_data))
	{
		if (data->nodal_lookup_region)
		{
			DEACCESS(Cmiss_region)(&(data->nodal_lookup_region));
		}
		if (data->nodal_lookup_node)
		{
			DEACCESS(FE_node)(&(data->nodal_lookup_node));
		}
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_nodal_lookup_clear_type_specific.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_nodal_lookup_clear_type_specific */

static void *Computed_field_nodal_lookup_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 10 October 2003

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_nodal_lookup_type_specific_data *destination, *source;

	ENTER(Computed_field_nodal_lookup_copy_type_specific);
	if (source_field && destination_field && (source = 
		(struct Computed_field_nodal_lookup_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_nodal_lookup_type_specific_data, 1))
		{
			destination->nodal_lookup_region = ACCESS(Cmiss_region)(source->nodal_lookup_region);
			destination->nodal_lookup_node_identifier = source->nodal_lookup_node_identifier;
			destination->nodal_lookup_node = ACCESS(FE_node)(source->nodal_lookup_node);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_nodal_lookup_copy_type_specific.  "
				"Unable to allocate memory");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_nodal_lookup_copy_type_specific.  Invalid argument(s)");
		destination = NULL;
	}
	LEAVE;

	return ((void *)destination);
} /* Computed_field_compose_copy_type_specific */

#define Computed_field_nodal_lookup_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 29 September 2003

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_nodal_lookup_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 10 October 2003

DESCRIPTION :
Compare the type specific data.
==============================================================================*/
{
	int return_code;
	struct Computed_field_nodal_lookup_type_specific_data *data, *other_data;

	ENTER(Computed_field_nodal_lookup_type_specific_contents_match);
	if (field && other_computed_field && (data = 
		(struct Computed_field_nodal_lookup_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_nodal_lookup_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		if ((data->nodal_lookup_region == other_data->nodal_lookup_region) &&
      (data->nodal_lookup_node_identifier == other_data->nodal_lookup_node_identifier))
		{
			return_code = 1;
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
	LEAVE;

	return (return_code);
} /* Computed_field_nodal_lookup_type_specific_contents_match */

int Computed_field_nodal_lookup_is_defined_in_element(struct Computed_field *field,
	struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 10 March 2004

DESCRIPTION :
Returns true if <field> can be calculated in <element>, which is determined
by checking the source field on the lookup node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_nodal_lookup_type_specific_data *data;

	ENTER(Computed_field_nodal_lookup_is_defined_in_element);
	if (field && element && (data = 
		(struct Computed_field_nodal_lookup_type_specific_data *)
		field->type_specific_data))
	{
		return_code = Computed_field_is_defined_at_node(field->source_fields[0],
			data->nodal_lookup_node);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_nodal_lookup_is_defined_in_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_nodal_lookup_is_defined_in_element */

int Computed_field_nodal_lookup_is_defined_at_node(struct Computed_field *field,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 10 March 2004

DESCRIPTION :
Checks whether the lookup source field is defined on the lookup node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_nodal_lookup_type_specific_data *data;

	ENTER(Computed_field_nodal_lookup_is_defined_at_node);
	if (field && node && (data = 
		(struct Computed_field_nodal_lookup_type_specific_data *)
		field->type_specific_data))
	{
		return_code = Computed_field_is_defined_at_node(field->source_fields[0],
			data->nodal_lookup_node);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_nodal_lookup_is_defined_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_nodal_lookup_is_defined_at_node */

#define Computed_field_nodal_lookup_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 29 September 2003

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_nodal_lookup_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 29 September 2003

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_nodal_lookup_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 29 September 2003

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code,i;
	struct Computed_field_nodal_lookup_type_specific_data *data;

	ENTER(Computed_field_nodal_lookup_evaluate_cache_at_node);
	USE_PARAMETER(node);
	if (field && (data = 
		(struct Computed_field_nodal_lookup_type_specific_data *)
		field->type_specific_data))
	{
		/* 1. Precalculate any source fields that this field depends on */
		/* only calculate the time_field at this time and then use the
		   results of that field for the time of the source_field */
		if (return_code=Computed_field_evaluate_cache_at_node(
          field->source_fields[0],data->nodal_lookup_node,time))
		{
			/* 2. Copy the results */
			for (i=0;i<field->number_of_components;i++)
			{
				field->values[i] = field->source_fields[0]->values[i];
			}
		}
    return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_nodal_lookup_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_nodal_lookup_evaluate_cache_at_node */

static int Computed_field_nodal_lookup_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 01 October 2003

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code,i,element_dimension;
	struct Computed_field_nodal_lookup_type_specific_data *data;

	ENTER(Computed_field_nodal_lookup_evaluate_cache_in_element);
	USE_PARAMETER(xi);
	USE_PARAMETER(top_level_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && (data = 
		(struct Computed_field_nodal_lookup_type_specific_data *)
		field->type_specific_data))
	{
		/* 1. Precalculate any source fields that this field depends on */
		/* only calculate the time_field at this time and then use the
		   results of that field for the time of the source_field */
		if (return_code=Computed_field_evaluate_cache_at_node(
          field->source_fields[0],data->nodal_lookup_node,time))
		{
			/* 2. Copy the results */
			for (i=0;i<field->number_of_components;i++)
			{
				field->values[i] = field->source_fields[0]->values[i];
			}
      /* 3. Set all derivatives to 0 */
      element_dimension = get_FE_element_dimension(element);
      for (i=0;i<(field->number_of_components*element_dimension);i++)
      {
        field->derivatives[i] = (FE_value)0.0;
      }
      field->derivatives_valid = 1;
		}
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

#define Computed_field_nodal_lookup_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 29 September 2003

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_nodal_lookup_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 29 September 2003

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_nodal_lookup_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 29 September 2003

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_nodal_lookup_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 29 September 2003

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_nodal_lookup_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 29 September 2003

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_nodal_lookup_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 29 September 2003

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_nodal_lookup(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 29 September 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_nodal_lookup_type_specific_data *data;

	ENTER(List_Computed_field_time_nodal_lookup);
	if (field && (data = 
        (struct Computed_field_nodal_lookup_type_specific_data *)
        field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE,
			"    field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    node : %d\n",
			get_FE_node_identifier(data->nodal_lookup_node));
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_nodal_lookup.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_nodal_lookup */

static char *Computed_field_nodal_lookup_get_command_string(
  struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 29 September 2003

DESCRIPTION :
Returns allocated command string for reproducing field.
==============================================================================*/
{
	char *command_string, *field_name,node_id[10];
	int error,node_number;
	struct Computed_field_nodal_lookup_type_specific_data *data;

	ENTER(Computed_field_time_nodal_lookup_get_command_string);
	command_string = (char *)NULL;
	if (field && (data = 
        (struct Computed_field_nodal_lookup_type_specific_data *)
        field->type_specific_data))
	{
		error = 0;
		append_string(&command_string,
			computed_field_nodal_lookup_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " node ", &error);
    node_number = get_FE_node_identifier(data->nodal_lookup_node);
    sprintf(node_id,"%d",node_number);
    append_string(&command_string, " ", &error);
    append_string(&command_string, node_id, &error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_nodal_lookup_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_nodal_lookup_get_command_string */

int Computed_field_nodal_lookup_has_multiple_times (struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 29 September 2003

DESCRIPTION :
Always has multiple times.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_nodal_lookup_has_multiple_times);
	if (field)
	{
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_nodal_lookup_has_multiple_times.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_time_value_has_multiple_times */

int Computed_field_set_type_nodal_lookup(struct Computed_field *field,
	struct Computed_field *source_field,struct Cmiss_region *region,
  int nodal_lookup_node_identifier)
/*******************************************************************************
LAST MODIFIED : 29 September 2003

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_NODAL_LOOKUP with the supplied
fields, <source_field> is the field the values are returned from but rather
than using the current node the <nodal_lookup_node_identifier> node is used.
==============================================================================*/
{
	int number_of_source_fields,number_of_source_values,return_code;
	struct Computed_field **source_fields;
	struct Computed_field_nodal_lookup_type_specific_data *data;
  struct FE_region *fe_region;

	ENTER(Computed_field_set_type_time_nodal_lookup);
	if (field && source_field && region)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		number_of_source_values=0;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields)
      && ALLOCATE(data,struct Computed_field_nodal_lookup_type_specific_data,1))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_nodal_lookup_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->source_values=(FE_value *)NULL;
			field->number_of_source_values=number_of_source_values;
			field->type_specific_data = (void *)data;

      if (fe_region = Cmiss_region_get_FE_region(region))
      {
        data->nodal_lookup_region=ACCESS(Cmiss_region)(region);
        data->nodal_lookup_node=
          ACCESS(FE_node)(FE_region_get_FE_node_from_identifier(fe_region,
                            nodal_lookup_node_identifier));
        data->nodal_lookup_node_identifier=nodal_lookup_node_identifier;
      }
      else
      {
        data->nodal_lookup_region = (struct Cmiss_region *)NULL;
        data->nodal_lookup_node = (struct FE_node *)NULL;
        data->nodal_lookup_node_identifier = 0;
      }

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(nodal_lookup);
		}
		else
		{
			DEALLOCATE(source_fields);
			DEALLOCATE(data);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_nodal_lookup.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_nodal_lookup */

int Computed_field_get_type_nodal_lookup(struct Computed_field *field,
  struct Computed_field **nodal_lookup_field,struct Cmiss_region **nodal_lookup_region,
  int *nodal_lookup_node_identifier)
/*******************************************************************************
LAST MODIFIED : 10 October 2003

DESCRIPTION :
If the field is of type COMPUTED_FIELD_NODAL_LOOKUP, the function returns the source
<nodal_lookup_field>, <nodal_lookup_region>, and the <nodal_lookup_node_identifier>.
Note that nothing returned has been ACCESSed.
==============================================================================*/
{
  int return_code = 0;
  struct Computed_field_nodal_lookup_type_specific_data *data;

	ENTER(Computed_field_get_type_nodal_lookup);
	if (field && (field->type_string == computed_field_nodal_lookup_type_string) &&
		(data = (struct Computed_field_nodal_lookup_type_specific_data *)
		field->type_specific_data))
	{
    *nodal_lookup_field = field->source_fields[0];
    *nodal_lookup_region = data->nodal_lookup_region;
    *nodal_lookup_node_identifier = data->nodal_lookup_node_identifier;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_nodal_lookup.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_nodal_lookup */

static int define_Computed_field_type_nodal_lookup(struct Parse_state *state,
	void *field_void,void *computed_field_lookup_package_void)
/*******************************************************************************
LAST MODIFIED : 01 October 2003

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_NODAL_LOOKUP (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_lookup_package *computed_field_lookup_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;
  int node_identifier;
  char node_flag;
  struct Cmiss_region *dummy_region;

	ENTER(define_Computed_field_type_nodal_lookup);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_lookup_package=
		(struct Computed_field_lookup_package *)
		computed_field_lookup_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			node_flag = 0;
			node_identifier = 0;
			if (computed_field_nodal_lookup_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_nodal_lookup(field, 
					source_fields,&dummy_region,&node_identifier);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}

				option_table = CREATE(Option_table)();
				/* source field */
				set_source_field_data.computed_field_manager=
					computed_field_lookup_package->computed_field_manager;
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				Option_table_add_entry(option_table,"field",&source_fields[0],
					&set_source_field_data,set_Computed_field_conditional);
				/* the node to nodal_lookup */
				Option_table_add_entry(option_table, "node", &node_identifier,
					&node_flag, set_int_and_char_flag);
				/* process the option table */
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code && node_flag)
				{
					return_code = Computed_field_set_type_nodal_lookup(field,
						source_fields[0],computed_field_lookup_package->root_region,
						node_identifier);
				}
				else
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_time_nodal_lookup.  Failed");
					}
				}
				if (source_fields[0])
				{
					DEACCESS(Computed_field)(&source_fields[0]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_nodal_lookup.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_nodal_lookup.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_nodal_lookup */

int Computed_field_register_types_lookup(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region)
/*******************************************************************************
LAST MODIFIED : 01 October 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_lookup_package 
		computed_field_lookup_package;

	ENTER(Computed_field_register_types_lookup);
	if (computed_field_package)
	{
		computed_field_lookup_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_lookup_package.root_region = root_region;
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_nodal_lookup_type_string,
			define_Computed_field_type_nodal_lookup,
			&computed_field_lookup_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_nodal_lookup.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_lookup */

