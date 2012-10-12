
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_conditional.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"

char computed_field_if_type_string[] = "if";

int Computed_field_get_type_if(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two,
	struct Computed_field **source_field_three);

int define_Computed_field_type_if(struct Parse_state *state,
	void *field_modify_void,void *computed_field_conditional_package_void)
/*******************************************************************************
LAST MODIFIED : 27 July 2007

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_IF (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_if);
	USE_PARAMETER(computed_field_conditional_package_void);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 3))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			source_fields[2] = (struct Computed_field *)NULL;
			if ((NULL != field_modify->get_field()) &&
				(computed_field_if_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_if(field_modify->get_field(),
					source_fields, source_fields + 1, source_fields + 2);
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
				if (source_fields[2])
				{
					ACCESS(Computed_field)(source_fields[2]);
				}

				option_table = CREATE(Option_table)();
				Option_table_add_help(option_table,
					"The if field uses three input fields.  "
					"The first field is evaluated and for each component "
					"if the value of the component is not zero (== true) then "
					"the value for that component is copied from the second field.  "
					"Otherwise (the first field value was zero == false) "
					"the value for that component is copied from the third field");

				/* fields */
				set_source_field_data.computed_field_manager=
					field_modify->get_field_manager();
				set_source_field_data.conditional_function=
		  Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=3;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->update_field_and_deaccess(
						Computed_field_create_if(field_modify->get_field_module(),
							source_fields[0], source_fields[1], source_fields[2]));
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_if.  Failed");
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
				if (source_fields[2])
				{
					DEACCESS(Computed_field)(&source_fields[2]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_if.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_if.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_if */

class Computed_field_conditional_package : public Computed_field_type_package
{
};

int Computed_field_register_types_conditional(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 27 July 2007

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_conditional_package
		*computed_field_conditional_package =
		new Computed_field_conditional_package;

	ENTER(Computed_field_register_types_conditional);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_if_type_string,
			define_Computed_field_type_if,
			computed_field_conditional_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_conditional.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_conditional */

