
#include <stdio.h>
#include <stdlib.h>

#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_composite.h"

class Computed_field_composite_package : public Computed_field_type_package
{
};

const char computed_field_composite_type_string[] = "composite";

int Computed_field_get_type_composite(struct Computed_field *field,
	int *number_of_components,
	int *number_of_source_fields,struct Computed_field ***source_fields,
	int *number_of_source_values, double **source_values,
	int **source_field_numbers,int **source_value_numbers);

int set_Computed_field_composite_source_data(struct Parse_state *state,
	void *source_data_void,void *computed_field_manager_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Note that fields are not ACCESSed by this function and should not be
ACCESSed in the initial source_data.
==============================================================================*/
{
	const char *current_token;
	double *temp_source_values, value;
	int component_no, components_to_add = -1, i, number_of_characters, return_code,
		source_field_number = -1, source_value_number = -1, *temp_source_field_numbers,
		*temp_source_value_numbers;
	struct Computed_field *field, **temp_source_fields;
	struct Computed_field_composite_source_data *source_data;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(set_Computed_field_composite_source_data);
	if (state && (source_data =
		(struct Computed_field_composite_source_data *)source_data_void) &&
		(computed_field_manager=
			(struct MANAGER(Computed_field) *)computed_field_manager_void))
	{
		return_code=1;
		current_token=state->current_token;
		if (current_token != 0)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				/* clear source_data */
				source_data->number_of_components = 0;
				source_data->number_of_source_fields = 0;
				if (source_data->source_fields)
				{
					DEALLOCATE(source_data->source_fields);
					source_data->source_fields = (struct Computed_field **)NULL;
				}
				source_data->number_of_source_values = 0;
				if (source_data->source_values)
				{
					DEALLOCATE(source_data->source_values);
					source_data->source_values = (double *)NULL;
				}
				if (source_data->source_field_numbers)
				{
					DEALLOCATE(source_data->source_field_numbers);
					source_data->source_field_numbers = (int *)NULL;
				}
				if (source_data->source_value_numbers)
				{
					DEALLOCATE(source_data->source_value_numbers);
					source_data->source_value_numbers = (int *)NULL;
				}
				while (return_code && (current_token = state->current_token))
				{
					/* first try to find a number in the token */
					if (strchr("+-0123456789",current_token[0]))
					{
						if ((1 == sscanf(current_token, "%lf%n",
							&value, &number_of_characters)) &&
							((int)strlen(current_token) == number_of_characters))
						{
							shift_Parse_state(state,1);
							if (REALLOCATE(temp_source_values, source_data->source_values,
								double, source_data->number_of_source_values+1))
							{
								source_data->source_values = temp_source_values;
								temp_source_values[source_data->number_of_source_values] =
									value;
								components_to_add = 1;
								source_field_number = -1;
								source_value_number = source_data->number_of_source_values;
								(source_data->number_of_source_values)++;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_Computed_field_composite_source_data.  "
									"Not enough memory");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Invalid characters in composite value: %s",current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						/* component_no = -1 denotes the whole field may be used */
						component_no = -1;
						field = Computed_field_manager_get_field_or_component(
							computed_field_manager, current_token, &component_no);
						if (field)
						{
							shift_Parse_state(state,1);
							if (component_no == -1)
							{
								components_to_add = field->number_of_components;
								/* following is the first source_value_number to add */
								source_value_number = 0;
							}
							else
							{
								components_to_add = 1;
								source_value_number = component_no;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown field or field.component: %s", current_token);
							display_parse_state_location(state);
							return_code = 0;
						}
						if (return_code)
						{
							/* try to find field in current source_fields */
							source_field_number = -1;
							for (i=0;(i<source_data->number_of_source_fields)&&
								(0>source_field_number);i++)
							{
								if (field == source_data->source_fields[i])
								{
									source_field_number = i;
								}
							}
							if (-1 == source_field_number)
							{
								if (REALLOCATE(temp_source_fields,
									source_data->source_fields,struct Computed_field *,
									source_data->number_of_source_fields+1))
								{
									source_data->source_fields = temp_source_fields;
									temp_source_fields[source_data->number_of_source_fields] =
										field;
									source_field_number = source_data->number_of_source_fields;
									(source_data->number_of_source_fields)++;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"set_Computed_field_composite_source_data.  "
										"Not enough memory");
									return_code=0;
								}
							}
						}
					}
					if (return_code)
					{
						if (REALLOCATE(temp_source_field_numbers,
							source_data->source_field_numbers,int,
							source_data->number_of_components+components_to_add))
						{
							source_data->source_field_numbers = temp_source_field_numbers;
							if (REALLOCATE(temp_source_value_numbers,
								source_data->source_value_numbers,int,
								source_data->number_of_components+components_to_add))
							{
								source_data->source_value_numbers = temp_source_value_numbers;
								for (i=0;i<components_to_add;i++)
								{
									source_data->source_field_numbers
										[source_data->number_of_components] = source_field_number;
									source_data->source_value_numbers
										[source_data->number_of_components] =
										source_value_number + i;
									(source_data->number_of_components)++;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_Computed_field_composite_source_data.  "
									"Not enough memory");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_Computed_field_composite_source_data.  Not enough memory");
							return_code=0;
						}
					}
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #|FIELD[.COMPONENT_NAME] "
					"#|FIELD[.COMPONENT_NAME] #|FIELD[.COMPONENT_NAME] ... ");
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"Missing composition fields/components and values");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Computed_field_composite_source_data.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Computed_field_composite_source_data */

int define_Computed_field_type_composite(struct Parse_state *state,
	void *field_modify_void,void *computed_field_composite_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_COMPOSITE (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	Computed_field_modify_data *field_modify;
	struct Computed_field_composite_source_data source_data;
	struct Option_table *option_table;

	ENTER(define_Computed_field_type_composite);
	USE_PARAMETER(computed_field_composite_package_void);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_data.number_of_components = 0;
		source_data.number_of_source_fields = 0;
		source_data.source_fields = (struct Computed_field **)NULL;
		source_data.number_of_source_values = 0;
		source_data.source_values = (double *)NULL;
		source_data.source_field_numbers = (int *)NULL;
		source_data.source_value_numbers = (int *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_composite_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code = Computed_field_get_type_composite(field_modify->get_field(),
				&(source_data.number_of_components),
				&(source_data.number_of_source_fields), &(source_data.source_fields),
				&(source_data.number_of_source_values), &(source_data.source_values),
				&(source_data.source_field_numbers),
				&(source_data.source_value_numbers));
		}
		if (return_code)
		{
			/*???RC begin temporary code */
			/*???RC swallow up old "number_of_scalars # scalars" tokens used by old
				composite field command for backward compatibility */
			if (((state->current_index+2) < state->number_of_tokens) &&
				(fuzzy_string_compare(state->tokens[state->current_index],
					"number_of_scalars")) &&
				(0<atoi(state->tokens[state->current_index+1])) &&
				(fuzzy_string_compare(state->tokens[state->current_index+2],
					"scalars")))
			{
				shift_Parse_state(state,3);
			}
			/*???RC end temporary code */

			option_table = CREATE(Option_table)();
			/* default option: composite field definition */
			Option_table_add_entry(option_table, (char *)NULL, &source_data,
				field_modify->get_field_manager(),
				set_Computed_field_composite_source_data);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_composite(field_modify->get_field_module(),
						source_data.number_of_components,
						source_data.number_of_source_fields, source_data.source_fields,
						source_data.number_of_source_values, source_data.source_values,
						source_data.source_field_numbers,
						source_data.source_value_numbers));
			}
			DESTROY(Option_table)(&option_table);
		}
		/* clean up the source data */
		if (source_data.source_fields)
		{
			DEALLOCATE(source_data.source_fields);
		}
		if (source_data.source_values)
		{
			DEALLOCATE(source_data.source_values);
		}
		if (source_data.source_field_numbers)
		{
			DEALLOCATE(source_data.source_field_numbers);
		}
		if (source_data.source_value_numbers)
		{
			DEALLOCATE(source_data.source_value_numbers);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_composite.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_composite */


int define_Computed_field_type_constant(struct Parse_state *state,
	void *field_modify_void, void *computed_field_composite_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CONSTANT (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	const char *current_token;
	double *values, *temp_values;
	int i, number_of_values, previous_number_of_values, return_code;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;

	ENTER(define_Computed_field_type_constant);
	USE_PARAMETER(computed_field_composite_package_void);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code = 1;
		previous_number_of_values = 0;
		values = (double *)NULL;

		Computed_field *field = field_modify->get_field();
		if (field && Computed_field_is_constant(field))
		{
			previous_number_of_values = field->number_of_source_values;
			if (ALLOCATE(values, double, previous_number_of_values))
			{
				for (i = 0; i < previous_number_of_values; i++)
				{
					values[i] = static_cast<double>(field->source_values[i]);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_constant.  Could not allocate values");
				return_code = 0;
			}
		}
		if (return_code)
		{
			/*???RC begin temporary code */
			/*???RC swallow up old "number_of_values # values" tokens used by old
				constant field command for backward compatibility */
			if (((state->current_index + 2) < state->number_of_tokens) &&
				(fuzzy_string_compare(state->tokens[state->current_index],
					"number_of_values")) &&
				(0<atoi(state->tokens[state->current_index+1])) &&
				(fuzzy_string_compare(state->tokens[state->current_index+2],
					"values")))
			{
				shift_Parse_state(state,3);
			}
			/*???RC swallow up old "value N" tokens used by old constant field if
				default number_of_values=1 used ... for backward compatibility */
			else if (((state->current_index + 2) == state->number_of_tokens) &&
				fuzzy_string_compare(state->tokens[state->current_index], "values"))
			{
				shift_Parse_state(state, 1);
			}
			/*???RC end temporary code */

			if ((current_token = state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING, current_token) &&
					strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))))
			{
				number_of_values = previous_number_of_values;
			}
			else if (0 < (number_of_values =
				state->number_of_tokens - state->current_index))
			{
				if (REALLOCATE(temp_values, values, double, number_of_values))
				{
					values = temp_values;
					/* clear new values to 0.0 */
					for (i = previous_number_of_values; i < number_of_values; i++)
					{
						values[i] = 0.0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_constant.  "
						"Could not reallocate values");
					return_code = 0;
				}
			}
			else
			{
				number_of_values = 0;
			}
			if (return_code)
			{
				option_table = CREATE(Option_table)();
				/* default option: composite field definition */
				Option_table_add_help(option_table,
				  "A constant field may be defined as having one or more components.  Each of the <values> listed is used to asign a constant value to the corresponding field component. Fields with more than 1 component can be used to represent vectors or matrices.  An m by n matrix requires (m*n) components and the components of the matrix are listed row by row.");
				Option_table_add_entry(option_table, (char *)NULL, values,
					&number_of_values, set_double_vector);
				return_code = Option_table_multi_parse(option_table, state);
				if (return_code)
				{
					return_code = field_modify->update_field_and_deaccess(
						Computed_field_create_constant(field_modify->get_field_module(),
							number_of_values, values));
				}
				DESTROY(Option_table)(&option_table);
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING,state->current_token) &&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_constant.  Failed");
				}
			}
		}
		if (values)
		{
			DEALLOCATE(values);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_constant.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_constant */


int Computed_field_register_types_composite(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_composite_package
		*computed_field_composite_package =
		new Computed_field_composite_package;

	ENTER(Computed_field_register_types_composite);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_composite_type_string,
			define_Computed_field_type_composite,
			computed_field_composite_package);
		/* "constant" = alias for composite included for backward compatibility */
		return_code = Computed_field_package_add_type(computed_field_package,
			"constant", define_Computed_field_type_constant,
			computed_field_composite_package);
		/* "component" = alias for composite included for backward compatibility */
		return_code = Computed_field_package_add_type(computed_field_package,
			"component", define_Computed_field_type_composite,
			computed_field_composite_package);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_composite.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_composite */
