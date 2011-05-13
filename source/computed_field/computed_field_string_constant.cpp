/*******************************************************************************
FILE : computed_field_string_constant.c

LAST MODIFIED : 25 August 2006

DESCRIPTION :
Implements a constant string field.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
extern "C" {
#include <math.h>
#include "computed_field/computed_field.h"
}
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_string_constant.h"
}

namespace {

char computed_field_string_constant_type_string[] = "string_constant";

class Computed_field_string_constant : public Computed_field_core
{
public:
	char *string_value;

	Computed_field_string_constant(const char *string_value_in) :
		Computed_field_core(),
		string_value(duplicate_string(string_value_in))
	{
	};

	virtual bool attach_to_field(Computed_field *parent)
	{
		if (Computed_field_core::attach_to_field(parent))
		{
			if (string_value)
			{
				return true;
			}
		}
		return false;
	}

	~Computed_field_string_constant()
	{
		DEALLOCATE(string_value);
	}

private:
	Computed_field_core *copy()
	{
		return new Computed_field_string_constant(string_value);
	}

	const char *get_type_string()
	{
		return(computed_field_string_constant_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int is_defined_at_location(Field_location* location)
	{
		USE_PARAMETER(location);
		return 1;
	}

	int has_numerical_components()
	{
		return 0;
	}

	int set_string_at_location(Field_location* location, const char *string_value_in);

};

int Computed_field_string_constant::compare(Computed_field_core* other_field)
{
	Computed_field_string_constant* other;
	int return_code;

	ENTER(Computed_field_string_constant::compare);
	if (field && (other = dynamic_cast<Computed_field_string_constant*>(other_field)))
	{
		return_code = (0 == strcmp(string_value, other->string_value));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_string_constant::compare.  Invalid argument(s)");
		return_code = 0;
	}

	return (return_code);
}

int Computed_field_string_constant::evaluate_cache_at_location(
	Field_location* location)
{
	USE_PARAMETER(location);
	int return_code = 1;
	if (field)
	{
		if (field->string_cache)
		{
			DEALLOCATE(field->string_cache);
		}
		field->string_cache = duplicate_string(string_value);
		field->values_valid = 0;
		field->derivatives_valid = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_string_constant::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int Computed_field_string_constant::list()
{
	display_message(INFORMATION_MESSAGE,
		"    string constant : %s\n", string_value);
	return 1;
}

char *Computed_field_string_constant::get_command_string()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string = 0;
	int error = 0;
	append_string(&command_string,
		computed_field_string_constant_type_string, &error);
	append_string(&command_string, " ", &error);
	char *string_token = duplicate_string(string_value);
	make_valid_token(&string_token);
	append_string(&command_string, string_token, &error);
	DEALLOCATE(string_token);
	return (command_string);
}

int Computed_field_string_constant::set_string_at_location(
	Field_location* location, const char *string_value_in)
{
	USE_PARAMETER(location);
	int return_code = 1;
	char *new_string_value = duplicate_string(string_value_in);
	if (new_string_value)
	{
		DEALLOCATE(string_value);
		string_value = new_string_value;
		Computed_field_changed(field);
	}
	else
	{
		return_code = 0;
	}
	return (return_code);
}

} //namespace

struct Computed_field *Computed_field_create_string_constant(
	struct Cmiss_field_module *field_module, const char *string_value_in)
{
	Computed_field *field = NULL;
	if (string_value_in)
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/false, /*number_of_components*/1,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_string_constant(string_value_in));
	}
	return (field);
}

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
					Computed_field_create_string_constant(field_modify->get_field_module(),
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

