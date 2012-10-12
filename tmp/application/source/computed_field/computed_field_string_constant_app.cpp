
#include "api/cmiss_field_constant.h"

#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_string_constant.h"

char computed_field_string_constant_type_string[] = "string_constant";

int define_Computed_field_type_string_constant(struct Parse_state *state,
	void *field_modify_void, void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_STRING_CONSTANT (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code = 1;
	USE_PARAMETER(dummy_void);
	Computed_field_modify_data *field_modify =
		reinterpret_cast<Computed_field_modify_data *>(field_modify_void);
	if (state && field_modify)
	{
		return_code = 1;
		char *new_string_value = 0;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_default_string_entry(option_table, &new_string_value,
			/*description_string*/"STRING");
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			if (new_string_value)
			{
				return_code = field_modify->update_field_and_deaccess(
					Cmiss_field_module_create_string_constant(field_modify->get_field_module(),
						new_string_value));
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx define field NAME string_constant.  Missing string");
				return_code = 0;
			}
		}
		DEALLOCATE(new_string_value);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_string_constant.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_string_constant */

int Computed_field_register_types_string_constant(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_register_types_string_constant);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_string_constant_type_string,
			define_Computed_field_type_string_constant,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_string_constant.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_string_constant */
