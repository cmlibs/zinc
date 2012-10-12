
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_function.h"

class Computed_field_function_package : public Computed_field_type_package
{
};

const char computed_field_function_type_string[] = "function";

int Computed_field_get_type_function(Computed_field *field,
	Computed_field **source_field, Computed_field **result_field,
	Computed_field **reference_field);

int define_Computed_field_type_function(Parse_state *state,
	void *field_modify_void, void *computed_field_function_package_void)
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_FUNCTION (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	Computed_field *source_field, *result_field,
		*reference_field;
	Computed_field_function_package *computed_field_function_package;
	Computed_field_modify_data *field_modify;
	Option_table *option_table;
	Set_Computed_field_conditional_data set_source_field_data,
		set_result_field_data, set_reference_field_data;

	ENTER(define_Computed_field_type_function);
	computed_field_function_package =
	  (Computed_field_function_package *)
	  computed_field_function_package_void;
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void) &&
		(computed_field_function_package != NULL))
	{
		USE_PARAMETER(computed_field_function_package);
		return_code = 1;
		source_field = (Computed_field *)NULL;
		result_field = (Computed_field *)NULL;
		reference_field = (Computed_field *)NULL;

		if ((NULL != field_modify->get_field()) &&
			(computed_field_function_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code = Computed_field_get_type_function(field_modify->get_field(),
				&source_field, &result_field,
				&reference_field);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			if (result_field)
			{
				ACCESS(Computed_field)(result_field);
			}
			if (reference_field)
			{
				ACCESS(Computed_field)(reference_field);
			}

			option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"The value of a function field is found by evaluating the <source_field> values, and then evaluating the <result_field> with respect to the <reference_field> using the values from the source field.  The sequence of operations <reference_field> to <result_field> become a function operating on the input <source_field> values.  Either the number of components in the <source_field> and <reference_field> should be the same, and then the number of components of this <field> will be the same as the number of components in the <result_field>, or if the <reference_field> and <result_field> are scalar then the function operation will be applied as many times as required for each component in the <source_field> and then this <field> will have as many components as the <source_field>.");
			/* reference_field */
			set_reference_field_data.computed_field_manager =
				field_modify->get_field_manager();
			set_reference_field_data.conditional_function =
				Computed_field_has_numerical_components;
			set_reference_field_data.conditional_function_user_data =
				(void *)NULL;
			Option_table_add_entry(option_table, "reference_field",
				&reference_field, &set_reference_field_data,
				set_Computed_field_conditional);
			/* result_field */
			set_result_field_data.computed_field_manager =
				field_modify->get_field_manager();
			set_result_field_data.conditional_function =
				Computed_field_has_numerical_components;
			set_result_field_data.conditional_function_user_data =
				(void *)NULL;
			Option_table_add_entry(option_table, "result_field",
				&result_field, &set_result_field_data,
				set_Computed_field_conditional);
			/* source_field */
			set_source_field_data.computed_field_manager =
				field_modify->get_field_manager();
			set_source_field_data.conditional_function =
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data =
				(void *)NULL;
			Option_table_add_entry(option_table, "source_field",
				&source_field, &set_source_field_data,
				set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table, state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_function(field_modify->get_field_module(),
						source_field, result_field, reference_field));
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_function.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			if (result_field)
			{
				DEACCESS(Computed_field)(&result_field);
			}
			if (reference_field)
			{
				DEACCESS(Computed_field)(&reference_field);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_function.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_function */

int Computed_field_register_types_function(
	Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_function_package
		*computed_field_function_package =
		new Computed_field_function_package;

	ENTER(Computed_field_register_types_function);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_function_type_string,
			define_Computed_field_type_function,
			computed_field_function_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_function.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_function */
