/*******************************************************************************
FILE : computed_field_arithmetic_operators.cpp

LAST MODIFIED : 15 May 2008

DESCRIPTION :
Implements a number of basic component wise operators on computed fields.
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
 * Shane Blackett (shane at blackett.co.nz)
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
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_arithmetic_operators.h"
}

class Computed_field_arithmetic_operators_package : public Computed_field_type_package
{
	/* empty; field manager now comes from region, passed in Computed_field_modify_data */
};

namespace {

char computed_field_power_type_string[] = "power";

class Computed_field_power : public Computed_field_core
{
public:
	Computed_field_power() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_power();
	}

	const char *get_type_string()
	{
		return(computed_field_power_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_power*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

int Computed_field_power::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_power::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields > 0) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (0 != (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location)))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)pow((double)(field->source_fields[0]->values[i]),
					(double)(field->source_fields[1]->values[i]));
			}
			if (field->source_fields[0]->derivatives_valid
				&& field->source_fields[1]->derivatives_valid)
			{
				number_of_xi = location->get_number_of_derivatives();
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						/* d(u^v)/dx =
						 *   v * u^(v-1) * du/dx   +   u^v * ln(u) * dv/dx
						 */
						*derivative =
							field->source_fields[1]->values[i] *
							(FE_value)pow((double)(field->source_fields[0]->values[i]),
								(double)(field->source_fields[1]->values[i]-1)) *
							field->source_fields[0]->derivatives[i * number_of_xi + j] +
							(FE_value)pow((double)(field->source_fields[0]->values[i]),
								(double)(field->source_fields[1]->values[i])) *
							(FE_value)log((double)(field->source_fields[0]->values[i])) *
							field->source_fields[1]->derivatives[i * number_of_xi + j];
						derivative++;
					}
				}
				field->derivatives_valid = 1;
			}
			else
			{
				field->derivatives_valid = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_power::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_power::evaluate_cache_at_location */


int Computed_field_power::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_power);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source fields : %s %s\n",field->source_fields[0]->name,
			field->source_fields[1]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_power.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_power */

char *Computed_field_power::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_power::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_power_type_string, &error);
		append_string(&command_string, " fields ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, " ", &error);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_power::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_power::get_command_string */

} //namespace

Computed_field *Computed_field_create_power(Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
{
	Computed_field *field, *source_fields[2];

	ENTER(Computed_field_create_power);
	/* Access and broadcast before checking components match,
		the local source_field_one and source_field_two will 
		get replaced if necessary. */
	ACCESS(Computed_field)(source_field_one);
	ACCESS(Computed_field)(source_field_two);
	if (field_module && source_field_one && source_field_two &&
		Computed_field_broadcast_field_components(field_module,
			&source_field_one, &source_field_two) &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field_one->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_power());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_power.  Invalid argument(s)");
		field = (Computed_field *)NULL;
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);
	LEAVE;

	return (field);
} /* Computed_field_create_power */

int Computed_field_get_type_power(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_POWER, the 
<source_field_one> and <source_field_two> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_power);
	if (field&&(dynamic_cast<Computed_field_power*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_power.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_power */

int define_Computed_field_type_power(struct Parse_state *state,
	void *field_modify_void,void *computed_field_arithmetic_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_POWER (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_power);
	USE_PARAMETER(computed_field_arithmetic_operators_package_void);
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
				(computed_field_power_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_power(field_modify->get_field(), 
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
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->update_field_and_deaccess(
						Computed_field_create_power(field_modify->get_field_module(),
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
							"define_Computed_field_type_power.  Failed");
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
				"define_Computed_field_type_power.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_power.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_power */

namespace {

char computed_field_multiply_components_type_string[] = "multiply_components";

class Computed_field_multiply_components : public Computed_field_core
{
public:
	Computed_field_multiply_components() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_multiply_components();
	}

	const char *get_type_string()
	{
		return(computed_field_multiply_components_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_multiply_components*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

int Computed_field_multiply_components::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

#if ! defined (OPTIMISED)
	ENTER(Computed_field_multiply_components::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields > 0) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
#endif /* ! defined (OPTIMISED) */
		/* 1. Precalculate any source fields that this field depends on */
		if (0 != (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location)))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = field->source_fields[0]->values[i] * 
					field->source_fields[1]->values[i];
			}
			if (field->source_fields[0]->derivatives_valid
				&& field->source_fields[1]->derivatives_valid)
			{
				number_of_xi = location->get_number_of_derivatives();
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						*derivative = 
							field->source_fields[0]->derivatives[i * number_of_xi + j] *
							field->source_fields[1]->values[i] + 
							field->source_fields[1]->derivatives[i * number_of_xi + j] *
							field->source_fields[0]->values[i];
						derivative++;
					}
				}
				field->derivatives_valid = 1;
			}
			else
			{
				field->derivatives_valid = 0;
			}
		}
#if ! defined (OPTIMISED)
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_multiply_components::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
#endif /* ! defined (OPTIMISED) */

	return (return_code);
} /* Computed_field_multiply_components::evaluate_cache_at_location */



int Computed_field_multiply_components::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_multiply_components);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source fields : %s %s\n",field->source_fields[0]->name,
			field->source_fields[1]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_multiply_components.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_multiply_components */

char *Computed_field_multiply_components::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_multiply_components::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_multiply_components_type_string, &error);
		append_string(&command_string, " fields ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, " ", &error);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_multiply_components::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_multiply_components::get_command_string */

} //namespace

Computed_field *Computed_field_create_multiply(
	Cmiss_field_module *field_module,
	Computed_field *source_field_one, Computed_field *source_field_two)
{
	Computed_field *field, *source_fields[2];

	ENTER(Computed_field_create_multiply);
	/* Access and broadcast before checking components match,
		the local source_field_one and source_field_two will 
		get replaced if necessary. */
	ACCESS(Computed_field)(source_field_one);
	ACCESS(Computed_field)(source_field_two);
	if (field_module && source_field_one && source_field_two &&
		Computed_field_broadcast_field_components(field_module,
			&source_field_one, &source_field_two) &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field_one->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_multiply_components());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_multiply.  Invalid argument(s)");
		field = (Computed_field *)NULL;
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);
	LEAVE;

	return (field);
} /* Computed_field_create_multiply */

int Computed_field_get_type_multiply_components(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_MULTIPLY_COMPONENTS, the 
<source_field_one> and <source_field_two> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_multiply_components);
	if (field&&(dynamic_cast<Computed_field_multiply_components*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_multiply_components.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_multiply_components */

int define_Computed_field_type_multiply_components(struct Parse_state *state,
	void *field_modify_void,void *computed_field_arithmetic_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_MULTIPLY_COMPONENTS (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_multiply_components);
	USE_PARAMETER(computed_field_arithmetic_operators_package_void);
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
				(computed_field_multiply_components_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_multiply_components(field_modify->get_field(), 
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
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->update_field_and_deaccess(
						Computed_field_create_multiply(field_modify->get_field_module(),
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
							"define_Computed_field_type_multiply_components.  Failed");
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
				"define_Computed_field_type_multiply_components.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_multiply_components.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_multiply_components */

namespace {

char computed_field_divide_components_type_string[] = "divide_components";

class Computed_field_divide_components : public Computed_field_core
{
public:
	Computed_field_divide_components() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_divide_components();
	}

	const char *get_type_string()
	{
		return(computed_field_divide_components_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_divide_components*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

int Computed_field_divide_components::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	FE_value *derivative, vsquared;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_divide_components::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 2) && 
		(field->number_of_components == field->source_fields[0]->number_of_components && 
		(field->number_of_components == field->source_fields[1]->number_of_components)))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (0 != (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location)))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = field->source_fields[0]->values[i] / 
					field->source_fields[1]->values[i];
			}
			if (field->source_fields[0]->derivatives_valid
				&& field->source_fields[1]->derivatives_valid)
			{
				number_of_xi = location->get_number_of_derivatives();
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					vsquared = field->source_fields[1]->values[i] 
						* field->source_fields[1]->values[i];
					for (j = 0 ; j < number_of_xi ; j++)
					{
						*derivative = 
							(field->source_fields[0]->derivatives[i * number_of_xi + j] *
							field->source_fields[1]->values[i] -
							field->source_fields[1]->derivatives[i * number_of_xi + j] *
							field->source_fields[0]->values[i]) / vsquared;
						derivative++;
					}
				}
				field->derivatives_valid = 1;
			}
			else
			{
				field->derivatives_valid = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_divide_components::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_divide_components::evaluate_cache_at_location */


int Computed_field_divide_components::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_divide_components);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source fields : %s %s\n",field->source_fields[0]->name,
			field->source_fields[1]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_divide_components.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_divide_components */

char *Computed_field_divide_components::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_divide_components::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_divide_components_type_string, &error);
		append_string(&command_string, " fields ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, " ", &error);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_divide_components::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_divide_components::get_command_string */

} //namespace

Computed_field *Computed_field_create_divide(Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
{
	Computed_field *field, *source_fields[2];

	ENTER(Computed_field_create_divide);
	/* Access and broadcast before checking components match,
		the local source_field_one and source_field_two will 
		get replaced if necessary. */
	ACCESS(Computed_field)(source_field_one);
	ACCESS(Computed_field)(source_field_two);
	if (field_module && source_field_one && source_field_two &&
		Computed_field_broadcast_field_components(field_module,
			&source_field_one, &source_field_two) &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field_one->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_divide_components());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_divide.  Invalid argument(s)");
		field = (Computed_field *)NULL;
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);
	LEAVE;

	return (field);
} /* Computed_field_create_divide_components */

int Computed_field_get_type_divide_components(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_DIVIDE_COMPONENTS, the 
<source_field_one> and <source_field_two> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_divide_components);
	if (field&&(dynamic_cast<Computed_field_divide_components*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_divide_components.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_divide_components */

int define_Computed_field_type_divide_components(struct Parse_state *state,
	void *field_modify_void,void *computed_field_arithmetic_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_DIVIDE_COMPONENTS (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_divide_components);
	USE_PARAMETER(computed_field_arithmetic_operators_package_void);
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
				(computed_field_divide_components_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_divide_components(field_modify->get_field(), 
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
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->update_field_and_deaccess(
						Computed_field_create_divide(field_modify->get_field_module(),
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
							"define_Computed_field_type_divide_components.  Failed");
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
				"define_Computed_field_type_divide_components.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_divide_components.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_divide_components */

namespace {

char computed_field_add_type_string[] = "add";

class Computed_field_add : public Computed_field_core
{
public:
	Computed_field_add() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_add();
	}

	const char *get_type_string()
	{
		return(computed_field_add_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_add*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

int Computed_field_add::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	FE_value *temp, *temp1, *temp2;
	int element_dimension, i, return_code;

	ENTER(Computed_field_add::evaluate_cache_at_location);
#if ! defined (OPTIMISED)
	if (field && location && (field->number_of_source_fields == 2) && 
		(field->number_of_components == field->source_fields[0]->number_of_components) && 
		(field->number_of_components == field->source_fields[1]->number_of_components))
	{
#endif /* ! defined (OPTIMISED) */
		/* 1. Precalculate any source fields that this field depends on */
		if (0 != (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location)))
		{
			/* 2. Calculate the field */
			for (i=0;i<field->number_of_components;i++)
			{
				field->values[i]=
					field->source_values[0]*field->source_fields[0]->values[i]+
					field->source_values[1]*field->source_fields[1]->values[i];
			}
			if ((element_dimension=location->get_number_of_derivatives()) &&
				field->source_fields[0]->derivatives_valid && 
				field->source_fields[1]->derivatives_valid)
			{
				temp=field->derivatives;
				temp1=field->source_fields[0]->derivatives;
				temp2=field->source_fields[1]->derivatives;
				for (i=(field->number_of_components*element_dimension);
					  0<i;i--)
				{
					(*temp)=field->source_values[0]*(*temp1)+
						field->source_values[1]*(*temp2);
					temp++;
					temp1++;
					temp2++;
				}
				field->derivatives_valid = 1;
			}
			else
			{
				field->derivatives_valid = 0;
			}
		}
#if ! defined (OPTIMISED)
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_add::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
#endif /* ! defined (OPTIMISED) */

	return (return_code);
} /* Computed_field_add::evaluate_cache_at_location */


int Computed_field_add::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_add);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    field 1 : %s\n    scale factor 1 : %g\n",
			field->source_fields[0]->name,field->source_values[0]);
		display_message(INFORMATION_MESSAGE,
			"    field 2 : %s\n    scale factor 2 : %g\n",
			field->source_fields[1]->name,field->source_values[1]);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_add.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_add */

char *Computed_field_add::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[80];
	int error;

	ENTER(Computed_field_add::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_add_type_string, &error);
		append_string(&command_string, " fields ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, " ", &error);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		sprintf(temp_string, " scale_factors %g %g",
			field->source_values[0], field->source_values[1]);
		append_string(&command_string, temp_string, &error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_add::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_add::get_command_string */

} //namespace

Computed_field *Computed_field_create_weighted_add(Cmiss_field_module *field_module,
	struct Computed_field *source_field_one, double scale_factor1,
	struct Computed_field *source_field_two, double scale_factor2)
{
	Computed_field *field, *source_fields[2];
	double source_values[2];

	ENTER(Computed_field_create_weighted_add);
	/* Access and broadcast before checking components match,
		the local source_field_one and source_field_two will 
		get replaced if necessary. */
	ACCESS(Computed_field)(source_field_one);
	ACCESS(Computed_field)(source_field_two);
	if (field_module && source_field_one && source_field_two &&
		Computed_field_broadcast_field_components(field_module,
			&source_field_one, &source_field_two) &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		source_values[0] = scale_factor1;
		source_values[1] = scale_factor2;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field_one->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/2, source_values,
			new Computed_field_add());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_weighted_add.  Invalid argument(s)");
		field = (Computed_field *)NULL;
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);
	LEAVE;

	return (field);
} /* Computed_field_create_weighted_add */
 
Computed_field *Computed_field_create_add(Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
{
	return(Computed_field_create_weighted_add(field_module,
		source_field_one, 1.0, source_field_two, 1.0));
} /* Computed_field_create_add */

Computed_field *Computed_field_create_subtract(Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
{
	return(Computed_field_create_weighted_add(field_module,
		source_field_one, 1.0, source_field_two, -1.0));
} /* Computed_field_create_subtract */

int Computed_field_get_type_add(struct Computed_field *field,
	struct Computed_field **source_field_one, FE_value *scale_factor1,
	struct Computed_field **source_field_two, FE_value *scale_factor2)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_ADD, the 
<source_field_one> and <source_field_two> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_add);
	if (field&&(dynamic_cast<Computed_field_add*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*scale_factor1 = field->source_values[0];
		*source_field_two = field->source_fields[1];
		*scale_factor2 = field->source_values[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_add.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_add */

int define_Computed_field_type_add(struct Parse_state *state,
	void *field_modify_void,void *computed_field_arithmetic_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_ADD (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	FE_value *scale_factors;
	int number_of_scale_factors,return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_add);
	USE_PARAMETER(computed_field_arithmetic_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		scale_factors=(FE_value *)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2)&&
			ALLOCATE(scale_factors, FE_value, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			scale_factors[0] = 1.0;
			scale_factors[1] = 1.0;
			if ((NULL != field_modify->get_field()) &&
				(computed_field_add_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_add(field_modify->get_field(), 
					source_fields, scale_factors,
					source_fields + 1, scale_factors + 1);
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
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				number_of_scale_factors=2;
				Option_table_add_entry(option_table,"scale_factors",scale_factors,
					&number_of_scale_factors,set_FE_value_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->update_field_and_deaccess(
						Computed_field_create_weighted_add(field_modify->get_field_module(),
							source_fields[0], scale_factors[0],
							source_fields[1], scale_factors[1]));
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_add.  Failed");
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
			DEALLOCATE(scale_factors);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_add.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_add.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_add */

namespace {

char computed_field_scale_type_string[] = "scale";

class Computed_field_scale : public Computed_field_core
{
public:
	Computed_field_scale() : Computed_field_core()
	{
	};

	virtual void inherit_source_field_attributes()
	{
		if (field)
		{
			Computed_field_set_coordinate_system_from_sources(field);
		}
	}

private:
	Computed_field_core *copy()
	{
		return new Computed_field_scale();
	}

	const char *get_type_string()
	{
		return(computed_field_scale_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_scale*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int set_values_at_location(Field_location* location, const FE_value *values);

	virtual int propagate_find_element_xi(const FE_value *values, int number_of_values,
		struct FE_element **element_address, FE_value *xi,
		FE_value time, Cmiss_mesh_id mesh);
};

int Computed_field_scale::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	FE_value *temp, *temp2;
	int element_dimension, i, j, return_code;

	ENTER(Computed_field_scale::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 1) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (0 != (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location)))
		{
			/* 2. Calculate the field */
			for (i=0;i<field->number_of_components;i++)
			{
				field->values[i]=
					field->source_values[i]*field->source_fields[0]->values[i];
			}
			if (field->source_fields[0]->derivatives_valid)
			{
				element_dimension=location->get_number_of_derivatives();
				temp=field->derivatives;
				temp2=field->source_fields[0]->derivatives;
				for (i=0;i<field->number_of_components;i++)
				{
					for (j=0;j<element_dimension;j++)
					{
						(*temp)=field->source_values[i]*(*temp2);
						temp++;
						temp2++;
					}
				}
				field->derivatives_valid = 1;
			}
			else
			{
				field->derivatives_valid = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_scale::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_scale::evaluate_cache_at_location */

int Computed_field_scale::set_values_at_location(
   Field_location* location, const FE_value *values)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	FE_value scale_value, *source_values;
	int k, return_code;

	ENTER(Computed_field_scale::set_values_at_location);
	if (field && location && values)
	{
		return_code=1;
		/* reverse the scaling */
		if (ALLOCATE(source_values,FE_value,field->number_of_components))
		{
			for (k=0;(k<field->number_of_components)&&return_code;k++)
			{
				scale_value=field->source_values[k];
				if (0.0 != scale_value)
				{
					source_values[k] = values[k] / scale_value;	
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_scale::set_values_at_location.  "
						"Cannot invert scale field %s with zero scale factor",
						field->name);
					return_code = 0;
				}
				if (return_code)
				{
					return_code=Computed_field_set_values_at_location(
						field->source_fields[0],location,source_values);
				}
			}
			DEALLOCATE(source_values);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_scale::set_values_at_location.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_scale::set_values_at_location */

int Computed_field_scale::propagate_find_element_xi(
	const FE_value *values, int number_of_values, struct FE_element **element_address,
	FE_value *xi, FE_value time, Cmiss_mesh_id mesh)
{
	FE_value *source_values;
	int i,return_code;

	ENTER(Computed_field_scale::propagate_find_element_xi);
	if (field && values && (number_of_values == field->number_of_components))
	{
		return_code = 1;
		/* reverse the scaling - unless any scale_factors are zero */
		if (ALLOCATE(source_values,FE_value,field->number_of_components))
		{
			for (i=0;(i<field->number_of_components)&&return_code;i++)
			{
				if (0.0 != field->source_values[i])
				{
					source_values[i] = values[i] / field->source_values[i];
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_set_values_at_node.  "
						"Cannot invert scale field %s with zero scale factor",
						field->name);
					return_code = 0;
				}
			}
			if (return_code)
			{
				return_code = Computed_field_find_element_xi(
					field->source_fields[0], source_values, number_of_values, time,
					element_address, xi, mesh, /*propagate_field*/1,
					/*find_nearest*/0);
			}
			DEALLOCATE(source_values);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_scale::propagate_find_element_xi.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_scale::propagate_find_element_xi */

int Computed_field_scale::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(List_Computed_field_scale);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    scale_factors :");
		for (i=0;i<field->source_fields[0]->number_of_components;i++)
		{
			display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
		}
		display_message(INFORMATION_MESSAGE,"\n");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_scale.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_scale */

char *Computed_field_scale::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error, i;

	ENTER(Computed_field_scale::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_scale_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " scale_factors", &error);
		for (i = 0; i < field->number_of_source_values; i++)
		{
			sprintf(temp_string, " %g", field->source_values[i]);
			append_string(&command_string, temp_string, &error);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_scale::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_scale::get_command_string */

} //namespace

Computed_field *Computed_field_create_scale(Cmiss_field_module *field_module,
	struct Computed_field *source_field, double *scale_factors)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SCALE which scales the values of the
<source_field> by <scale_factors>.
Sets the number of components equal to that of <source_field>.
Not exposed in the API as this is really just a multiply with constant
==============================================================================*/
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		source_field->number_of_components,
		/*number_of_source_fields*/1, &source_field,
		/*number_of_source_values*/source_field->number_of_components, scale_factors,
		new Computed_field_scale());

	return (field);
} /* Computed_field_create_scale */

int Computed_field_get_type_scale(struct Computed_field *field,
	struct Computed_field **source_field, double **scale_factors)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_SCALE, the 
<source_field> and <scale_factors> used by it are returned.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_get_type_scale);
	if (field&&(dynamic_cast<Computed_field_scale*>(field->core)))
	{
		if (ALLOCATE(*scale_factors,double,
			field->source_fields[0]->number_of_components))
		{
			*source_field=field->source_fields[0];
			for (i=0;i<field->source_fields[0]->number_of_components;i++)
			{
				(*scale_factors)[i] = static_cast<double>(field->source_values[i]);
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_scale.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_scale.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_scale */

int define_Computed_field_type_scale(struct Parse_state *state,
	void *field_modify_void,void *computed_field_arithmetic_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_SCALE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	const char *current_token;
	double *scale_factors, *temp_scale_factors;
	int i, number_of_scale_factors, previous_number_of_scale_factors, return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_scale);
	USE_PARAMETER(computed_field_arithmetic_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		set_source_field_data.computed_field_manager =
			field_modify->get_field_manager();
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		source_field = (struct Computed_field *)NULL;
		scale_factors = (double *)NULL;
		previous_number_of_scale_factors = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_scale_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				Computed_field_get_type_scale(field_modify->get_field(), &source_field, &scale_factors);
		}
		if (return_code)
		{
			if (source_field)
			{
				previous_number_of_scale_factors = source_field->number_of_components;
				ACCESS(Computed_field)(source_field);
			}
			if ((current_token=state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				option_table = CREATE(Option_table)();					
				Option_table_add_entry(option_table, "field", &source_field,
					&set_source_field_data, set_Computed_field_conditional);
				Option_table_add_entry(option_table, "scale_factors", scale_factors,
					&previous_number_of_scale_factors, set_double_vector);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the field ... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token, "field"))
				{
					option_table = CREATE(Option_table)();
					/* field */
					Option_table_add_entry(option_table, "field", &source_field,
						&set_source_field_data, set_Computed_field_conditional);
					if (0 != (return_code = Option_table_parse(option_table, state)))
					{
						if (source_field)
						{
							number_of_scale_factors = source_field->number_of_components;
							if (REALLOCATE(temp_scale_factors, scale_factors, double,
								number_of_scale_factors))
							{
								scale_factors = temp_scale_factors;
								/* make any new scale_factors equal to 1.0 */
								for (i = previous_number_of_scale_factors;
									i < number_of_scale_factors; i++)
								{
									scale_factors[i] = 1.0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"define_Computed_field_type_scale.  Invalid field");
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			if (return_code && !source_field)
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_scale.  Must specify a source field before the scale_factors.");
				return_code = 0;
			}
			if (return_code && !source_field)
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_scale.  "
					"You must specify the source field before the offsets.");
				return_code = 0;
			}
			/* parse the scale_factors */
			if (return_code&&state->current_token)
			{
				option_table = CREATE(Option_table)();
				number_of_scale_factors=source_field->number_of_components;
				Option_table_add_entry(option_table,"scale_factors",scale_factors,
					&number_of_scale_factors, set_double_vector);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors, not asking for help */
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_scale(field_modify->get_field_module(),
						source_field, scale_factors));
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token) &&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_scale.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
		DEALLOCATE(scale_factors);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_scale.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_scale */

namespace {

char computed_field_clamp_maximum_type_string[] = "clamp_maximum";

class Computed_field_clamp_maximum : public Computed_field_core
{
public:
	Computed_field_clamp_maximum() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_clamp_maximum();
	}

	const char *get_type_string()
	{
		return(computed_field_clamp_maximum_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_clamp_maximum*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int set_values_at_location(Field_location* location, const FE_value *values);

};

int Computed_field_clamp_maximum::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	FE_value *temp = 0, *temp2 = 0;
	int element_dimension = -1, i, j, return_code;

	ENTER(Computed_field_clamp_maximum::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 1) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (0 != (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location)))
		{
			/* 2. Calculate the field */
			if (field->source_fields[0]->derivatives_valid)
			{
				temp=field->derivatives;
				temp2=field->source_fields[0]->derivatives;
				field->derivatives_valid = 1;
				element_dimension = location->get_number_of_derivatives();
			}
			for (i=0;i<field->number_of_components;i++)
			{
				if (field->source_fields[0]->values[i] < field->source_values[i])
				{
					field->values[i]=field->source_fields[0]->values[i];
					if (field->source_fields[0]->derivatives_valid)
					{
						for (j=0;j<element_dimension;j++)
						{
							(*temp)=(*temp2);
							temp++;
							temp2++;
						}
					}
				}
				else
				{
					field->values[i]=field->source_values[i];
					if (field->source_fields[0]->derivatives_valid)
					{
						for (j=0;j<element_dimension;j++)
						{
							(*temp)=0.0;
							temp++;
							temp2++; /* To ensure that the following components match */
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clamp_maximum::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_clamp_maximum::evaluate_cache_at_location */

int Computed_field_clamp_maximum::set_values_at_location(
   Field_location* location, const FE_value *values)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	FE_value max, *source_values;
	int k, return_code;

	ENTER(Computed_field_clamp_maximum::set_values_at_location);
	if (field && location && values)
	{
		return_code=1;
		/* clamps to limits of maximums when setting values too */
		if (ALLOCATE(source_values,FE_value,field->number_of_components))
		{
			for (k=0;k<field->number_of_components;k++)
			{
				max=field->source_values[k];
				if (values[k] > max)
				{
					source_values[k] = max;
				}
				else
				{
					source_values[k] = values[k];
				}
			}
			return_code=Computed_field_set_values_at_location(
				field->source_fields[0],location,source_values);
			DEALLOCATE(source_values);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clamp_maximum::set_values_at_location.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_clamp_maximum::set_values_at_location */

int Computed_field_clamp_maximum::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(List_Computed_field_clamp_maximum);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    maximums :");
		for (i=0;i<field->source_fields[0]->number_of_components;i++)
		{
			display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
		}
		display_message(INFORMATION_MESSAGE,"\n");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_clamp_maximum.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_clamp_maximum */

char *Computed_field_clamp_maximum::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error, i;

	ENTER(Computed_field_clamp_maximum::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_clamp_maximum_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " maximums", &error);
		for (i = 0; i < field->number_of_source_values; i++)
		{
			sprintf(temp_string, " %g", field->source_values[i]);
			append_string(&command_string, temp_string, &error);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clamp_maximum::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_clamp_maximum::get_command_string */

} //namespace

Computed_field *Computed_field_create_clamp_maximum(Cmiss_field_module *field_module,
	struct Computed_field *source_field, double *maximums)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CLAMP_MAXIMUM with the supplied
<source_field> and <maximums>.  Each component is clamped by its respective limit
in <maximums>.
The <maximums> array must therefore contain as many FE_values as there are
components in <source_field>.
SAB.  I think this should be changed so that the maximums come from a source
field rather than constant maximums before it is exposed in the API.
==============================================================================*/
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		source_field->number_of_components,
		/*number_of_source_fields*/1, &source_field,
		/*number_of_source_values*/source_field->number_of_components, maximums,
		new Computed_field_clamp_maximum());

	return (field);
} /* Computed_field_create_clamp_maximum */

int Computed_field_get_type_clamp_maximum(struct Computed_field *field,
	struct Computed_field **source_field, double **maximums)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CLAMP_MAXIMUM, the 
<source_field_one> and <source_field_two> used by it are returned.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_get_type_clamp_maximum);
	if (field&&(dynamic_cast<Computed_field_clamp_maximum*>(field->core))
		&&source_field&&maximums)
	{
		if (ALLOCATE(*maximums,double,field->number_of_components))
		{
			*source_field=field->source_fields[0];
			for (i=0;i<field->number_of_components;i++)
			{
				(*maximums)[i] = static_cast<double>(field->source_values[i]);
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_clamp_maximum.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_clamp_maximum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_clamp_maximum */

int define_Computed_field_type_clamp_maximum(struct Parse_state *state,
	void *field_modify_void,void *computed_field_arithmetic_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CLAMP_MAXIMUM (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	const char *current_token;
	double *maximums, *temp_maximums;
	int i, number_of_maximums, previous_number_of_maximums, return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_clamp_maximum);
	USE_PARAMETER(computed_field_arithmetic_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		set_source_field_data.computed_field_manager =
			field_modify->get_field_manager();
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		source_field = (struct Computed_field *)NULL;
		maximums = (double *)NULL;
		previous_number_of_maximums = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_clamp_maximum_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code = Computed_field_get_type_clamp_maximum(field_modify->get_field(),
				&source_field, &maximums);
		}
		if (return_code)
		{
			if (source_field)
			{
				previous_number_of_maximums = source_field->number_of_components;
				ACCESS(Computed_field)(source_field);
			}
			if ((current_token=state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				option_table = CREATE(Option_table)();					
				Option_table_add_entry(option_table,"field",&source_field,
					&set_source_field_data,set_Computed_field_conditional);
				Option_table_add_entry(option_table,"maximums",maximums,
					&previous_number_of_maximums,set_double_vector);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the field ... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token, "field"))
				{
					option_table = CREATE(Option_table)();
					/* field */
					Option_table_add_entry(option_table, "field", &source_field,
						&set_source_field_data, set_Computed_field_conditional);
					if (0 != (return_code = Option_table_parse(option_table, state)))
					{
						if (source_field)
						{
							number_of_maximums = source_field->number_of_components;
							if (REALLOCATE(temp_maximums, maximums, double,
								number_of_maximums))
							{
								maximums = temp_maximums;
								/* make any new maximums equal to 1.0 */
								for (i = previous_number_of_maximums; i < number_of_maximums;
									i++)
								{
									maximums[i] = 1.0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"define_Computed_field_type_clamp_maximum.  Invalid field");
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			if (return_code && !source_field)
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_clamp_maximum.  "
					"You must specify the source field before the offsets.");
				return_code = 0;
			}
			/* parse the maximums */
			if (return_code && state->current_token)
			{
				option_table = CREATE(Option_table)();
				number_of_maximums = source_field->number_of_components;
				Option_table_add_entry(option_table, "maximums", maximums,
					&number_of_maximums, set_double_vector);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors, not asking for help */
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_clamp_maximum(field_modify->get_field_module(),
						source_field, maximums));
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token) &&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_clamp_maximum.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
		DEALLOCATE(maximums);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_clamp_maximum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_clamp_maximum */

namespace {

char computed_field_clamp_minimum_type_string[] = "clamp_minimum";

class Computed_field_clamp_minimum : public Computed_field_core
{
public:
	Computed_field_clamp_minimum() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_clamp_minimum();
	}

	const char *get_type_string()
	{
		return(computed_field_clamp_minimum_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_clamp_minimum*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int set_values_at_location(Field_location* location, const FE_value *values);
};

int Computed_field_clamp_minimum::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	FE_value *temp = 0, *temp2 = 0;
	int element_dimension = -1, i, j, return_code;

	ENTER(Computed_field_clamp_minimum::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 1) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (0 != (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location)))
		{
			/* 2. Calculate the field */
			if (field->source_fields[0]->derivatives_valid)
			{
				temp=field->derivatives;
				temp2=field->source_fields[0]->derivatives;
				field->derivatives_valid = 1;
				element_dimension=location->get_number_of_derivatives();
			}
			for (i=0;i<field->number_of_components;i++)
			{
				if (field->source_fields[0]->values[i] > field->source_values[i])
				{
					field->values[i]=field->source_fields[0]->values[i];
					if (field->source_fields[0]->derivatives_valid)
					{
						for (j=0;j<element_dimension;j++)
						{
							(*temp)=(*temp2);
							temp++;
							temp2++;
						}
					}
				}
				else
				{
					field->values[i]=field->source_values[i];
					if (field->source_fields[0]->derivatives_valid)
					{
						for (j=0;j<element_dimension;j++)
						{
							(*temp)=0.0;
							temp++;
							temp2++; /* To ensure that the following components match */
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clamp_minimum::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_clamp_minimum::evaluate_cache_at_location */

int Computed_field_clamp_minimum::set_values_at_location(
   Field_location* location, const FE_value *values)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	FE_value min, *source_values;
	int k, return_code;

	ENTER(Computed_field_clamp_minimum::set_values_at_location);
	if (field && location && values)
	{
		return_code=1;
		/* clamps to limits of minimums when setting values too */
		if (ALLOCATE(source_values,FE_value,field->number_of_components))
		{
			for (k=0;k<field->number_of_components;k++)
			{
				min=field->source_values[k];
				if (values[k] < min)
				{
					source_values[k] = min;
				}
				else
				{
					source_values[k] = values[k];
				}
			}
			return_code=Computed_field_set_values_at_location(
				field->source_fields[0],location,source_values);
			DEALLOCATE(source_values);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clamp_minimum::set_values_at_location.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_clamp_minimum::set_values_at_location */


int Computed_field_clamp_minimum::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(List_Computed_field_clamp_minimum);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    minimums :");
		for (i=0;i<field->source_fields[0]->number_of_components;i++)
		{
			display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
		}
		display_message(INFORMATION_MESSAGE,"\n");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_clamp_minimum.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_clamp_minimum */

char *Computed_field_clamp_minimum::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error, i;

	ENTER(Computed_field_clamp_minimum::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_clamp_minimum_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " minimums", &error);
		for (i = 0; i < field->number_of_source_values; i++)
		{
			sprintf(temp_string, " %g", field->source_values[i]);
			append_string(&command_string, temp_string, &error);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clamp_minimum::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_clamp_minimum::get_command_string */

} //namespace

Computed_field *Computed_field_create_clamp_minimum(Cmiss_field_module *field_module,
	struct Computed_field *source_field, double *minimums)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CLAMP_MINIMUM with the supplied
<source_field> and <minimums>.  Each component is clamped by its respective limit
in <minimums>.
The <minimums> array must therefore contain as many FE_values as there are
components in <source_field>.
SAB.  I think this should be changed so that the minimums come from a source
field rather than constant minimums before it is exposed in the API.
==============================================================================*/
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		source_field->number_of_components,
		/*number_of_source_fields*/1, &source_field,
		/*number_of_source_values*/source_field->number_of_components, minimums,
		new Computed_field_clamp_minimum());

	return (field);
} /* Computed_field_create_clamp_minimum */

int Computed_field_get_type_clamp_minimum(struct Computed_field *field,
	struct Computed_field **source_field, double **minimums)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CLAMP_MINIMUM, the 
<source_field_one> and <source_field_two> used by it are returned.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_get_type_clamp_minimum);
	if (field&&(dynamic_cast<Computed_field_clamp_minimum*>(field->core))
		&&source_field&&minimums)
	{
		if (ALLOCATE(*minimums, double, field->number_of_components))
		{
			*source_field=field->source_fields[0];
			for (i=0;i<field->number_of_components;i++)
			{
				(*minimums)[i] = static_cast<double>(field->source_values[i]);
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_clamp_minimum.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_clamp_minimum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_clamp_minimum */

int define_Computed_field_type_clamp_minimum(struct Parse_state *state,
	void *field_modify_void,void *computed_field_arithmetic_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CLAMP_MINIMUM (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	const char *current_token;
	double *minimums, *temp_minimums;
	int i, number_of_minimums, previous_number_of_minimums, return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_clamp_minimum);
	USE_PARAMETER(computed_field_arithmetic_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		set_source_field_data.computed_field_manager =
			field_modify->get_field_manager();
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		source_field = (struct Computed_field *)NULL;
		minimums = (double *)NULL;
		previous_number_of_minimums = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_clamp_minimum_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code = Computed_field_get_type_clamp_minimum(field_modify->get_field(),
				&source_field, &minimums);
		}
		if (return_code)
		{
			if (source_field)
			{
				previous_number_of_minimums = source_field->number_of_components;
				ACCESS(Computed_field)(source_field);
			}
			if ((current_token=state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				option_table = CREATE(Option_table)();					
				Option_table_add_entry(option_table,"field",&source_field,
					&set_source_field_data,set_Computed_field_conditional);
				Option_table_add_entry(option_table,"minimums",minimums,
					&previous_number_of_minimums,set_double_vector);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the field ... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token, "field"))
				{
					option_table = CREATE(Option_table)();
					/* field */
					Option_table_add_entry(option_table, "field", &source_field,
						&set_source_field_data, set_Computed_field_conditional);
					if (0 != (return_code = Option_table_parse(option_table, state)))
					{
						if (source_field)
						{
							number_of_minimums = source_field->number_of_components;
							if (REALLOCATE(temp_minimums, minimums, double,
								number_of_minimums))
							{
								minimums = temp_minimums;
								/* make any new minimums equal to 1.0 */
								for (i = previous_number_of_minimums; i < number_of_minimums;
									i++)
								{
									minimums[i] = 1.0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"define_Computed_field_type_clamp_minimum.  Invalid field");
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			if (return_code && !source_field)
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_clamp_minimum.  "
					"You must specify the source field before the offsets.");
				return_code = 0;
			}
			/* parse the minimums */
			if (return_code && state->current_token)
			{
				option_table = CREATE(Option_table)();
				number_of_minimums = source_field->number_of_components;
				Option_table_add_entry(option_table, "minimums", minimums,
					&number_of_minimums, set_double_vector);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors, not asking for help */
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_clamp_minimum(field_modify->get_field_module(),
						source_field, minimums));
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token) &&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_clamp_minimum.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
		DEALLOCATE(minimums);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_clamp_minimum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_clamp_minimum */

namespace {

char computed_field_offset_type_string[] = "offset";

class Computed_field_offset : public Computed_field_core
{
public:
	Computed_field_offset() : Computed_field_core()
	{
	};

	virtual void inherit_source_field_attributes()
	{
		if (field)
		{
			Computed_field_set_coordinate_system_from_sources(field);
		}
	}

private:
	Computed_field_core *copy()
	{
		return new Computed_field_offset();
	}

	const char *get_type_string()
	{
		return(computed_field_offset_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_offset*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int set_values_at_location(Field_location* location, const FE_value *values);

	virtual int propagate_find_element_xi(const FE_value *values, int number_of_values,
		struct FE_element **element_address, FE_value *xi,
		FE_value time, Cmiss_mesh_id mesh);
};

int Computed_field_offset::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	FE_value *temp, *temp2;
	int element_dimension, i, j, return_code;

	ENTER(Computed_field_offset::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 1) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (0 != (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location)))
		{
			/* 2. Calculate the field */
			for (i=0;i<field->number_of_components;i++)
			{
				field->values[i]=
					field->source_values[i]+field->source_fields[0]->values[i];
			}
			if (field->source_fields[0]->derivatives_valid)
			{
				element_dimension=location->get_number_of_derivatives();
				temp=field->derivatives;
				temp2=field->source_fields[0]->derivatives;
				for (i=0;i<field->number_of_components;i++)
				{
					for (j=0;j<element_dimension;j++)
					{
						(*temp)=(*temp2);
						temp++;
						temp2++;
					}
				}
				field->derivatives_valid = 1;
			}
			else
			{
				field->derivatives_valid = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_offset::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_offset::evaluate_cache_at_location */

int Computed_field_offset::set_values_at_location(
   Field_location* location, const FE_value *values)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	FE_value *source_values;
	int k, return_code;

	ENTER(Computed_field_offset::set_values_at_location);
	if (field && location && values)
	{
		return_code=1;
		/* reverse the offset */
		if (ALLOCATE(source_values,FE_value,field->number_of_components))
		{
			for (k=0;k<field->number_of_components;k++)
			{
				source_values[k] = values[k] - field->source_values[k];
			}
			return_code=Computed_field_set_values_at_location(
				field->source_fields[0],location,source_values);
			DEALLOCATE(source_values);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_offset::set_values_at_location.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_offset::set_values_at_location */


int Computed_field_offset::propagate_find_element_xi(
	const FE_value *values, int number_of_values, struct FE_element **element_address,
	FE_value *xi, FE_value time, Cmiss_mesh_id mesh)
{
	FE_value *source_values;
	int i,return_code;

	ENTER(Computed_field_offset::propagate_find_element_xi);
	if (field && values && (number_of_values == field->number_of_components))
	{
		return_code = 1;
		/* reverse the offset */
		if (ALLOCATE(source_values,FE_value,field->number_of_components))
		{
			for (i=0;i<field->number_of_components;i++)
			{
				source_values[i] = values[i] - field->source_values[i];
			}
			return_code = Computed_field_find_element_xi(
				field->source_fields[0], source_values, number_of_values, time,
				element_address, xi, mesh, /*propagate_field*/1,
				/*find_nearest*/0);
			DEALLOCATE(source_values);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_offset::propagate_find_element_xi.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_offset::propagate_find_element_xi */

int Computed_field_offset::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(List_Computed_field_offset);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    offsets :");
		for (i=0;i<field->source_fields[0]->number_of_components;i++)
		{
			display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
		}
		display_message(INFORMATION_MESSAGE,"\n");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_offset.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_offset */

char *Computed_field_offset::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error, i;

	ENTER(Computed_field_offset::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_offset_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " offsets", &error);
		for (i = 0; i < field->number_of_source_values; i++)
		{
			sprintf(temp_string, " %g", field->source_values[i]);
			append_string(&command_string, temp_string, &error);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_offset::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_offset::get_command_string */

} //namespace

Computed_field *Computed_field_create_offset(Cmiss_field_module *field_module,
	struct Computed_field *source_field, double *offsets)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_OFFSET which returns the values of the
<source_field> plus the <offsets>.
The <offsets> array must therefore contain as many FE_values as there are
components in <source_field>; this is the number of components in the field.
Not exposed in the API is this is just an add with constant field.
==============================================================================*/
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		source_field->number_of_components,
		/*number_of_source_fields*/1, &source_field,
		/*number_of_source_values*/source_field->number_of_components, offsets,
		new Computed_field_offset());

	return (field);
} /* Computed_field_create_offset */

int Computed_field_get_type_offset(struct Computed_field *field,
	struct Computed_field **source_field, double **offsets)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field type_string matches computed_field_offset_type_string, 
the source_field and offsets used by it are returned. Since the number of 
offsets is equal to the number of components in the source_field (and you don't 
know this yet), this function returns in *offsets a pointer to an allocated array
containing the FE_values.
It is up to the calling function to DEALLOCATE the returned <*offsets>.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_get_type_offset);
	if (field&&(dynamic_cast<Computed_field_offset*>(field->core))
		&&source_field&&offsets)
	{
		if (ALLOCATE(*offsets, double, field->number_of_components))
		{
			*source_field=field->source_fields[0];
			for (i=0;i<field->number_of_components;i++)
			{
				(*offsets)[i] = static_cast<double>(field->source_values[i]);
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_offset.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_offset.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_offset */

int define_Computed_field_type_offset(struct Parse_state *state,
	void *field_modify_void,void *computed_field_arithmetic_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_OFFSET (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	const char *current_token;
	double *offsets, *temp_offsets;
	int i, number_of_offsets, previous_number_of_offsets, return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_offset);
	USE_PARAMETER(computed_field_arithmetic_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		set_source_field_data.computed_field_manager =
			field_modify->get_field_manager();
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		source_field = (struct Computed_field *)NULL;
		offsets = (double *)NULL;
		previous_number_of_offsets = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_offset_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				Computed_field_get_type_offset(field_modify->get_field(), &source_field, &offsets);
		}
		if (return_code)
		{
			if (source_field)
			{
				previous_number_of_offsets = source_field->number_of_components;
				ACCESS(Computed_field)(source_field);
			}
			if ((current_token=state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				option_table = CREATE(Option_table)();					
				/* field */
				Option_table_add_entry(option_table, "field", &source_field,
					&set_source_field_data, set_Computed_field_conditional);
				/* offsets */
				Option_table_add_entry(option_table, "offsets", offsets,
					&previous_number_of_offsets, set_double_vector);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the field ... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token, "field"))
				{
					option_table = CREATE(Option_table)();
					/* field */
					Option_table_add_entry(option_table, "field", &source_field,
						&set_source_field_data, set_Computed_field_conditional);
					if (0 != (return_code = Option_table_parse(option_table,state)))
					{
						if (source_field)
						{
							number_of_offsets = source_field->number_of_components;
							if (REALLOCATE(temp_offsets, offsets, double,
								number_of_offsets))
							{
								offsets = temp_offsets;
								/* set new offsets to 0.0 */
								for (i = previous_number_of_offsets; i < number_of_offsets; i++)
								{
									offsets[i] = 0.0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"define_Computed_field_type_offset.  Invalid field");
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			if (return_code && !source_field)
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_offset.  "
					"You must specify the source field before the offsets.");
				return_code = 0;
			}
			/* parse the offsets */
			if (return_code && state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* offsets */
				number_of_offsets=source_field->number_of_components;
				Option_table_add_entry(option_table,"offsets",offsets,
					&number_of_offsets, set_double_vector);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_offset(field_modify->get_field_module(),
						source_field, offsets));
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_offset.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
		DEALLOCATE(offsets);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_offset.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_offset */

namespace {

char computed_field_sum_components_type_string[] = "sum_components";

class Computed_field_sum_components : public Computed_field_core
{
public:
	Computed_field_sum_components() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_sum_components();
	}

	const char *get_type_string()
	{
		return(computed_field_sum_components_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_sum_components*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};



int Computed_field_sum_components::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	FE_value sum, *temp;
	int element_dimension, i, j, return_code;

	ENTER(Computed_field_sum_components::evaluate_cache_at_location);
	if (field && location && (1 == field->number_of_components) &&
		(field->number_of_source_fields == 1) && 
		(field->number_of_source_values ==
			field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (0 != (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location)))
		{
			/* 2. Calculate the field */
			/* weighted sum of components of source field */
			temp = field->source_fields[0]->values;
			sum = 0.0;
			for (i = 0; i < field->number_of_source_values; i++)
			{
				sum += (*temp)*field->source_values[i];
				temp++;
			}
			field->values[0] = sum;
			if (field->source_fields[0]->derivatives_valid)
			{
				element_dimension = location->get_number_of_derivatives();
				for (j = 0; j < element_dimension; j++)
				{
					temp = field->source_fields[0]->derivatives + j;
					sum = 0.0;
					for (i = 0; i < field->number_of_source_values; i++)
					{
						sum += (*temp)*field->source_values[i];
						temp += element_dimension;
					}
					field->derivatives[j] = sum;
				}
				field->derivatives_valid = 1;
			}
			else
			{
				field->derivatives_valid = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sum_components::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sum_components::evaluate_cache_at_location */


int Computed_field_sum_components::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(List_Computed_field_sum_components);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    weights :");
		for (i = 0; i < field->number_of_source_values; i++)
		{
			display_message(INFORMATION_MESSAGE, " %g", field->source_values[i]);
		}
		display_message(INFORMATION_MESSAGE,"\n");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_sum_components.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_sum_components */

char *Computed_field_sum_components::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error, i;

	ENTER(Computed_field_sum_components::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_sum_components_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " weights", &error);
		for (i = 0; i < field->number_of_source_values; i++)
		{
			sprintf(temp_string, " %g", field->source_values[i]);
			append_string(&command_string, temp_string, &error);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sum_components::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_sum_components::get_command_string */

} //namespace

Computed_field *Computed_field_create_sum_components(Cmiss_field_module *field_module,
	struct Computed_field *source_field, double *weights)
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		/*number_of_components*/1,
		/*number_of_source_fields*/1, &source_field,
		/*number_of_source_values*/source_field->number_of_components, weights,
		new Computed_field_sum_components());

	return (field);
} /* Computed_field_create_sum_components */

int Computed_field_get_type_sum_components(struct Computed_field *field,
	struct Computed_field **source_field, double **weights)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_SUM_COMPONENTS, the 
<source_field> and <weights> used by it are returned.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_get_type_sum_components);
	if (field && (dynamic_cast<Computed_field_sum_components*>(field->core)))
	{
		if (ALLOCATE(*weights, double,
			field->source_fields[0]->number_of_components))
		{
			*source_field = field->source_fields[0];
			for (i = 0; i < field->source_fields[0]->number_of_components; i++)
			{
				(*weights)[i] = field->source_values[i];
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_sum_components.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_sum_components.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_sum_components */

int define_Computed_field_type_sum_components(struct Parse_state *state,
	void *field_modify_void,void *computed_field_arithmetic_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_SUM_COMPONENTS (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	const char *current_token;
	double *weights, *temp_weights;
	int i, number_of_weights, previous_number_of_weights, return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_sum_components);
	USE_PARAMETER(computed_field_arithmetic_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		set_source_field_data.computed_field_manager =
			field_modify->get_field_manager();
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		source_field = (struct Computed_field *)NULL;
		weights = (double *)NULL;
		previous_number_of_weights = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_sum_components_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				Computed_field_get_type_sum_components(field_modify->get_field(), &source_field, &weights);
		}
		if (return_code)
		{
			if (source_field)
			{
				previous_number_of_weights = source_field->number_of_components;
				ACCESS(Computed_field)(source_field);
			}
			if ((current_token = state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING, current_token) &&
					strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))))
			{
				option_table = CREATE(Option_table)();					
				Option_table_add_entry(option_table, "field", &source_field,
					&set_source_field_data, set_Computed_field_conditional);
				Option_table_add_double_vector_entry(option_table,
					"weights", weights, &previous_number_of_weights);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the field ... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token, "field"))
				{
					option_table = CREATE(Option_table)();
					/* field */
					Option_table_add_entry(option_table, "field", &source_field,
						&set_source_field_data, set_Computed_field_conditional);
					if (0 != (return_code = Option_table_parse(option_table, state)))
					{
						if (source_field)
						{
							number_of_weights = source_field->number_of_components;
							if (REALLOCATE(temp_weights, weights, double,
								number_of_weights))
							{
								weights = temp_weights;
								/* make any new weights equal to 1.0 */
								for (i = previous_number_of_weights; i < number_of_weights; i++)
								{
									weights[i] = 1.0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"define_Computed_field_type_sum_components.  Invalid field");
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			if (return_code && !source_field)
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_sum_components.  "
					"You must specify the source field before the offsets.");
				return_code = 0;
			}
			/* parse the weights */
			if (return_code && state->current_token)
			{
				option_table = CREATE(Option_table)();
				number_of_weights = source_field->number_of_components;
				Option_table_add_double_vector_entry(option_table,
					"weights", weights, &number_of_weights);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_sum_components(field_modify->get_field_module(),
						source_field, weights));
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token) &&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_sum_components.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
		DEALLOCATE(weights);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_sum_components.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_sum_components */

namespace {

char computed_field_edit_mask_type_string[] = "edit_mask";

class Computed_field_edit_mask : public Computed_field_core
{
public:
	Computed_field_edit_mask() : Computed_field_core()
	{
	};

	virtual void inherit_source_field_attributes()
	{
		if (field)
		{
			Computed_field_set_coordinate_system_from_sources(field);
		}
	}

private:
	Computed_field_core *copy()
	{
		return new Computed_field_edit_mask();
	}

	const char *get_type_string()
	{
		return(computed_field_edit_mask_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_edit_mask*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int set_values_at_location(Field_location* location, const FE_value *values);
};

int Computed_field_edit_mask::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	FE_value *temp, *temp2;
	int element_dimension, i, j, return_code;

	ENTER(Computed_field_edit_mask::evaluate_cache_at_location);
	if (field && location &&
		(dynamic_cast<Computed_field_edit_mask*>(field->core)))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (0 != (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location)))
		{
			/* 2. Calculate the field */
			/* exact copy of source field */
			temp = field->source_fields[0]->values;
			for (i = 0; i < field->number_of_components; i++)
			{
				field->values[i] = temp[i];
			}
			if (field->source_fields[0]->derivatives_valid)
			{
				element_dimension = location->get_number_of_derivatives();
				temp = field->derivatives;
				temp2 = field->source_fields[0]->derivatives;
				for (i = 0; i < field->number_of_components; i++)
				{
					for (j = 0; j < element_dimension; j++)
					{
						(*temp) = (*temp2);
						temp++;
						temp2++;
					}
				}
				field->derivatives_valid = 1;
			}
			else
			{
				field->derivatives_valid = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_edit_mask::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_edit_mask::evaluate_cache_at_location */

int Computed_field_edit_mask::set_values_at_location(
   Field_location* location, const FE_value *values)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	FE_value *source_values;
	int k, return_code;

	ENTER(Computed_field_edit_mask::set_values_at_location);
	if (field && location && values)
	{
		return_code = 1;
		if (ALLOCATE(source_values, FE_value, field->source_fields[0]->number_of_components))
		{
			/* need current field values to partially set */
			if (Computed_field_evaluate_cache_at_location(field->source_fields[0],
					location))
			{
				/* insert the components with mask on into this array */
				for (k = 0; k < field->number_of_components; k++)
				{
					if (field->source_values[k])
					{
						source_values[k] = values[k];
					}
					else
					{
						source_values[k] = field->source_fields[0]->values[k];
					}
				}
				return_code = Computed_field_set_values_at_location(
					field->source_fields[0], location, source_values);
			}
			else
			{
				return_code = 0;
			}
			DEALLOCATE(source_values);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_edit_mask::set_values_at_location.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_edit_mask::set_values_at_location */


int Computed_field_edit_mask::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(List_Computed_field_edit_mask);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    edit mask :");
		for (i = 0; i < field->number_of_source_values; i++)
		{
			display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
		}
		display_message(INFORMATION_MESSAGE,"\n");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_edit_mask.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_edit_mask */

char *Computed_field_edit_mask::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error, i;

	ENTER(Computed_field_edit_mask::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_edit_mask_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " edit_mask", &error);
		for (i = 0; i < field->number_of_source_values; i++)
		{
			sprintf(temp_string, " %g", field->source_values[i]);
			append_string(&command_string, temp_string, &error);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_edit_mask::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_edit_mask::get_command_string */

} //namespace

Computed_field *Computed_field_create_edit_mask(Cmiss_field_module *field_module,
	struct Computed_field *source_field, double *edit_mask)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_EDIT_MASK, returning the <source_field>
with each component edit_masked by its respective FE_value in <edit_mask>, ie.
if the edit_mask value for a component is non-zero, the component is editable.
The <edit_mask> array must therefore contain as many FE_values as there are
components in <source_field>.
Sets the number of components to the same as <source_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		source_field->number_of_components,
		/*number_of_source_fields*/1, &source_field,
		/*number_of_source_values*/source_field->number_of_components, edit_mask,
		new Computed_field_edit_mask());

	return (field);
} /* Computed_field_create_edit_mask */

int Computed_field_get_type_edit_mask(struct Computed_field *field,
	struct Computed_field **source_field, double **edit_mask)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_EDIT_MASK, the 
<source_field> and <edit_mask> used by it are returned. Since the number of
edit_mask values is equal to the number of components in the source_field, and
you don't know this yet, this function returns in *edit_mask a pointer to an
allocated array containing the FE_values.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_get_type_edit_mask);
	if (field && (dynamic_cast<Computed_field_edit_mask*>(field->core)))
	{
		if (ALLOCATE(*edit_mask, double,
			field->source_fields[0]->number_of_components))
		{
			*source_field = field->source_fields[0];
			for (i = 0; i < field->source_fields[0]->number_of_components; i++)
			{
				(*edit_mask)[i] = static_cast<double>(field->source_values[i]);
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_edit_mask.  Could not allocate edit masks");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_edit_mask.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_edit_mask */

int define_Computed_field_type_edit_mask(struct Parse_state *state,
	void *field_modify_void, void *computed_field_arithmetic_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_EDIT_MASK (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	const char *current_token;
	double *edit_mask, *temp_edit_mask;
	int i, number_of_edit_mask, previous_number_of_edit_mask, return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_edit_mask);
	USE_PARAMETER(computed_field_arithmetic_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		set_source_field_data.computed_field_manager =
			field_modify->get_field_manager();
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		source_field = (struct Computed_field *)NULL;
		edit_mask = (double *)NULL;
		previous_number_of_edit_mask = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_edit_mask_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				Computed_field_get_type_edit_mask(field_modify->get_field(), &source_field, &edit_mask);
		}
		if (return_code)
		{
			if (source_field)
			{
				previous_number_of_edit_mask = source_field->number_of_components;
				ACCESS(Computed_field)(source_field);
			}
			if ((current_token = state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING, current_token) &&
					strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))))
			{
				option_table = CREATE(Option_table)();					
				Option_table_add_entry(option_table, "field", &source_field,
					&set_source_field_data, set_Computed_field_conditional);
				Option_table_add_entry(option_table, "edit_mask", edit_mask,
					&previous_number_of_edit_mask, set_double_vector);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the field ... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token, "field"))
				{
					option_table = CREATE(Option_table)();
					/* field */
					Option_table_add_entry(option_table, "field", &source_field,
						&set_source_field_data, set_Computed_field_conditional);
					if (0 != (return_code = Option_table_parse(option_table, state)))
					{
						if (source_field)
						{
							number_of_edit_mask = source_field->number_of_components;
							if (REALLOCATE(temp_edit_mask, edit_mask, double,
								number_of_edit_mask))
							{
								edit_mask = temp_edit_mask;
								/* make any new edit_mask equal to 1.0 */
								for (i = previous_number_of_edit_mask; i < number_of_edit_mask;
									i++)
								{
									edit_mask[i] = 1.0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"define_Computed_field_type_edit_mask.  Invalid field");
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			if (return_code && !source_field)
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_edit_mask.  "
					"You must specify the source field before the offsets.");
				return_code = 0;
			}
			/* parse the edit_mask */
			if (return_code && state->current_token)
			{
				option_table = CREATE(Option_table)();
				number_of_edit_mask = source_field->number_of_components;
				Option_table_add_entry(option_table, "edit_mask", edit_mask,
					&number_of_edit_mask, set_double_vector);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_edit_mask(field_modify->get_field_module(),
						source_field, edit_mask));
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token) &&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_edit_mask.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
		DEALLOCATE(edit_mask);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_edit_mask.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_edit_mask */

namespace {

char computed_field_log_type_string[] = "log";

class Computed_field_log : public Computed_field_core
{
public:
	Computed_field_log() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_log();
	}

	const char *get_type_string()
	{
		return(computed_field_log_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_log*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

int Computed_field_log::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_log::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (0 != (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location)))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)log((double)(field->source_fields[0]->values[i]));
			}
			if (field->source_fields[0]->derivatives_valid
				&& (0 < (number_of_xi = location->get_number_of_derivatives())))
			{
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						/* d(log u)/dx = 1 / u * du/dx */
						*derivative = 1.0 / field->source_fields[0]->values[i] *
							field->source_fields[0]->derivatives[i * number_of_xi + j];
						derivative++;
					}
				}
				field->derivatives_valid = 1;
			}
			else
			{
				field->derivatives_valid = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_log::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_log::evaluate_cache_at_location */


int Computed_field_log::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_log);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_log.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_log */

char *Computed_field_log::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_log::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_log_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_log::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_log::get_command_string */

} //namespace

Computed_field *Computed_field_create_log(Cmiss_field_module *field_module,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_LOG with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		source_field->number_of_components,
		/*number_of_source_fields*/1, &source_field,
		/*number_of_source_values*/0, NULL,
		new Computed_field_log());

	return (field);
} /* Computed_field_create_log */

int Computed_field_get_type_log(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_LOG, the 
<source_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_log);
	if (field&&(dynamic_cast<Computed_field_log*>(field->core)))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_log.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_log */

int define_Computed_field_type_log(struct Parse_state *state,
	void *field_modify_void,void *computed_field_arithmetic_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_LOG (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_log);
	USE_PARAMETER(computed_field_arithmetic_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if ((NULL != field_modify->get_field()) &&
				(computed_field_log_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_log(field_modify->get_field(), 
					source_fields);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					field_modify->get_field_manager();
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=1;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"field",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->update_field_and_deaccess(
						Computed_field_create_log(field_modify->get_field_module(),
							source_fields[0]));
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_log.  Failed");
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
				"define_Computed_field_type_log.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_log.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_log */

namespace {

char computed_field_sqrt_type_string[] = "sqrt";

class Computed_field_sqrt : public Computed_field_core
{
public:
	Computed_field_sqrt() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_sqrt();
	}

	const char *get_type_string()
	{
		return(computed_field_sqrt_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_sqrt*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

int Computed_field_sqrt::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_sqrt::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (0 != (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location)))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)sqrt((double)(field->source_fields[0]->values[i]));
			}
			if (field->source_fields[0]->derivatives_valid
				&& (0 < (number_of_xi = location->get_number_of_derivatives())))
			{
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						/* d(sqrt u)/dx = du/dx / 2 sqrt(u) */
						*derivative = 
							field->source_fields[0]->derivatives[i * number_of_xi + j]
							/ ( 2 * field->values[i] );
						derivative++;
					}
				}
				field->derivatives_valid = 1;
			}
			else
			{
				field->derivatives_valid = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sqrt::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sqrt::evaluate_cache_at_location */


int Computed_field_sqrt::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_sqrt);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_sqrt.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_sqrt */

char *Computed_field_sqrt::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_sqrt::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_sqrt_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sqrt::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_sqrt::get_command_string */

} //namespace

Computed_field *Computed_field_create_sqrt(Cmiss_field_module *field_module,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SQRT with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		source_field->number_of_components,
		/*number_of_source_fields*/1, &source_field,
		/*number_of_source_values*/0, NULL,
		new Computed_field_sqrt());

	return (field);
} /* Computed_field_create_sqrt */

int Computed_field_get_type_sqrt(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_SQRT, the 
<source_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_sqrt);
	if (field&&(dynamic_cast<Computed_field_sqrt*>(field->core)))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_sqrt.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_sqrt */

int define_Computed_field_type_sqrt(struct Parse_state *state,
	void *field_modify_void,void *computed_field_arithmetic_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_SQRT (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_sqrt);
	USE_PARAMETER(computed_field_arithmetic_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if ((NULL != field_modify->get_field()) &&
				(computed_field_sqrt_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
		{
				return_code=Computed_field_get_type_sqrt(field_modify->get_field(), 
					source_fields);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					field_modify->get_field_manager();
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=1;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"field",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->update_field_and_deaccess(
						Computed_field_create_sqrt(field_modify->get_field_module(),
							source_fields[0]));
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_sqrt.  Failed");
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
				"define_Computed_field_type_sqrt.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_sqrt.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_sqrt */

namespace {

char computed_field_exp_type_string[] = "exp";

class Computed_field_exp : public Computed_field_core
{
public:
	Computed_field_exp() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_exp();
	}

	const char *get_type_string()
	{
		return(computed_field_exp_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_exp*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

int Computed_field_exp::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_exp::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (0 != (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location)))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)exp((double)(field->source_fields[0]->values[i]));
			}
			if (field->source_fields[0]->derivatives_valid
				&& (0 < (number_of_xi = location->get_number_of_derivatives())))
			{
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						/* d(exp u)/dx = du/dx exp(u) */
						*derivative = 
							field->source_fields[0]->derivatives[i * number_of_xi + j]
							* field->values[i];
						derivative++;
					}
				}
				field->derivatives_valid = 1;
			}
			else
			{
				field->derivatives_valid = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_exp::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_exp::evaluate_cache_at_location */


int Computed_field_exp::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_exp);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_exp.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_exp */

char *Computed_field_exp::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_exp::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_exp_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_exp::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_exp::get_command_string */

} //namespace

Computed_field *Computed_field_create_exp(Cmiss_field_module *field_module,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_EXP with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		source_field->number_of_components,
		/*number_of_source_fields*/1, &source_field,
		/*number_of_source_values*/0, NULL,
		new Computed_field_exp());

	return (field);
} /* Computed_field_create_exp */

int Computed_field_get_type_exp(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_EXP, the 
<source_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_exp);
	if (field&&(dynamic_cast<Computed_field_exp*>(field->core)))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_exp.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_exp */

int define_Computed_field_type_exp(struct Parse_state *state,
	void *field_modify_void,void *computed_field_arithmetic_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_EXP (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_exp);
	USE_PARAMETER(computed_field_arithmetic_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if ((NULL != field_modify->get_field()) &&
				(computed_field_exp_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_exp(field_modify->get_field(), 
					source_fields);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					field_modify->get_field_manager();
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=1;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"field",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->update_field_and_deaccess(
						Computed_field_create_exp(field_modify->get_field_module(),
							source_fields[0]));
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_exp.  Failed");
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
				"define_Computed_field_type_exp.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_exp.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_exp */

namespace {

char computed_field_abs_type_string[] = "abs";

class Computed_field_abs : public Computed_field_core
{
public:
	Computed_field_abs() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_abs();
	}

	const char *get_type_string()
	{
		return(computed_field_abs_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_abs*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

int Computed_field_abs::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_abs::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (0 != (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location)))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)fabs((double)(field->source_fields[0]->values[i]));
			}
			if (field->source_fields[0]->derivatives_valid
				&& (0 < (number_of_xi = location->get_number_of_derivatives())))
			{
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					/* d(abs u)/dx = du/dx u>0
					 *               -du/dx u<0
					 *               and lets just put 0 at u=0 */
					if (field->source_fields[0]->values[i] > 0)
					{
						for (j = 0 ; j < number_of_xi ; j++)
						{
							*derivative = field->source_fields[0]->derivatives[i * number_of_xi + j];
							derivative++;
						}
					}
					else if (field->source_fields[0]->values[i] < 0)
					{
						for (j = 0 ; j < number_of_xi ; j++)
						{
							*derivative = -field->source_fields[0]->derivatives[i * number_of_xi + j];
							derivative++;
						}
					}
					else
					{
						for (j = 0 ; j < number_of_xi ; j++)
						{
							*derivative = 0.0;
							derivative++;
						}
					}
				}
				field->derivatives_valid = 1;
			}
			else
			{
				field->derivatives_valid = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_abs::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_abs::evaluate_cache_at_location */


int Computed_field_abs::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_abs);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_abs.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_abs */

char *Computed_field_abs::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_abs::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_abs_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_abs::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_abs::get_command_string */

} //namespace

Computed_field *Computed_field_create_abs(Cmiss_field_module *field_module,
	struct Computed_field *source_field)
/*******************************************************************************
DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_EXP with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		source_field->number_of_components,
		/*number_of_source_fields*/1, &source_field,
		/*number_of_source_values*/0, NULL,
		new Computed_field_abs());

	return (field);
} /* Computed_field_create_abs */

int Computed_field_get_type_abs(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
DESCRIPTION :
If the field is of type COMPUTED_FIELD_EXP, the 
<source_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_abs);
	if (field&&(dynamic_cast<Computed_field_abs*>(field->core)))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_abs.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_abs */

int define_Computed_field_type_abs(struct Parse_state *state,
	void *field_modify_void,void *computed_field_arithmetic_operators_package_void)
/*******************************************************************************
DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_EXP (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_abs);
	USE_PARAMETER(computed_field_arithmetic_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if ((NULL != field_modify->get_field()) &&
				(computed_field_abs_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_abs(field_modify->get_field(), 
					source_fields);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					field_modify->get_field_manager();
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=1;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"field",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->update_field_and_deaccess(
						Computed_field_create_abs(field_modify->get_field_module(),
							source_fields[0]));
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_abs.  Failed");
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
				"define_Computed_field_type_abs.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_abs.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_abs */

int Computed_field_register_types_arithmetic_operators(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_arithmetic_operators_package
		*computed_field_arithmetic_operators_package = 
		new Computed_field_arithmetic_operators_package;

	ENTER(Computed_field_register_types_arithmetic_operators);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_power_type_string, 
			define_Computed_field_type_power,
			computed_field_arithmetic_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_multiply_components_type_string, 
			define_Computed_field_type_multiply_components,
			computed_field_arithmetic_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_divide_components_type_string, 
			define_Computed_field_type_divide_components,
			computed_field_arithmetic_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_add_type_string,
			define_Computed_field_type_add,
			computed_field_arithmetic_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_scale_type_string,
			define_Computed_field_type_scale,
			computed_field_arithmetic_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_clamp_maximum_type_string, 
			define_Computed_field_type_clamp_maximum,
			computed_field_arithmetic_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_clamp_minimum_type_string, 
			define_Computed_field_type_clamp_minimum,
			computed_field_arithmetic_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_offset_type_string, 
			define_Computed_field_type_offset,
			computed_field_arithmetic_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_sum_components_type_string, 
			define_Computed_field_type_sum_components,
			computed_field_arithmetic_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_edit_mask_type_string, 
			define_Computed_field_type_edit_mask,
			computed_field_arithmetic_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_log_type_string, 
			define_Computed_field_type_log,
			computed_field_arithmetic_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_sqrt_type_string, 
			define_Computed_field_type_sqrt,
			computed_field_arithmetic_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_exp_type_string, 
			define_Computed_field_type_exp,
			computed_field_arithmetic_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_abs_type_string, 
			define_Computed_field_type_abs,
			computed_field_arithmetic_operators_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_arithmetic_operators.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_arithmetic_operators */

