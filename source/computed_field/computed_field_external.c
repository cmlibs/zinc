/*******************************************************************************
FILE : computed_field_external.c

LAST MODIFIED : 23 January 2002

DESCRIPTION :
Implements a computed_field that uses evaluates one field, does a
"find element_xi" look up on a field in a host element group to find the same 
values and then evaluates a third field at that location.
Essentially it is used to embed one mesh in the elements of another.
==============================================================================*/
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "general/child_process.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_external.h"

struct Computed_field_external_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
};

struct Computed_field_external_type_specific_data
{
	char *child_filename;
	int timeout;
	struct Child_process *child_process;
};

static char computed_field_external_type_string[] = "external";

int Computed_field_is_type_external(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_external);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_external_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_external.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_type_external */

static int Computed_field_external_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_external_type_specific_data *data;

	ENTER(Computed_field_external_clear_type_specific);
	if (field && (data = 
		(struct Computed_field_external_type_specific_data *)
		field->type_specific_data))
	{
		if (data->child_filename)
		{
			DEALLOCATE(data->child_filename);
		}
		if (data->child_process)
		{
			DEACCESS(Child_process)(&(data->child_process));
		}
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_external_clear_type_specific.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_external_clear_type_specific */

static void *Computed_field_external_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_external_type_specific_data *destination, *source;

	ENTER(Computed_field_external_copy_type_specific);
	if (field && (source = 
		(struct Computed_field_external_type_specific_data *)
		field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_external_type_specific_data, 1) &&
			ALLOCATE(destination->child_filename, char,
				strlen(source->child_filename) + 1))
		{
			strcpy(destination->child_filename, source->child_filename);
			destination->timeout = source->timeout;
			if (source->child_process)
			{
				destination->child_process = 
					ACCESS(Child_process)(source->child_process);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_external_copy_type_specific.  "
				"Unable to allocate memory");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_external_copy_type_specific.  Invalid argument(s)");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_external_copy_type_specific */

#define Computed_field_external_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_external_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Compare the type specific data.
==============================================================================*/
{
	int return_code;
	struct Computed_field_external_type_specific_data *data, *other_data;

	ENTER(Computed_field_external_type_specific_contents_match);
	if (field && other_computed_field && (data = 
		(struct Computed_field_external_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_external_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		if ((!strcmp(data->child_filename, other_data->child_filename))
			 && (data->timeout == other_data->timeout)
			 && (data->child_process == other_data->child_process))
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
} /* Computed_field_external_type_specific_contents_match */

#define Computed_field_external_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_external_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_external_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
==============================================================================*/

#define Computed_field_external_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_external_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	char *temp_string;
	int i, j, k, return_code, total_values;
	struct Computed_field_external_type_specific_data *data;

	ENTER(Computed_field_external_evaluate_cache_at_node);
	if (field && node && (data = 
		(struct Computed_field_external_type_specific_data *)
		field->type_specific_data))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			total_values = field->number_of_source_values;
			for (i = 0 ; i < field->number_of_source_fields ; i++)
			{
				total_values+=field->source_fields[i]->number_of_components;
			}
			if (ALLOCATE(temp_string, char, total_values * 12 + 2))
			{
				k = 0;
				for (i = 0 ; i < field->number_of_source_values ; i++)
				{
					sprintf(temp_string + k, "%10.5e ", field->source_values[i]);
					k += 12;
				}
				for (i = 0 ; i < field->number_of_source_fields ; i++)
				{
					for (j = 0 ; j < field->source_fields[i]->number_of_components ; j++)
					{
						sprintf(temp_string + k, "%10.5e ", field->source_fields[i]->values[j]);
						k += 12;
					}
				}
				sprintf(temp_string + k, "\n");
				Child_process_send_string_to_stdin(data->child_process,temp_string);
				DEALLOCATE(temp_string);
				if (temp_string = Child_process_get_line_from_stdout(data->child_process,
						 data->timeout))
				{
					k = 0;
					for (i = 0 ; i < field->number_of_components ; i++)
					{
						sscanf(temp_string + k, "%f%n", &(field->values[i]), &j);
						k += j;
					}
					DEALLOCATE(temp_string);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_evaluate_cache_at_node."
						"  Invalid response from child process");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_evaluate_cache_at_node."
					"  Unable to allocate temporary string");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_external_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_external_evaluate_cache_at_node */

static int Computed_field_external_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	char buffer[100], *temp_string;
	int element_dimension, i, index, j, k, return_code, total_values;
	struct Computed_field_external_type_specific_data *data;

	ENTER(Computed_field_external_evaluate_cache_in_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi && (data = 
		(struct Computed_field_external_type_specific_data *)
		field->type_specific_data))
	{
		/* 1. Precalculate any source fields that this field depends on */
		/* only calculate the first source_field at this location */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			total_values = field->number_of_source_values * 12 + 2;
			for (i = 0 ; i < field->number_of_source_fields ; i++)
			{
				total_values+=field->source_fields[i]->number_of_components * 12;
				if (calculate_derivatives /* and not never || always give derivatives */)
				{
					total_values+=field->source_fields[i]->number_of_components *
						(element_dimension * 12 + 20);
				}
			}
			if (ALLOCATE(temp_string, char, total_values))
			{
				index = 0;
				for (i = 0 ; i < field->number_of_source_values ; i++)
				{
					sprintf(temp_string + index, "%10.5e ", field->source_values[i]);
					index += 12;
				}
				for (i = 0 ; i < field->number_of_source_fields ; i++)
				{
					for (j = 0 ; j < field->source_fields[i]->number_of_components ; j++)
					{
						sprintf(temp_string + index, "%10.5e ", field->source_fields[i]->values[j]);
						index += 12;
						if (calculate_derivatives /* and not never || always give derivatives */)
						{
							sprintf(temp_string + index, "#deriv=%d %n", element_dimension,
								&k);
							index += k;
							for (k = 0 ; k < element_dimension ; k++)
							{
								sprintf(temp_string + index, "%10.5e ",
									field->source_fields[i]->derivatives[j * element_dimension + k]);
								index += 12;
							}
						}
					}
				}
				sprintf(temp_string + index, "\n");
				Child_process_send_string_to_stdin(data->child_process,temp_string);
				DEALLOCATE(temp_string);
				if (temp_string = Child_process_get_line_from_stdout(
					data->child_process, data->timeout))
				{
					index = 0;
					for (i = 0 ; i < field->number_of_components ; i++)
					{
						sscanf(temp_string + index, "%f%n", &(field->values[i]), &j);
						index += j;
						sscanf(temp_string + index, "%s%n", buffer, &j);
						if (!strncmp(buffer, "#deriv=", 7))
						{
							index += j;
							/* Should check to see derivatives are supplied for
								all or none of the components and set the derivatives
								valid flag appropriately */
							if (sscanf(buffer + 7, "%d", &k) && k == element_dimension)
							{											
								for (k = 0 ; k < element_dimension ; k++)
								{
									sscanf(temp_string + index, "%f%n",
										&(field->derivatives[i * element_dimension + k]), &j);
									index += j;
								}
							}
						}
					}
					DEALLOCATE(temp_string);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_evaluate_cache_in_element."
						"  Invalid response from child process");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_evaluate_cache_in_element."
					"  Unable to allocate temporary string");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_external_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_external_evaluate_cache_in_element */

#define Computed_field_external_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_external_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_external_set_values_at_node \
	(Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Unavailable for this field type.
==============================================================================*/

#define Computed_field_external_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Unavailable for this field type.
==============================================================================*/

#define Computed_field_external_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_external_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_external(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;
	struct Computed_field_external_type_specific_data *data;

	ENTER(List_Computed_field_external);
	if (field && (data = 
		(struct Computed_field_external_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE,"    external_filename : %s\n",
			data->child_filename);
		display_message(INFORMATION_MESSAGE,"    number_of_source_values : %d\n",
			field->number_of_source_values);
		if (field->number_of_source_values)
		{
			display_message(INFORMATION_MESSAGE,"    source_values :");
			for (i=0;i<field->number_of_source_values;i++)
			{
				display_message(INFORMATION_MESSAGE," %f",
					field->source_values[i]);
			}
			display_message(INFORMATION_MESSAGE,"\n");
		}
		display_message(INFORMATION_MESSAGE,"    number_of_source_fields : %d\n",
			field->number_of_source_fields);
		if (field->number_of_source_fields)
		{				
			display_message(INFORMATION_MESSAGE,"    source_fields :");
			for (i=0;i<field->number_of_source_fields;i++)
			{
				display_message(INFORMATION_MESSAGE," %s",
					field->source_fields[i]->name);
			}
			display_message(INFORMATION_MESSAGE,"\n");
		}
		display_message(INFORMATION_MESSAGE,"    timeout %d\n",
			data->timeout);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_external.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_external */

static char *Computed_field_external_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string;
	int i;
	struct Computed_field_external_type_specific_data *data;

	ENTER(Computed_field_external_get_command_string);
	command_string = (char *)NULL;
	if (field && (data = 
		(struct Computed_field_external_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE," number_of_values %d",
			field->number_of_source_values);
		display_message(INFORMATION_MESSAGE," number_of_fields %d",
			field->number_of_source_fields);
		display_message(INFORMATION_MESSAGE," filename %s",
			data->child_filename);
		if (field->number_of_source_values)
		{
			display_message(INFORMATION_MESSAGE," values");
			for (i=0;i<field->number_of_source_values;i++)
			{
				display_message(INFORMATION_MESSAGE," %f",
					field->source_values[i]);
			}
		}
		if (field->number_of_source_fields)
		{				
			display_message(INFORMATION_MESSAGE," fields");
			for (i=0;i<field->number_of_source_fields;i++)
			{
				display_message(INFORMATION_MESSAGE," %s",
					field->source_fields[i]->name);
			}
		}
		display_message(INFORMATION_MESSAGE," timeout %d",
			data->timeout);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_external_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_external_get_command_string */

#define Computed_field_external_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_external(struct Computed_field *field,
	char *filename, int timeout,
	int number_of_source_values, FE_value *source_values,
	int number_of_source_fields, struct Computed_field **source_fields)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_EXTERNAL.
<source_fields> must point to an array of <number_of_sources> pointers to
Computed_fields. The resulting field will have as many
components as <number_of_sources> * components + number_of_source_values.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	char *filename_space, outstring[500], *result_string;
	FE_value *source_values_copy;
	int components, i, index, j, number_of_fields, number_of_values,
		return_code, total_values;
	struct Child_process *child_process;
	struct Computed_field **source_fields_copy;
	struct Computed_field_external_type_specific_data *data;

	ENTER(Computed_field_set_type_external);
	if (field&&filename&&source_fields)
	{
		return_code = 1;
		for (i=0;return_code&&(i<number_of_source_fields);i++)
		{
			if (source_fields[i])
			{
				total_values += source_fields[i]->number_of_components;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_external.  Invalid source_fields");
				return_code=0;
			}
		}
		/* Try executable and see how many components it will return */
		if (return_code && (child_process = CREATE(Child_process)(filename)))
		{
			sprintf(outstring, "%d #values %d #fields %d%n", total_values, number_of_source_values, 
				number_of_source_fields, &index);
			for (i = 0 ; i < number_of_source_fields ; i++)
			{
				sprintf(outstring + index, " %s %d AS_FOR_FIELD%n", source_fields[i]->name,
					source_fields[i]->number_of_components, &j);
				index += j;
			}
			sprintf(outstring + index, "\n");
			Child_process_send_string_to_stdin(child_process, outstring);
			if (result_string = Child_process_get_line_from_stdout(
				child_process, timeout))
			{
				sscanf(result_string, "%d%n", &components, &index);
				if (components < 1)
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_set_type_external.  "
						"External field incompatible with source fields");
					display_message(ERROR_MESSAGE,
						"%s", result_string);
					DESTROY(Child_process)(&child_process);
					return_code=0;
				}
				else
				{
					/* Check all the fields of the return string and give
					 sensible errors if they don't match */
					sscanf(result_string + index, "%s%d%n", outstring, &number_of_values,
						&j);
					index += j;
					if (fuzzy_string_compare(outstring, "#values"))
					{
						if (number_of_values == number_of_source_values)
						{
							sscanf(result_string + index, "%s%d%n", outstring, &number_of_fields,
								&j);
							index += j;
							if (fuzzy_string_compare(outstring, "#fields"))
							{
								if (number_of_fields == number_of_source_fields)
								{
									for (i = 0 ; (i < number_of_fields) ; i++)
									{
										sscanf(result_string + index, "%*s%d%s%n", 
											&number_of_values, outstring, &j);
										index += j;
										if (number_of_values != source_fields[i]->number_of_components)
										{
											display_message(ERROR_MESSAGE,
												"Computed_field_set_type_external."
												"  Number of components in field (%s %d) does not match the requirements of the external program (%d)",
												source_fields[i]->name, source_fields[i]->number_of_components,
												number_of_values);
											DESTROY(Child_process)(&child_process);
											return_code = 0;
										}
										if (fuzzy_string_compare(outstring, "AS_FOR_FIELD"))
										{
											/* Set a flag saying this and also check for
												other flags (i.e. NEVER and ALWAYS) */
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"Computed_field_set_type_external."
												"  Unknown derivative specifier (%s)",
												outstring);
											DESTROY(Child_process)(&child_process);
											return_code = 0;
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Computed_field_set_type_external."
										"  Number of source fields given (%d) does not match the requirements of the external program (%d)",
										number_of_source_fields, number_of_fields);
									DESTROY(Child_process)(&child_process);
									return_code = 0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Computed_field_set_type_external.  #values keyword not found");
								DESTROY(Child_process)(&child_process);
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_set_type_external."
								"  Number of source values given (%d) does not match the requirements of the external program (%d)",
								number_of_source_values, number_of_values);
							DESTROY(Child_process)(&child_process);
							return_code = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_set_type_external.  #values keyword not found");
						DESTROY(Child_process)(&child_process);
						return_code = 0;
					}
				}
				DEALLOCATE(result_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_external.  Invalid response from child process");
				DESTROY(Child_process)(&child_process);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_external.  Could not create child process");
			return_code=0;
		}
		if (return_code)
		{
			/* 1. make dynamic allocations for any new type-specific data */
			if (number_of_source_fields)
			{
				if (!ALLOCATE(source_fields_copy, struct Computed_field *,
					number_of_source_fields))
				{
					return_code = 0;
				}
			}
			else
			{
				source_fields_copy = (struct Computed_field **)NULL;
			}
			if (number_of_source_values)
			{
				if (!ALLOCATE(source_values_copy, FE_value, number_of_source_values))
				{
					return_code = 0;
				}
			}
			else
			{
				source_values_copy = (FE_value *)NULL;
			}
			if (return_code &&
				ALLOCATE(filename_space,char,strlen(filename) + 1) &&
				ALLOCATE(data, struct Computed_field_external_type_specific_data, 1))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->type_string = computed_field_external_type_string;
				for (i=0;i<number_of_source_fields;i++)
				{
					source_fields_copy[i]=ACCESS(Computed_field)(source_fields[i]);
				}
				field->source_fields=source_fields_copy;
				field->number_of_source_fields=number_of_source_fields;
				field->number_of_source_values = number_of_source_values;
				for (i=0;i<number_of_source_values;i++)
				{
					source_values_copy[i]=source_values[i];
				}
				field->source_values = source_values_copy;
				field->number_of_components=components;
				data->child_filename = filename_space;
				strcpy(filename_space, filename);
				data->timeout = timeout;
				data->child_process = ACCESS(Child_process)(child_process);

				/* Set all the methods */
				COMPUTED_FIELD_ESTABLISH_METHODS(external);
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_external.  Unable to establish external field");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_external.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_external */

int Computed_field_get_type_external(struct Computed_field *field,
	char **filename, int *timeout,
	int *number_of_source_values, FE_value **source_values,
	int *number_of_source_fields,struct Computed_field ***source_fields)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
If the field is of type COMPUTED_FIELD_EXTERNAL, the function allocates and
returns in <**source_fields> an array containing the <number_of_sources> source
fields making up the composite field - otherwise an error is reported.
It is up to the calling function to DEALLOCATE the returned array. Note that the
fields in the returned array are not ACCESSed.
==============================================================================*/
{
	int i, return_code;
	struct Computed_field_external_type_specific_data *data;

	ENTER(Computed_field_get_type_external);
	if (field && (field->type_string == computed_field_external_type_string) &&
		(data = (struct Computed_field_external_type_specific_data *)
		field->type_specific_data) && number_of_source_fields&&
		source_fields)
	{
		return_code = 1;
		*number_of_source_fields=field->number_of_source_fields;
		*number_of_source_values=field->number_of_source_values;
		*timeout = data->timeout;
		if (*number_of_source_fields)
		{
			if (ALLOCATE(*source_fields,struct Computed_field *,*number_of_source_fields))
			{
				for (i=0;i<(*number_of_source_fields);i++)
				{
					(*source_fields)[i]=field->source_fields[i];
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_get_type_external.  Could not allocate source_field array");
				return_code = 0;
			}
		}
		else
		{
			*source_fields = (struct Computed_field **)NULL;
		}
		if (return_code && *number_of_source_values)
		{
			if (ALLOCATE(*source_values, FE_value, *number_of_source_values))
			{
				for (i=0;i<(*number_of_source_values);i++)
				{
					(*source_values)[i]=field->source_values[i];
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_get_type_external.  Could not allocate source_values array");
				return_code = 0;
			}
		}
		else
		{
			*source_values = (FE_value *)NULL;
		}
		if (return_code &&
			ALLOCATE(*filename, char, strlen(data->child_filename)+1))
		{
			strcpy(*filename, data->child_filename);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_external.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_external */

static int define_Computed_field_type_external(struct Parse_state *state,
	void *field_void, void *computed_field_external_package_void)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_EXTERNAL (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token, *filename;
	int i,number_of_fields,number_of_source_values,return_code,
		temp_number_of_fields, temp_number_of_source_values,
		timeout;
	FE_value *source_values, *temp_source_values;
	struct Computed_field *field,**source_fields,**temp_source_fields;
	struct Computed_field_external_package *computed_field_external_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_external);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_external_package=
			(struct Computed_field_external_package *)
			computed_field_external_package_void))
	{
		filename = (char *)NULL;
		return_code=1;
		set_source_field_data.computed_field_manager=
			computed_field_external_package->computed_field_manager;
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data=(void *)NULL;
		/* get valid parameters for external field */
		source_fields=(struct Computed_field **)NULL;
		number_of_source_values = 0;
		source_values = (FE_value *)NULL;
		timeout = 5;
		if (Computed_field_is_type_external(field))
		{
			return_code=Computed_field_get_type_external(field,
				&filename, &timeout,
				&number_of_source_values,&source_values,
				&number_of_fields,&source_fields);
		}
		else
		{
			/* ALLOCATE and clear array of source fields */
			number_of_fields=1;
			if (ALLOCATE(source_fields,struct Computed_field *,number_of_fields))
			{
				for (i = 0; i < number_of_fields; i++)
				{
					source_fields[i] = (struct Computed_field *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_external.  Not enough memory");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* ACCESS the source fields for set_Computed_field_array */
			for (i = 0; i < number_of_fields; i++)
			{
				if (source_fields[i])
				{
					ACCESS(Computed_field)(source_fields[i]);
				}
			}
			/* try to handle help first */
			if (current_token=state->current_token)
			{
				if (!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
				{
					option_table = CREATE(Option_table)();
					/* number_of_fields */
					Option_table_add_entry(option_table, "number_of_fields", 
						&number_of_fields, NULL, set_int_non_negative);
					/* number_of_source_values */
					Option_table_add_entry(option_table, "number_of_values", 
						&number_of_source_values, NULL, set_int_non_negative);
					/* fields */
					set_source_field_array_data.number_of_fields=number_of_fields;
					set_source_field_array_data.conditional_data= &set_source_field_data;
					Option_table_add_entry(option_table, "fields", 
						source_fields, &set_source_field_array_data, 
						set_Computed_field_array);
					/* filename */
					Option_table_add_entry(option_table, "filename",
						&filename, (void *)1, set_name);
					/* timeout */
					Option_table_add_entry(option_table, "timeout",
						&timeout, NULL, set_int_positive);
					/* values */
					Option_table_add_entry(option_table, "values",
						source_values, &number_of_source_values, set_FE_value_array);
					return_code = Option_table_multi_parse(option_table, state);
					DESTROY(Option_table)(&option_table);
				}
			}
			/* parse the number_of_fields... */
			if (return_code&&(current_token=state->current_token))
			{
				/* ... only if the "number_of_fields" token or 
					"number_of_values" is next */
				while (fuzzy_string_compare("number_of_",state->current_token))
				{
					/* keep the number_of_fields to maintain any current ones */
					temp_number_of_fields=number_of_fields;
					temp_number_of_source_values=number_of_source_values;
					option_table = CREATE(Option_table)();
					/* number_of_fields */
					Option_table_add_entry(option_table, "number_of_fields", 
						&number_of_fields, NULL, set_int_non_negative);
					/* number_of_source_values */
					Option_table_add_entry(option_table, "number_of_values", 
						&number_of_source_values, NULL, set_int_non_negative);
					if (return_code = Option_table_multi_parse(option_table, state))
					{
						if (temp_number_of_fields != number_of_fields)
						{
							if (ALLOCATE(temp_source_fields,struct Computed_field *,
								temp_number_of_fields))
							{
								for (i=0;i<temp_number_of_fields;i++)
								{
									temp_source_fields[i] = (struct Computed_field *)NULL;
									if ((i < number_of_fields) && source_fields[i])
									{
										temp_source_fields[i] =
											ACCESS(Computed_field)(source_fields[i]);
									}
								}
								/* clean up the previous source_fields array */
								for (i=0;i<number_of_fields;i++)
								{
									DEACCESS(Computed_field)(&(source_fields[i]));
								}
								DEALLOCATE(source_fields);
								source_fields=temp_source_fields;
								number_of_fields=temp_number_of_fields;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"define_Computed_field_type_external.  Not enough memory");
								return_code=0;
							}
						}
						if (number_of_source_values != temp_number_of_source_values)
						{
							if (REALLOCATE(temp_source_values,source_values,FE_value,
								temp_number_of_source_values))
							{
								source_values=temp_source_values;
								/* make any new source values 0.0 */
								for (i=temp_number_of_source_values;
									i<number_of_source_values;i++)
								{
									source_values[i]=0.0;
								}
								number_of_source_values = temp_number_of_source_values;
							}
							else
							{
								return_code=0;
							}
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			/* parse the fields */
			if (return_code&&state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_array_data.number_of_fields=number_of_fields;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table, "fields", 
					source_fields, &set_source_field_array_data, 
					set_Computed_field_array);
				/* filename */
				Option_table_add_entry(option_table, "filename",
					&filename, (void *)1, set_name);
				/* timeout */
				Option_table_add_entry(option_table, "timeout",
					&timeout, NULL, set_int_positive);
				/* values */
				Option_table_add_entry(option_table, "values",
					source_values, &number_of_source_values, set_FE_value_array);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			if (return_code)
			{
				return_code=Computed_field_set_type_external(field,
					filename, timeout, number_of_source_values, source_values,
					number_of_fields,source_fields);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_external.  Failed");
				}
			}
			/* clean up the source fields array */
			for (i=0;i<number_of_fields;i++)
			{
				if (source_fields[i])
				{
					DEACCESS(Computed_field)(&(source_fields[i]));
				}
			}
			DEALLOCATE(source_fields);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_external.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_external */

int Computed_field_register_types_external(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_external_package 
		computed_field_external_package;

	ENTER(Computed_field_register_types_external);
	if (computed_field_package)
	{
		computed_field_external_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
			computed_field_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_external_type_string, 
			define_Computed_field_type_external,
			&computed_field_external_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_external.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_external */
