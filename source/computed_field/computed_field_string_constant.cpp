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
	/* This array is of size equal to the number of components of the field */
	char **string_constant_array;

	Computed_field_string_constant(Computed_field *field,
		char** string_constant_array_in) : Computed_field_core(field)
	{
		int i;
		string_constant_array = new char*[field->number_of_components];
		for (i = 0 ; i < field->number_of_components ; i++)
		{
			string_constant_array[i] = 
				duplicate_string(string_constant_array_in[i]);
		}

	};

	~Computed_field_string_constant();

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_string_constant(new_parent,
			string_constant_array);
	}

	char *get_type_string()
	{
		return(computed_field_string_constant_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int is_defined_at_location(Field_location* location);

	int has_numerical_components();
};

Computed_field_string_constant::~Computed_field_string_constant()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int i;

	ENTER(Computed_field_string_constant::~Computed_field_string_constant);
	if (field)
	{
		for (i = 0 ; i < field->number_of_components ; i++)
		{
			DEALLOCATE(string_constant_array[i]);
		}
		delete string_constant_array;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_string_constant::~Computed_field_string_constant.  Invalid argument(s)");
	}
	LEAVE;

} /* Computed_field_string_constant::~Computed_field_string_constant */

int Computed_field_string_constant::compare(Computed_field_core* other_field)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
String constant does not have numerical components.
==============================================================================*/
{
	Computed_field_string_constant* other;
	int i, return_code;

	ENTER(Computed_field_string_constant::is_defined_at_location);
	if (field && (other = dynamic_cast<Computed_field_string_constant*>(other_field)))
	{
		return_code = 1;
		for (i = 0 ; return_code && (i < field->number_of_components) ; i++)
		{
			return_code = (!strcmp(string_constant_array[i],
					other->string_constant_array[i]));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_string_constant::is_defined_at_location. Missing field.");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_string_constant::is_defined_at_location */

int Computed_field_string_constant::is_defined_at_location(
	Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
String constant does not have numerical components.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_string_constant::is_defined_at_location);
	if (field&&location)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_string_constant::is_defined_at_location. Missing field.");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_string_constant::is_defined_at_location */

int Computed_field_string_constant::has_numerical_components()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
String constant does not have numerical components.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_string_constant::has_numerical_components);
	if (field)
	{
		return_code = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_string_constant::has_numerical_components. Missing field.");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_string_constant::has_numerical_components */

int Computed_field_string_constant::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Print the constant values.
==============================================================================*/
{
	int error,i, return_code;

	ENTER(Computed_field_evaluate_cache_at_location);
	if (field && location)
	{
		error = 0;
		/* write the component values space separated,
			the string cache should store the separate components. */
		append_string(&field->string_cache,string_constant_array[0],&error);
		for (i=1;i<field->number_of_components;i++)
		{
			append_string(&field->string_cache," ",&error);
			append_string(&field->string_cache,string_constant_array[i],&error);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_string_constant_evaluate_as_string_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_string_constant_evaluate_as_string_at_node */

int Computed_field_string_constant::list()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(List_Computed_field_string_constant);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    string_constants :");
		for (i=0;i<field->number_of_components;i++)
		{
			display_message(INFORMATION_MESSAGE, " ");
			if (strchr(string_constant_array[i], ' '))
			{
				display_message(INFORMATION_MESSAGE, "\"");
				display_message(INFORMATION_MESSAGE,
					string_constant_array[i]);
				display_message(INFORMATION_MESSAGE, "\"");				
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					string_constant_array[i]);
			}
		}
		display_message(INFORMATION_MESSAGE,
			"\n");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_string_constant.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_string_constant */

char *Computed_field_string_constant::get_command_string()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string;
	int error, i;

	ENTER(Computed_field_string_constant::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_string_constant_type_string, &error);
		for (i=0;i<field->number_of_components;i++)
		{
			append_string(&command_string, " ", &error);
			if (strchr(string_constant_array[i], ' '))
			{
				append_string(&command_string, "\"", &error);
				append_string(&command_string,
					string_constant_array[i], &error);
				append_string(&command_string, "\"", &error);
			}
			else
			{
				append_string(&command_string,
					string_constant_array[i], &error);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_string_constant::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_string_constant::get_command_string */

} //namespace

int Computed_field_set_type_string_constant(struct Computed_field *field,
	int number_of_components, char **string_constant_array)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_STRING_CONSTANT with the supplied
<number_of_components> and the string values from <string_constant_array>.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_set_type_string_constant);
	if (field&&number_of_components&&string_constant_array)
	{
		return_code=1;
		/* 2. free current type-specific data */
		Computed_field_clear_type(field);
		/* 3. establish the new type */
		field->number_of_source_fields=0;
		field->number_of_source_values = 0;
		field->number_of_components=number_of_components;
		field->core = new Computed_field_string_constant(field,
			string_constant_array);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_string_constant.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_string_constant */

int Computed_field_get_type_string_constant(struct Computed_field *field,
	int *number_of_components, char ***string_constant_array_address)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_STRING_CONSTANT, the 
<number_of_components> and <string_constant_array> used by it are returned.
==============================================================================*/
{
	char **string_constant_array;
	int i, return_code;

	ENTER(Computed_field_get_type_string_constant);
	if (field&&(dynamic_cast<Computed_field_string_constant*>(field->core)))
	{
		*number_of_components = field->number_of_components;
		return_code=1;
		if (ALLOCATE(string_constant_array, char *, *number_of_components))
		{
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				if (!(string_constant_array[i] = duplicate_string(string_constant_array[i])))
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_get_type_string_constant.  "
						"Unable to duplicate string.");
					return_code = 0;
				}
			}
			*string_constant_array_address = string_constant_array;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_string_constant.  "
				"Unable to allocate array space.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_string_constant.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_string_constant */

int define_Computed_field_type_string_constant(struct Parse_state *state,
	void *field_modify_void, void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_STRING_CONSTANT (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	Computed_field_modify_data *field_modify;
	struct Computed_field *field;

	ENTER(define_Computed_field_type_string_constant);
	USE_PARAMETER(dummy_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
			(field=field_modify->field))
	{
		return_code=1;
		if (!(strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
		{
			/* error */
			display_message(INFORMATION_MESSAGE,
				" <STRINGS>");
		}
		else
		{
			Computed_field_set_type_string_constant(field,
				state->number_of_tokens - state->current_index,
				state->tokens + state->current_index);	
		}
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

