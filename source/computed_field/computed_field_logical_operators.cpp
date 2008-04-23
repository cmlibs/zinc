/*******************************************************************************
FILE : computed_field_logical_operators.c

LAST MODIFIED : 25 August 2006

DESCRIPTION :
Implements a number of logical operations on computed fields.
Three methods are developed here: AND, OR, XOR, EQUAL_TO, LESS_THAN, GREATER_THAN
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
#include "computed_field/computed_field_logical_operators.h"
}

class Computed_field_logical_operators_package : public Computed_field_type_package
{
public:
	struct MANAGER(Computed_field) *computed_field_manager;
};

namespace {

char computed_field_or_type_string[] = "or";

class Computed_field_or : public Computed_field_core
{
public:
	Computed_field_or(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_or(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_or_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_or*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);
};

int Computed_field_or::evaluate_cache_at_location(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	
	int i, return_code;

	ENTER(Computed_field_or::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
                 field->source_fields[0]->number_of_components) &&
                 (field->source_fields[0]->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = (0.0 != field->source_fields[0]->values[i]) || 
					(0.0 != field->source_fields[1]->values[i]);
			}	
			field->derivatives_valid = 0;
			if (0 < location->get_number_of_derivatives())
			{
				display_message(ERROR_MESSAGE,
				"Computed_field_or::evaluate_cache_at_location.  "
				"Cannot calculate derivatives of or");
			        return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_or::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_or::evaluate_cache_at_location */

} //namespace

int Computed_field_set_type_or(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_XYZ with the supplied
field, <source_field> .  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_or);
	if (field&&
		/* Access and broadcast before checking components match,
			the source_field_one and source_field_two will get replaced
			if necessary. */
		ACCESS(Computed_field)(source_field_one) &&
		ACCESS(Computed_field)(source_field_two) &&
		Computed_field_broadcast_field_components(
			&source_field_one, &source_field_two) &&
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
			field->core = new Computed_field_or(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
		DEACCESS(Computed_field)(&source_field_one);
		DEACCESS(Computed_field)(&source_field_two);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_or.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_or */

int Computed_field_get_type_or(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_XYZ, the 
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_or);
	if (field&&(dynamic_cast<Computed_field_or*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_or.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_or */

int define_Computed_field_type_or(struct Parse_state *state,
	void *field_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_POWER (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	Computed_field_logical_operators_package 
		*computed_field_logical_operators_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_or);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_logical_operators_package=
		(Computed_field_logical_operators_package *)
		computed_field_logical_operators_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_or_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_or(field, 
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
					computed_field_logical_operators_package->computed_field_manager;
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
					return_code = Computed_field_set_type_or(field,
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
							"define_Computed_field_type_or.  Failed");
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
				"define_Computed_field_type_or.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_or.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_or */

namespace {

char computed_field_and_type_string[] = "and";

class Computed_field_and : public Computed_field_core
{
public:
	Computed_field_and(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_and(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_and_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_and*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

};

int Computed_field_and::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{

	int i, return_code;

	ENTER(Computed_field_and::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
                 field->source_fields[0]->number_of_components) &&
                 (field->source_fields[0]->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = (0.0 != field->source_fields[0]->values[i])
					&& (0.0 != field->source_fields[1]->values[i]);
			}	
			field->derivatives_valid = 0;
			if (0 < location->get_number_of_derivatives())
			{
				display_message(ERROR_MESSAGE,
				"Computed_field_and::evaluate_cache_at_location.  "
				"Cannot calculate derivatives of and");
			        return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_and::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_and::evaluate_cache_at_location */

} //namespace

int Computed_field_set_type_and(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_AND with the supplied
field, <source_field> .  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_and);
	if (field&&
		/* Access and broadcast before checking components match,
			the source_field_one and source_field_two will get replaced
			if necessary. */
		ACCESS(Computed_field)(source_field_one) &&
		ACCESS(Computed_field)(source_field_two) &&
		Computed_field_broadcast_field_components(
			&source_field_one, &source_field_two) &&
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
			field->core = new Computed_field_and(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
		DEACCESS(Computed_field)(&source_field_one);
		DEACCESS(Computed_field)(&source_field_two);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_and.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_and */

int Computed_field_get_type_and(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_AND, the 
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_and);
	if (field&&(dynamic_cast<Computed_field_and*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_and.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_and */

int define_Computed_field_type_and(struct Parse_state *state,
	void *field_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_AND (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	Computed_field_logical_operators_package 
		*computed_field_logical_operators_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_and);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_logical_operators_package=
		(Computed_field_logical_operators_package *)
		computed_field_logical_operators_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_and_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_and(field, 
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
					computed_field_logical_operators_package->computed_field_manager;
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errands,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_and(field,
						source_fields[0], source_fields[1]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* errand */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_and.  Failed");
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
				"define_Computed_field_type_and.  Not enough memandy");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_and.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_and */

namespace {

char computed_field_xor_type_string[] = "xor";

class Computed_field_xor : public Computed_field_core
{
public:
	Computed_field_xor(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_xor(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_xor_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_xor*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);
};

int Computed_field_xor::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_xor::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
                 field->source_fields[0]->number_of_components) &&
                 (field->source_fields[0]->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				if (0.0 != field->source_fields[0]->values[i])
				{
					field->values[i] = (0.0 == field->source_fields[1]->values[i]);
				}
				else
				{
					field->values[i] = (0.0 != field->source_fields[1]->values[i]);
				}
			}	
			if (0 < location->get_number_of_derivatives())
			{
				display_message(ERROR_MESSAGE,
				"Computed_field_xor::evaluate_cache_at_location.  "
				"Cannot calculate derivatives of xor");
			        return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_xor::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_xor::evaluate_cache_at_location */



} //namespace

int Computed_field_set_type_xor(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_XOR with the supplied
field, <source_field> .  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_xor);
	if (field&&
		/* Access and broadcast before checking components match,
			the source_field_one and source_field_two will get replaced
			if necessary. */
		ACCESS(Computed_field)(source_field_one) &&
		ACCESS(Computed_field)(source_field_two) &&
		Computed_field_broadcast_field_components(
			&source_field_one, &source_field_two) &&
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
			field->core = new Computed_field_xor(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
		DEACCESS(Computed_field)(&source_field_one);
		DEACCESS(Computed_field)(&source_field_two);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_xor.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_xor */

int Computed_field_get_type_xor(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_XOR, the 
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_xor);
	if (field&&(dynamic_cast<Computed_field_xor*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_xor.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_xor */

int define_Computed_field_type_xor(struct Parse_state *state,
	void *field_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_XOR (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	Computed_field_logical_operators_package 
		*computed_field_logical_operators_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_xor);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_logical_operators_package=
		(Computed_field_logical_operators_package *)
		computed_field_logical_operators_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_xor_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_xor(field, 
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
					computed_field_logical_operators_package->computed_field_manager;
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errxors,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_xor(field,
						source_fields[0], source_fields[1]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* errxor */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_xor.  Failed");
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
				"define_Computed_field_type_xor.  Not enough memxory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_xor.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_xor */

namespace {

char computed_field_equal_to_type_string[] = "equal_to";

class Computed_field_equal_to : public Computed_field_core
{
public:
	Computed_field_equal_to(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_equal_to(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_equal_to_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_equal_to*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);
};

int Computed_field_equal_to::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_equal_to::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
		field->source_fields[0]->number_of_components) &&
		(field->source_fields[0]->number_of_components == field->source_fields[1]->number_of_components))
	{
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			if (field->source_fields[0]->values_valid &&
				field->source_fields[1]->values_valid)
			{
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					field->values[i] = (field->source_fields[0]->values[i]
						== field->source_fields[1]->values[i]);
				}
			}
			/* Also should try and convert a numerical valid value to match
				a string valid value if possible */
			else if (field->source_fields[0]->string_cache
				&& field->source_fields[1]->string_cache)
			{
				if (!strcmp(field->source_fields[0]->string_cache,
						field->source_fields[1]->string_cache))
				{
					/* Cannot do a component wise comparison with the current
						string cache storage unfortunately */
					for (i = 0 ; i < field->number_of_components ; i++)
					{
						field->values[i] = 1;
					}
				}
				else
				{
					for (i = 0 ; i < field->number_of_components ; i++)
					{
						field->values[i] = 0;
					}
				}
			}
			else
			{
				return_code = 0;
			}
			field->derivatives_valid = 0;
			if (0 < location->get_number_of_derivatives())
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_equal_to::evaluate_cache_at_location.  "
					"Cannot calculate derivatives of equal_to");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_equal_to::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_equal_to::evaluate_cache_at_location */



} //namespace

int Computed_field_set_type_equal_to(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_EQUAL_TO with the supplied
field, <source_field> .  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_equal_to);
	if (field&&
		/* Access and broadcast before checking components match,
			the source_field_one and source_field_two will get replaced
			if necessary. */
		ACCESS(Computed_field)(source_field_one) &&
		ACCESS(Computed_field)(source_field_two) &&
		Computed_field_broadcast_field_components(
			&source_field_one, &source_field_two) &&
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
			field->core = new Computed_field_equal_to(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
		DEACCESS(Computed_field)(&source_field_one);
		DEACCESS(Computed_field)(&source_field_two);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_equal_to.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_equal_to */

int Computed_field_get_type_equal_to(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_EQUAL_TO, the 
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_equal_to);
	if (field&&(dynamic_cast<Computed_field_equal_to*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_equal_to.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_equal_to */

int define_Computed_field_type_equal_to(struct Parse_state *state,
	void *field_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_EQUAL_TO (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	Computed_field_logical_operators_package 
		*computed_field_logical_operators_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_equal_to);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_logical_operators_package=
		(Computed_field_logical_operators_package *)
		computed_field_logical_operators_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_equal_to_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_equal_to(field, 
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
					computed_field_logical_operators_package->computed_field_manager;
				set_source_field_data.conditional_function=
					(LIST_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errequal_tos,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_equal_to(field,
						source_fields[0], source_fields[1]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* errequal_to */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_equal_to.  Failed");
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
				"define_Computed_field_type_equal_to.  Not enough memequal_toy");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_equal_to.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_equal_to */

namespace {

char computed_field_less_than_type_string[] = "less_than";

class Computed_field_less_than : public Computed_field_core
{
public:
	Computed_field_less_than(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_less_than(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_less_than_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_less_than*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);
};

int Computed_field_less_than::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_less_than::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
                 field->source_fields[0]->number_of_components) &&
                 (field->source_fields[0]->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = (field->source_fields[0]->values[i]
					< field->source_fields[1]->values[i]);
			}	
			field->derivatives_valid = 0;
			if (0 < location->get_number_of_derivatives())
			{
				display_message(ERROR_MESSAGE,
				"Computed_field_less_than::evaluate_cache_at_location.  "
				"Cannot calculate derivatives of less_than");
			        return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_less_than::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_less_than::evaluate_cache_at_location */



} //namespace

int Computed_field_set_type_less_than(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_LESS_THAN with the supplied
field, <source_field> .  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_less_than);
	if (field&&
		/* Access and broadcast before checking components match,
			the source_field_one and source_field_two will get replaced
			if necessary. */
		ACCESS(Computed_field)(source_field_one) &&
		ACCESS(Computed_field)(source_field_two) &&
		Computed_field_broadcast_field_components(
			&source_field_one, &source_field_two) &&
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
			field->core = new Computed_field_less_than(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
		DEACCESS(Computed_field)(&source_field_one);
		DEACCESS(Computed_field)(&source_field_two);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_less_than.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_less_than */

int Computed_field_get_type_less_than(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_LESS_THAN, the 
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_less_than);
	if (field&&(dynamic_cast<Computed_field_less_than*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_less_than.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_less_than */

int define_Computed_field_type_less_than(struct Parse_state *state,
	void *field_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_LESS_THAN (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	Computed_field_logical_operators_package 
		*computed_field_logical_operators_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_less_than);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_logical_operators_package=
		(Computed_field_logical_operators_package *)
		computed_field_logical_operators_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_less_than_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_less_than(field, 
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
					computed_field_logical_operators_package->computed_field_manager;
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errless_thans,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_less_than(field,
						source_fields[0], source_fields[1]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* errless_than */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_less_than.  Failed");
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
				"define_Computed_field_type_less_than.  Not enough memless_thany");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_less_than.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_less_than */

namespace {

char computed_field_greater_than_type_string[] = "greater_than";

class Computed_field_greater_than : public Computed_field_core
{
public:
	Computed_field_greater_than(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_greater_than(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_greater_than_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_greater_than*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);
};

int Computed_field_greater_than::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_greater_than::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
                 field->source_fields[0]->number_of_components) &&
                 (field->source_fields[0]->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = (field->source_fields[0]->values[i]
					> field->source_fields[1]->values[i]);
			}	
			field->derivatives_valid = 0;
			if (0 < location->get_number_of_derivatives())
			{
				display_message(ERROR_MESSAGE,
				"Computed_field_greater_than::evaluate_cache_at_location.  "
				"Cannot calculate derivatives of greater_than");
			        return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_greater_than::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_greater_than::evaluate_cache_at_location */



} //namespace

int Computed_field_set_type_greater_than(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_GREATER_THAN with the supplied
field, <source_field> .  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_greater_than);
	if (field&&
		/* Access and broadcast before checking components match,
			the source_field_one and source_field_two will get replaced
			if necessary. */
		ACCESS(Computed_field)(source_field_one) &&
		ACCESS(Computed_field)(source_field_two) &&
		Computed_field_broadcast_field_components(
			&source_field_one, &source_field_two) &&
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
			field->core = new Computed_field_greater_than(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
		DEACCESS(Computed_field)(&source_field_one);
		DEACCESS(Computed_field)(&source_field_two);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_greater_than.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_greater_than */

int Computed_field_get_type_greater_than(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_GREATER_THAN, the 
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_greater_than);
	if (field&&(dynamic_cast<Computed_field_greater_than*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_greater_than.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_greater_than */

int define_Computed_field_type_greater_than(struct Parse_state *state,
	void *field_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_GREATER_THAN (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	Computed_field_logical_operators_package 
		*computed_field_logical_operators_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_greater_than);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_logical_operators_package=
		(Computed_field_logical_operators_package *)
		computed_field_logical_operators_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_greater_than_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_greater_than(field, 
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
					computed_field_logical_operators_package->computed_field_manager;
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errgreater_thans,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_greater_than(field,
						source_fields[0], source_fields[1]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* errgreater_than */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_greater_than.  Failed");
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
				"define_Computed_field_type_greater_than.  Not enough memgreater_thany");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_greater_than.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_greater_than */

namespace {

char computed_field_is_defined_type_string[] = "is_defined";

class Computed_field_is_defined : public Computed_field_core
{
public:
	Computed_field_is_defined(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_is_defined(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_is_defined_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_is_defined*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int is_defined_at_location(Field_location* location);
};

int Computed_field_is_defined::is_defined_at_location(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Always returns 1 as the value is whether or not the source field is defined.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_defined::is_defined_at_location);
	if (field && location)
	{
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_defined::is_defined_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_defined::is_defined_at_location */

int Computed_field_is_defined::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_defined::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields == 1))
	{
		/* Calculate the field */
		field->values[0] = Computed_field_is_defined_at_location(
			field->source_fields[0], location);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_defined::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_defined::evaluate_cache_at_location */



} //namespace

int Computed_field_set_type_is_defined(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_IS_DEFINED with the supplied
field, <source_field> .  Number of components is one.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_is_defined);
	if (field&&source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components = 1;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->core = new Computed_field_is_defined(field);
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
			"Computed_field_set_type_is_defined.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_is_defined */

int Computed_field_get_type_is_defined(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_IS_DEFINED, the 
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_is_defined);
	if (field&&(dynamic_cast<Computed_field_is_defined*>(field->core)))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_is_defined.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_is_defined */

int define_Computed_field_type_is_defined(struct Parse_state *state,
	void *field_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_IS_DEFINED (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,*source_field;
	Computed_field_logical_operators_package 
		*computed_field_logical_operators_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_is_defined);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_logical_operators_package=
		(Computed_field_logical_operators_package *)
		computed_field_logical_operators_package_void))
	{
		return_code=1;
		source_field = (struct Computed_field *)NULL;
		if (computed_field_is_defined_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_is_defined(field, 
				&source_field);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			option_table = CREATE(Option_table)();
			/* field */
			set_source_field_data.computed_field_manager=
				computed_field_logical_operators_package->computed_field_manager;
			set_source_field_data.conditional_function=Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				&set_source_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			/* no erris_defineds,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_is_defined(field,
					source_field);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* erris_defined */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_is_defined.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_is_defined.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_is_defined */

int Computed_field_register_types_logical_operators(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_logical_operators_package
		*computed_field_logical_operators_package =
		new Computed_field_logical_operators_package;

	ENTER(Computed_field_register_types_logical_operators);
	if (computed_field_package)
	{
		computed_field_logical_operators_package->computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_or_type_string, 
			define_Computed_field_type_or,
			computed_field_logical_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_and_type_string, 
			define_Computed_field_type_and,
			computed_field_logical_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_xor_type_string, 
			define_Computed_field_type_xor,
			computed_field_logical_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_equal_to_type_string, 
			define_Computed_field_type_equal_to,
			computed_field_logical_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_less_than_type_string, 
			define_Computed_field_type_less_than,
			computed_field_logical_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_greater_than_type_string, 
			define_Computed_field_type_greater_than,
			computed_field_logical_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_is_defined_type_string, 
			define_Computed_field_type_is_defined,
			computed_field_logical_operators_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_logical_operators.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_logical_operators */

