
#include "zinc/fieldtime.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_time.h"

class Computed_field_time_package : public Computed_field_type_package
{
public:
	struct Time_keeper *time_keeper;
};

char computed_field_time_value_type_string[] = "time_value";
char computed_field_time_lookup_type_string[] = "time_lookup";

int Computed_field_get_type_time_lookup(struct Computed_field *field,
	struct Computed_field **source_field, struct Computed_field **time_field);

int define_Computed_field_type_time_lookup(struct Parse_state *state,
	void *field_modify_void,void *computed_field_time_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_TIME_LOOKUP (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data,
		set_time_field_data;

	ENTER(define_Computed_field_type_time_lookup);
	USE_PARAMETER(computed_field_time_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if ((NULL != field_modify->get_field()) &&
				(computed_field_time_lookup_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_time_lookup(field_modify->get_field(),
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
					field_modify->get_field_manager();
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				Option_table_add_entry(option_table,"field",&source_fields[0],
					&set_source_field_data,set_Computed_field_conditional);
				set_time_field_data.computed_field_manager=
					field_modify->get_field_manager();
				set_time_field_data.conditional_function=Computed_field_is_scalar;
				set_time_field_data.conditional_function_user_data=(void *)NULL;
				Option_table_add_entry(option_table,"time_field",&source_fields[1],
					&set_time_field_data,set_Computed_field_conditional);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->update_field_and_deaccess(
						Cmiss_field_module_create_time_lookup(field_modify->get_field_module(),
							source_fields[0], source_fields[1]));
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



/* Computed_field_get_type_time_value(struct Computed_field *field) */
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
There are no fields to fetch from a time value field.
==============================================================================*/

int define_Computed_field_type_time_value(struct Parse_state *state,
	void *field_modify_void,void *computed_field_time_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_TIME_VALUE (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code = 0;
	Computed_field_time_package *computed_field_time_package;
	Computed_field_modify_data *field_modify;

	ENTER(define_Computed_field_type_time_value);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
		(computed_field_time_package=
		(Computed_field_time_package *)
		computed_field_time_package_void))
	{
	if ((!(state->current_token)) ||
		(strcmp(PARSER_HELP_STRING,state->current_token)&&
		strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
		{
			return_code = field_modify->update_field_and_deaccess(
				Cmiss_field_module_create_time_value(field_modify->get_field_module(),
					computed_field_time_package->time_keeper));
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
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_time_package
		*computed_field_time_package =
		new Computed_field_time_package;

	ENTER(Computed_field_register_types_time);
	if (computed_field_package)
	{
		computed_field_time_package->time_keeper =
			time_keeper;
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_time_lookup_type_string,
			define_Computed_field_type_time_lookup,
			computed_field_time_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_time_value_type_string,
			define_Computed_field_type_time_value,
			computed_field_time_package);
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
