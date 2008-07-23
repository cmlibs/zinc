/*******************************************************************************
FILE : computed_field_trigonometry.c

LAST MODIFIED : 24 August 2006

DESCRIPTION :
Implements a number of basic component wise operations on computed fields.
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
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_trigonometry.h"
}

class Computed_field_component_operations_package : public Computed_field_type_package
{
};

namespace {

char computed_field_sin_type_string[] = "sin";

class Computed_field_sin : public Computed_field_core
{
public:
	Computed_field_sin(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_sin(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_sin_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_sin*>(other_field))
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

int Computed_field_sin::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_sin::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)sin((double)(field->source_fields[0]->values[i]));
			}
			if (field->source_fields[0]->derivatives_valid
				&& (0 < (number_of_xi = location->get_number_of_derivatives())))
			{
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						/* d(sin u)/dx = cos u * du/dx */
						*derivative =
							(FE_value)(cos((double)(field->source_fields[0]->values[i])) *
							field->source_fields[0]->derivatives[i * number_of_xi + j]);
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
			"Computed_field_sin::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sin::evaluate_cache_at_location */


int Computed_field_sin::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_sin);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_sin.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_sin */

char *Computed_field_sin::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_sin::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_sin_type_string, &error);
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
			"Computed_field_sin::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_sin::get_command_string */

} //namespace

int Computed_field_set_type_sin(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SIN with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_sin);
	if (field && source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->core = new Computed_field_sin(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_sin.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_sin */

int Computed_field_get_type_sin(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_SIN, the 
<source_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_sin);
	if (field&&(dynamic_cast<Computed_field_sin*>(field->core)))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_sin.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_sin */

int define_Computed_field_type_sin(struct Parse_state *state,
	void *field_modify_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_SIN (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_sin);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
			(field=field_modify->field)&&
		(computed_field_component_operations_package=
		(Computed_field_component_operations_package *)
		computed_field_component_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if (computed_field_sin_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_sin(field, 
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
					Cmiss_region_get_Computed_field_manager(field_modify->region);
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
					return_code = Computed_field_set_type_sin(field,
						source_fields[0]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_sin.  Failed");
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
				"define_Computed_field_type_sin.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_sin.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_sin */

namespace {

char computed_field_cos_type_string[] = "cos";

class Computed_field_cos : public Computed_field_core
{
public:
	Computed_field_cos(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_cos(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_cos_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_cos*>(other_field))
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

int Computed_field_cos::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_cos::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)cos((double)(field->source_fields[0]->values[i]));
			}
			if (field->source_fields[0]->derivatives_valid
				&& (0 < (number_of_xi = location->get_number_of_derivatives())))
			{
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						/* d(cos u)/dx = -sin u * du/dx */
						*derivative =
							(FE_value)(-sin((double)(field->source_fields[0]->values[i])) *
							field->source_fields[0]->derivatives[i * number_of_xi + j]);
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
			"Computed_field_cos::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cos::evaluate_cache_at_location */


int Computed_field_cos::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_cos);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_cos.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_cos */

char *Computed_field_cos::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_cos::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_cos_type_string, &error);
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
			"Computed_field_cos::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_cos::get_command_string */

} //namespace

int Computed_field_set_type_cos(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_COS with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_cos);
	if (field && source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->core = new Computed_field_cos(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_cos.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_cos */

int Computed_field_get_type_cos(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_COS, the 
<source_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_cos);
	if (field&&(dynamic_cast<Computed_field_cos*>(field->core)))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_cos.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_cos */

int define_Computed_field_type_cos(struct Parse_state *state,
	void *field_modify_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_COS (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_cos);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
			(field=field_modify->field)&&
		(computed_field_component_operations_package=
		(Computed_field_component_operations_package *)
		computed_field_component_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if (computed_field_cos_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_cos(field, 
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
					Cmiss_region_get_Computed_field_manager(field_modify->region);
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
					return_code = Computed_field_set_type_cos(field,
						source_fields[0]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_cos.  Failed");
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
				"define_Computed_field_type_cos.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_cos.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_cos */

namespace {

char computed_field_tan_type_string[] = "tan";

class Computed_field_tan : public Computed_field_core
{
public:
	Computed_field_tan(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_tan(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_tan_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_tan*>(other_field))
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

int Computed_field_tan::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_tan::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)tan((double)(field->source_fields[0]->values[i]));
			}
			if (field->source_fields[0]->derivatives_valid
				&& (0 < (number_of_xi = location->get_number_of_derivatives())))
			{
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						/* d(tan u)/dx = sec^2 u * du/dx */
						*derivative = (FE_value)(field->source_fields[0]->derivatives[i * number_of_xi + j] /
							(cos((double)(field->source_fields[0]->values[i])) * 
							cos((double)(field->source_fields[0]->values[i]))));
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
			"Computed_field_tan::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_tan::evaluate_cache_at_location */


int Computed_field_tan::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_tan);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_tan.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_tan */

char *Computed_field_tan::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_tan::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_tan_type_string, &error);
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
			"Computed_field_tan::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_tan::get_command_string */

} //namespace

int Computed_field_set_type_tan(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_TAN with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_tan);
	if (field && source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->core = new Computed_field_tan(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_tan.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_tan */

int Computed_field_get_type_tan(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_TAN, the 
<source_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_tan);
	if (field&&(dynamic_cast<Computed_field_tan*>(field->core)))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_tan.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_tan */

int define_Computed_field_type_tan(struct Parse_state *state,
	void *field_modify_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_TAN (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_tan);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
			(field=field_modify->field)&&
		(computed_field_component_operations_package=
		(Computed_field_component_operations_package *)
		computed_field_component_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if (computed_field_tan_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_tan(field, 
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
					Cmiss_region_get_Computed_field_manager(field_modify->region);
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
					return_code = Computed_field_set_type_tan(field,
						source_fields[0]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_tan.  Failed");
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
				"define_Computed_field_type_tan.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_tan.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_tan */

namespace {

char computed_field_asin_type_string[] = "asin";

class Computed_field_asin : public Computed_field_core
{
public:
	Computed_field_asin(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_asin(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_asin_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_asin*>(other_field))
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

int Computed_field_asin::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_asin::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)asin((double)(field->source_fields[0]->values[i]));
			}
			if (field->source_fields[0]->derivatives_valid
				&& (0 < (number_of_xi = location->get_number_of_derivatives())))
			{
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						/* d(asin u)/dx = 1.0/sqrt(1.0 - u^2) * du/dx */
						if (field->source_fields[0]->values[i] != 1.0)
						{
							*derivative =
								(FE_value)(field->source_fields[0]->derivatives[i * number_of_xi + j] /
								sqrt(1.0 - ((double)field->source_fields[0]->values[i] * 
								(double)field->source_fields[0]->values[i])));
						}
						else
						{
							*derivative = 0.0;
						}
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
			"Computed_field_asin::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_asin::evaluate_cache_at_location */


int Computed_field_asin::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_asin);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_asin.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_asin */

char *Computed_field_asin::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_asin::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_asin_type_string, &error);
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
			"Computed_field_asin::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_asin::get_command_string */

} //namespace

int Computed_field_set_type_asin(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ASIN with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_asin);
	if (field && source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->core = new Computed_field_asin(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_asin.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_asin */

int Computed_field_get_type_asin(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_ASIN, the 
<source_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_asin);
	if (field&&(dynamic_cast<Computed_field_asin*>(field->core)))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_asin.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_asin */

int define_Computed_field_type_asin(struct Parse_state *state,
	void *field_modify_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_ASIN (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_asin);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
			(field=field_modify->field)&&
		(computed_field_component_operations_package=
		(Computed_field_component_operations_package *)
		computed_field_component_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if (computed_field_asin_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_asin(field, 
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
					Cmiss_region_get_Computed_field_manager(field_modify->region);
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
					return_code = Computed_field_set_type_asin(field,
						source_fields[0]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_asin.  Failed");
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
				"define_Computed_field_type_asin.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_asin.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_asin */

namespace {

char computed_field_acos_type_string[] = "acos";

class Computed_field_acos : public Computed_field_core
{
public:
	Computed_field_acos(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_acos(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_acos_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_acos*>(other_field))
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

int Computed_field_acos::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_acos::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)acos((double)(field->source_fields[0]->values[i]));
			}
			if (field->source_fields[0]->derivatives_valid
				&& (0 < (number_of_xi = location->get_number_of_derivatives())))
			{
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						/* d(asin u)/dx = -1.0/sqrt(1.0 - u^2) * du/dx */
						if (field->source_fields[0]->values[i] != 1.0)
						{
							*derivative =
								(FE_value)(-field->source_fields[0]->derivatives[i * number_of_xi + j] /
								sqrt(1.0 - ((double)field->source_fields[0]->values[i] * 
								(double)field->source_fields[0]->values[i])));
						}
						else
						{
							*derivative = 0.0;
						}
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
			"Computed_field_acos::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_acos::evaluate_cache_at_location */


int Computed_field_acos::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_acos);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_acos.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_acos */

char *Computed_field_acos::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_acos::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_acos_type_string, &error);
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
			"Computed_field_acos::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_acos::get_command_string */

} //namespace

int Computed_field_set_type_acos(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ACOS with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_acos);
	if (field && source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->core = new Computed_field_acos(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_acos.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_acos */

int Computed_field_get_type_acos(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_ACOS, the 
<source_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_acos);
	if (field&&(dynamic_cast<Computed_field_acos*>(field->core)))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_acos.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_acos */

int define_Computed_field_type_acos(struct Parse_state *state,
	void *field_modify_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_ACOS (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_acos);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
			(field=field_modify->field)&&
		(computed_field_component_operations_package=
		(Computed_field_component_operations_package *)
		computed_field_component_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if (computed_field_acos_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_acos(field, 
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
					Cmiss_region_get_Computed_field_manager(field_modify->region);
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
					return_code = Computed_field_set_type_acos(field,
						source_fields[0]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_acos.  Failed");
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
				"define_Computed_field_type_acos.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_acos.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_acos */

namespace {

char computed_field_atan_type_string[] = "atan";

class Computed_field_atan : public Computed_field_core
{
public:
	Computed_field_atan(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_atan(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_atan_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_atan*>(other_field))
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

int Computed_field_atan::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_atan::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)atan((double)(field->source_fields[0]->values[i]));
			}
			if (field->source_fields[0]->derivatives_valid
				&& (0 < (number_of_xi = location->get_number_of_derivatives())))
			{
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						/* d(atan u)/dx = 1.0/(1.0 + u^2) * du/dx */
						*derivative =
							(FE_value)(field->source_fields[0]->derivatives[i * number_of_xi + j] /
							(1.0 + ((double)field->source_fields[0]->values[i] * 
							(double)field->source_fields[0]->values[i])));
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
			"Computed_field_atan::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_atan::evaluate_cache_at_location */


int Computed_field_atan::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_atan);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_atan.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_atan */

char *Computed_field_atan::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_atan::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_atan_type_string, &error);
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
			"Computed_field_atan::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_atan::get_command_string */

} //namespace

int Computed_field_set_type_atan(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ATAN with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_atan);
	if (field && source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->core = new Computed_field_atan(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_atan.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_atan */

int Computed_field_get_type_atan(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_ATAN, the 
<source_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_atan);
	if (field&&(dynamic_cast<Computed_field_atan*>(field->core)))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_atan.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_atan */

int define_Computed_field_type_atan(struct Parse_state *state,
	void *field_modify_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_ATAN (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_atan);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
			(field=field_modify->field)&&
		(computed_field_component_operations_package=
		(Computed_field_component_operations_package *)
		computed_field_component_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if (computed_field_atan_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_atan(field, 
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
					Cmiss_region_get_Computed_field_manager(field_modify->region);
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
					return_code = Computed_field_set_type_atan(field,
						source_fields[0]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_atan.  Failed");
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
				"define_Computed_field_type_atan.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_atan.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_atan */

namespace {

char computed_field_atan2_type_string[] = "atan2";

class Computed_field_atan2 : public Computed_field_core
{
public:
	Computed_field_atan2(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_atan2(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_atan2_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_atan2*>(other_field))
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

int Computed_field_atan2::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	double u, v;
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_atan2::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields > 0) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)atan2((double)(field->source_fields[0]->values[i]),
					(double)(field->source_fields[1]->values[i]));
			}
			if (field->source_fields[0]->derivatives_valid
				&& field->source_fields[1]->derivatives_valid
				&& (0 < (number_of_xi = location->get_number_of_derivatives())))
			{
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						/* d(atan (u/v))/dx =  ( v * du/dx - u * dv/dx ) / ( v^2 * u^2 ) */
						u = field->source_fields[0]->values[i];
						v = field->source_fields[1]->values[i];
						*derivative =
							(FE_value)(v * field->source_fields[0]->derivatives[i * number_of_xi + j] -
							u * field->source_fields[1]->derivatives[i * number_of_xi + j]) /
							(u * u + v * v);
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
			"Computed_field_atan2::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_atan2::evaluate_cache_at_location */


int Computed_field_atan2::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_atan2);
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
			"list_Computed_field_atan2.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_atan2 */

char *Computed_field_atan2::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_atan2::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_atan2_type_string, &error);
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
			"Computed_field_atan2::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_atan2::get_command_string */

} //namespace

int Computed_field_set_type_atan2(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ATAN2 with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_atan2);
	if (field&&source_field_one&&source_field_two&&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components = source_field_one->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field_one);
			source_fields[1]=ACCESS(Computed_field)(source_field_two);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->core = new Computed_field_atan2(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_atan2.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_atan2 */

int Computed_field_get_type_atan2(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_ATAN2, the 
<source_field_one> and <source_field_two> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_atan2);
	if (field&&(dynamic_cast<Computed_field_atan2*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_atan2.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_atan2 */

int define_Computed_field_type_atan2(struct Parse_state *state,
	void *field_modify_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_ATAN2 (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_atan2);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
			(field=field_modify->field)&&
		(computed_field_component_operations_package=
		(Computed_field_component_operations_package *)
		computed_field_component_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_atan2_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_atan2(field, 
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
					Cmiss_region_get_Computed_field_manager(field_modify->region);
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
					return_code = Computed_field_set_type_atan2(field,
						source_fields[0], source_fields[1]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_atan2.  Failed");
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
				"define_Computed_field_type_atan2.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_atan2.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_atan2 */

int Computed_field_register_types_trigonometry(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_component_operations_package
		*computed_field_component_operations_package =
		new Computed_field_component_operations_package;

	ENTER(Computed_field_register_types_component_operations);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_sin_type_string, 
			define_Computed_field_type_sin,
			computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_cos_type_string, 
			define_Computed_field_type_cos,
			computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_tan_type_string, 
			define_Computed_field_type_tan,
			computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_asin_type_string, 
			define_Computed_field_type_asin,
			computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_acos_type_string, 
			define_Computed_field_type_acos,
			computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_atan_type_string, 
			define_Computed_field_type_atan,
			computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_atan2_type_string, 
			define_Computed_field_type_atan2,
			computed_field_component_operations_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_component_operations.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_trigonometry */

