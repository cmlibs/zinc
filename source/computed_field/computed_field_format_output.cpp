/*******************************************************************************
FILE : computed_field_format_output.c

LAST MODIFIED : 14 December 2010

DESCRIPTION :
Implements a field which formats numeric values as a string.
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
#include "computed_field/computed_field_format_output.h"
#include "computed_field/computed_field_set.h"
}

namespace {

char computed_field_format_output_type_string[] = "format_output";

class Computed_field_format_output : public Computed_field_core
{
public:
	char *format_string;
	/* Estimate from the format_string and number of components 
	 * a sufficient output string allocation. */
	int output_allocation_size;

	Computed_field_format_output(int number_of_components, char* format_string_in) :
		Computed_field_core()
	{
		format_string = duplicate_string(format_string_in);
		output_allocation_size = number_of_components * 30 + strlen(format_string_in);
	}

	~Computed_field_format_output()
	{
		if (format_string)
			DEALLOCATE(format_string);
	}

private:
	Computed_field_core *copy()
	{
		return new Computed_field_format_output(field->number_of_components, format_string);
	}

	const char *get_type_string()
	{
		return(computed_field_format_output_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		Computed_field_format_output *other_format_output;
		if ((other_format_output = dynamic_cast<Computed_field_format_output*>(other_field)))
		{
			if (0 == strcmp(format_string, other_format_output->format_string))
				return 1;
			else
				return 0;
		}
		else
			return 0;
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	/*****************************************************************************//**
	 * Computed_field_format_output never has numerical components 
	 */
	int has_numerical_components()
	{
		return 0;
	}

	virtual Cmiss_field_value_type get_value_type() const
	{
		return CMISS_FIELD_VALUE_TYPE_STRING;
	}

};

int Computed_field_format_output::evaluate_cache_at_location(
    Field_location* location)
{
	int return_code;

	ENTER(Computed_field_evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields > 0) &&
		(field->number_of_components <= 4) &&
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (0 != (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location)))
		{
			field->values_valid = 0; // string-valued
			/* 2. Write out the source field values using the format_string */
			/* Allocate a generous string.
			 */
			if (field->string_cache)
				DEALLOCATE(field->string_cache);
			ALLOCATE(field->string_cache, char, output_allocation_size);
#if defined (_MSC_VER)
			/* If the MSVC _snprintf overflows then it won't be null terminated so ensure this 0. */
			field->string_cache[output_allocation_size-1] = 0;
#endif // defined (_MSC_VER)
			switch (field->number_of_components)
			{
			case 1:
			{
				snprintf(field->string_cache, output_allocation_size-1, format_string,
					field->source_fields[0]->values[0]);
				return_code = 1;
			} break;
			case 2:
			{
				snprintf(field->string_cache, output_allocation_size-1, format_string,
					field->source_fields[0]->values[0],
					field->source_fields[0]->values[1]);
				return_code = 1;
			} break;
			case 3:
			{
				snprintf(field->string_cache, output_allocation_size-1, format_string,
					field->source_fields[0]->values[0],
					field->source_fields[0]->values[1],
					field->source_fields[0]->values[2]);
				return_code = 1;
			} break;
			case 4:
			{
				snprintf(field->string_cache, output_allocation_size-1, format_string,
					field->source_fields[0]->values[0],
					field->source_fields[0]->values[1],
					field->source_fields[0]->values[2],
					field->source_fields[0]->values[3]);
				return_code = 1;
			} break;
			default:
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_format_output_evaluate_as_string_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_format_output_evaluate_as_string_at_node */

int Computed_field_format_output::list()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_format_output);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    format_string : \"%s\"\n", format_string);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_format_output.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_format_output */

char *Computed_field_format_output::get_command_string()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string;
	int error;

	ENTER(Computed_field_format_output::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string, " format_string", &error);
		append_string(&command_string, "\"", &error);
		append_string(&command_string,
			format_string, &error);
		append_string(&command_string, "\"", &error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_format_output::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_format_output::get_command_string */

} //namespace

struct Computed_field *Computed_field_create_format_output(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field, char *format_string)
{
	Cmiss_field_id field = 0;
	if (source_field && format_string)
	{
		if (source_field->number_of_components <= 4)
		{
			int valid_string = 1;
			char *remaining_string = format_string;
			int number_of_format_specifiers = 0;
			while (valid_string && (remaining_string = strchr(remaining_string, '%')))
			{
				number_of_format_specifiers++;
				remaining_string++;
				/* Ignore modifiers */
				int specifiers = strspn(remaining_string, "0123456789.hlL -+#");
				remaining_string += specifiers;
				/* Fail if we don't get the expected format codes */
				if (0 != strcspn(remaining_string, "eEfgG"))
				{
					valid_string = 0;
				}
				remaining_string++;
			}
			if (number_of_format_specifiers != source_field->number_of_components)
				valid_string = 0;
			if (valid_string)
			{
				field = Computed_field_create_generic(field_module,
					/*check_source_field_regions*/true, source_field->number_of_components,
					/*number_of_source_fields*/1, &source_field,
					/*number_of_source_values*/0, NULL,
					new Computed_field_format_output(source_field->number_of_components, format_string));
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_create_format_output.  Invalid or unsupported format_string.");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_create_format_output.  Only source fields with between 1 and 4 components are currently supported.");
		}
	}
	return (field);
}

int Computed_field_get_type_format_output(struct Computed_field *field,
	struct Computed_field **source_field, char **format_string_out)
{
	int return_code = 1;

	ENTER(Computed_field_get_type_format_output);
	Computed_field_format_output *this_field;
	if (field&&(this_field = dynamic_cast<Computed_field_format_output*>(field->core)))
	{
		*source_field = field->source_fields[0];
		*format_string_out = duplicate_string(this_field->format_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_format_output.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_format_output */

int define_Computed_field_type_format_output(struct Parse_state *state,
	void *field_modify_void,void *computed_field_arithmetic_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_FORMAT_OUTPUT (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	char *format_string;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_format_output);
	USE_PARAMETER(computed_field_arithmetic_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		source_field = (struct Computed_field *)NULL;
		format_string = NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_format_output_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code=Computed_field_get_type_format_output(field_modify->get_field(), 
				&source_field, &format_string);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}

			option_table = CREATE(Option_table)();
			/* fields */
			set_source_field_data.computed_field_manager=
				field_modify->get_field_manager();
			set_source_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_Computed_field_conditional_entry(option_table,"field",
				&source_field, &set_source_field_data);
			Option_table_add_string_entry(option_table, "format_string", &format_string,
				"C style formatting string");
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_format_output(field_modify->get_field_module(),
						source_field, format_string));
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_format_output.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			if (format_string)
				DEALLOCATE(format_string);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_format_output.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_format_output.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_format_output */

int Computed_field_register_types_format_output(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_register_types_format_output);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_format_output_type_string,
			define_Computed_field_type_format_output,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_format_output.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_format_output */

