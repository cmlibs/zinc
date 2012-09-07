/*******************************************************************************
FILE : computed_field_logical_operators.c

LAST MODIFIED : 16 May 2008

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
};

namespace {

char computed_field_or_type_string[] = "or";

class Computed_field_or : public Computed_field_core
{
public:
	Computed_field_or() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_or();
	}

	const char *get_type_string()
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

};

int Computed_field_or::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] = (0.0 != source1Cache->values[i]) || (0.0 != source2Cache->values[i]);
		}
		valueCache.derivatives_valid = 0;
		return 1;
	}
	return 0;
}

} //namespace

Computed_field *Cmiss_field_module_create_or(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
{
	Computed_field *field = NULL;
	/* Access and broadcast before checking components match,
		the local source_field_one and source_field_two will 
		get replaced if necessary. */
	ACCESS(Computed_field)(source_field_one);
	ACCESS(Computed_field)(source_field_two);
	if (field_module && source_field_one && source_field_one->isNumerical() &&
		source_field_two && source_field_two->isNumerical() &&
		Computed_field_broadcast_field_components(field_module,
			&source_field_one, &source_field_two) &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		Computed_field *source_fields[2];
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field_one->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_or());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_or.  Invalid argument(s)");
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);

	return (field);
}

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
	void *field_modify_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

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

	ENTER(define_Computed_field_type_or);
	USE_PARAMETER(computed_field_logical_operators_package_void);
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
				(computed_field_or_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_or(field_modify->get_field(), 
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
						Cmiss_field_module_create_or(field_modify->get_field_module(),
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
	Computed_field_and() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_and();
	}

	const char *get_type_string()
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

};

int Computed_field_and::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] = (0.0 != source1Cache->values[i]) && (0.0 != source2Cache->values[i]);
		}
		valueCache.derivatives_valid = 0;
		return 1;
	}
	return 0;
}

} //namespace

Computed_field *Cmiss_field_module_create_and(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
{
	Computed_field *field = NULL;
	/* Access and broadcast before checking components match,
		the local source_field_one and source_field_two will 
		get replaced if necessary. */
	ACCESS(Computed_field)(source_field_one);
	ACCESS(Computed_field)(source_field_two);
	if (field_module && source_field_one && source_field_one->isNumerical() &&
		source_field_two && source_field_two->isNumerical() &&
		Computed_field_broadcast_field_components(field_module,
			&source_field_one, &source_field_two) &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		Computed_field *source_fields[2];
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field_one->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_and());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_or.  Invalid argument(s)");
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);

	return (field);
}

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
	void *field_modify_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_AND (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_and);
	USE_PARAMETER(computed_field_logical_operators_package_void);
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
				(computed_field_and_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_and(field_modify->get_field(), 
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
				/* no errands,not asking for help */
				if (return_code)
				{
					return_code = field_modify->update_field_and_deaccess(
						Cmiss_field_module_create_and(field_modify->get_field_module(),
							source_fields[0], source_fields[1]));
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
	Computed_field_xor() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_xor();
	}

	const char *get_type_string()
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);
};

int Computed_field_xor::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
			if (0.0 != source1Cache->values[i])
			{
				valueCache.values[i] = (0.0 == source2Cache->values[i]);
			}
			else
			{
				valueCache.values[i] = (0.0 != source2Cache->values[i]);
			}
		}
		valueCache.derivatives_valid = 0;
		return 1;
	}
	return 0;
}

} //namespace

Computed_field *Cmiss_field_module_create_xor(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
{
	Computed_field *field = NULL;
	/* Access and broadcast before checking components match,
		the local source_field_one and source_field_two will 
		get replaced if necessary. */
	ACCESS(Computed_field)(source_field_one);
	ACCESS(Computed_field)(source_field_two);
	if (field_module && source_field_one && source_field_one->isNumerical() &&
		source_field_two && source_field_two->isNumerical() &&
		Computed_field_broadcast_field_components(field_module,
			&source_field_one, &source_field_two) &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		Computed_field *source_fields[2];
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field_one->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_xor());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_or.  Invalid argument(s)");
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);

	return (field);
}

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
	void *field_modify_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_XOR (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_xor);
	USE_PARAMETER(computed_field_logical_operators_package_void);
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
				(computed_field_xor_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_xor(field_modify->get_field(), 
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
				/* no errxors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->update_field_and_deaccess(
						Cmiss_field_module_create_xor(field_modify->get_field_module(),
							source_fields[0], source_fields[1]));
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
	Computed_field_equal_to() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_equal_to();
	}

	const char *get_type_string()
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);
};

int Computed_field_equal_to::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	switch (Cmiss_field_get_value_type(getSourceField(0)))
	{
		case CMISS_FIELD_VALUE_TYPE_REAL:
		{
			RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
			RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
			if (source1Cache && source2Cache)
			{
				for (int i = 0 ; i < field->number_of_components ; i++)
				{
					valueCache.values[i] = (source1Cache->values[i] == source2Cache->values[i]) ? 1.0 : 0.0;
				}
				valueCache.derivatives_valid = 0;
				return 1;
			}
		} break;
		case CMISS_FIELD_VALUE_TYPE_STRING:
		{
			StringFieldValueCache *source1Cache = StringFieldValueCache::cast(getSourceField(0)->evaluate(cache));
			StringFieldValueCache *source2Cache = StringFieldValueCache::cast(getSourceField(1)->evaluate(cache));
			if (source1Cache && source2Cache)
			{
				FE_value result = (FE_value)(0 == strcmp(source1Cache->stringValue, source2Cache->stringValue));
				// should only be one component if string arguments
				for (int i = 0 ; i < field->number_of_components ; i++)
				{
					valueCache.values[i] = result;
				}
				return 1;
			}
		} break;
		default:
		{
			// unsupported type
		} break;
	}
	return 0;
}

} //namespace

Computed_field *Cmiss_field_module_create_equal_to(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
{
	Computed_field *field = NULL;
	/* Access and broadcast before checking components match,
		the local source_field_one and source_field_two will 
		get replaced if necessary. */
	ACCESS(Computed_field)(source_field_one);
	ACCESS(Computed_field)(source_field_two);
	if (field_module && source_field_one && source_field_two &&
		(Cmiss_field_get_value_type(source_field_one) == Cmiss_field_get_value_type(source_field_two)) &&
		Computed_field_broadcast_field_components(field_module,
			&source_field_one, &source_field_two) &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		Computed_field *source_fields[2];
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field_one->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_equal_to());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_or.  Invalid argument(s)");
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);

	return (field);
}

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
	void *field_modify_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_EQUAL_TO (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_equal_to);
	USE_PARAMETER(computed_field_logical_operators_package_void);
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
				(computed_field_equal_to_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_equal_to(field_modify->get_field(), 
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
					return_code = field_modify->update_field_and_deaccess(
						Cmiss_field_module_create_equal_to(field_modify->get_field_module(),
							source_fields[0], source_fields[1]));
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
	Computed_field_less_than() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_less_than();
	}

	const char *get_type_string()
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);
};

int Computed_field_less_than::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] = (source1Cache->values[i] < source2Cache->values[i]) ? 1.0 : 0.0;
		}
		valueCache.derivatives_valid = 0;
		return 1;
	}
	return 0;
}

} //namespace

Computed_field *Computed_field_create_less_than(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
{
	Computed_field *field = NULL;
	/* Access and broadcast before checking components match,
		the local source_field_one and source_field_two will 
		get replaced if necessary. */
	ACCESS(Computed_field)(source_field_one);
	ACCESS(Computed_field)(source_field_two);
	if (field_module && source_field_one  && source_field_one->isNumerical() &&
		source_field_two && source_field_two->isNumerical() &&
		Computed_field_broadcast_field_components(field_module,
			&source_field_one, &source_field_two) &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		Computed_field *source_fields[2];
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field_one->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_less_than());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_or.  Invalid argument(s)");
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);

	return (field);
}

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
	void *field_modify_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_LESS_THAN (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_less_than);
	USE_PARAMETER(computed_field_logical_operators_package_void);
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
				(computed_field_less_than_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_less_than(field_modify->get_field(), 
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
				/* no errless_thans,not asking for help */
				if (return_code)
				{
					return_code = field_modify->update_field_and_deaccess(
						Computed_field_create_less_than(field_modify->get_field_module(),
							source_fields[0], source_fields[1]));
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
	Computed_field_greater_than() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_greater_than();
	}

	const char *get_type_string()
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);
};

int Computed_field_greater_than::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] = (source1Cache->values[i] > source2Cache->values[i]) ? 1.0 : 0.0;
		}
		valueCache.derivatives_valid = 0;
		return 1;
	}
	return 0;
}

} //namespace

Computed_field *Computed_field_create_greater_than(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
{
	Computed_field *field = NULL;
	/* Access and broadcast before checking components match,
		the local source_field_one and source_field_two will 
		get replaced if necessary. */
	ACCESS(Computed_field)(source_field_one);
	ACCESS(Computed_field)(source_field_two);
	if (field_module && source_field_one  && source_field_one->isNumerical() &&
		source_field_two && source_field_two->isNumerical() &&
		Computed_field_broadcast_field_components(field_module,
			&source_field_one, &source_field_two) &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		Computed_field *source_fields[2];
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field_one->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_greater_than());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_or.  Invalid argument(s)");
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);

	return (field);
}

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
	void *field_modify_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_GREATER_THAN (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_greater_than);
	USE_PARAMETER(computed_field_logical_operators_package_void);
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
				(computed_field_greater_than_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_greater_than(field_modify->get_field(), 
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
				/* no errgreater_thans,not asking for help */
				if (return_code)
				{
					return_code = field_modify->update_field_and_deaccess(
						Computed_field_create_greater_than(field_modify->get_field_module(),
							source_fields[0], source_fields[1]));
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
	Computed_field_is_defined() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_is_defined();
	}

	const char *get_type_string()
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	virtual bool is_defined_at_location(Cmiss_field_cache&)
	{
		return true;
	}
};

int Computed_field_is_defined::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
	valueCache.values[0] = getSourceField(0)->core->is_defined_at_location(cache);
	return 1;
}

} //namespace

/*****************************************************************************//**
 * Creates a scalar field whose value is 1 wherever the source field is defined,
 * and 0 elsewhere (without error).
 * 
 * @param field_module  Region field module which will own new field.
 * @param source_field  Source field to check whether defined.
 * @return Newly created field
 */
Computed_field *Computed_field_create_is_defined(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field)
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		/*number_of_components*/1,
		/*number_of_source_fields*/1, &source_field,
		/*number_of_source_values*/0, NULL,
		new Computed_field_is_defined());

	return (field);
}

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
	void *field_modify_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_IS_DEFINED (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_is_defined);
	USE_PARAMETER(computed_field_logical_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		source_field = (struct Computed_field *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_is_defined_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code=Computed_field_get_type_is_defined(field_modify->get_field(), 
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
				field_modify->get_field_manager();
			set_source_field_data.conditional_function=Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				&set_source_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			/* no erris_defineds,not asking for help */
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_is_defined(field_modify->get_field_module(),
						source_field));
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

namespace {

char computed_field_not_type_string[] = "not";

class Computed_field_not : public Computed_field_core
{
public:
	Computed_field_not() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_not();
	}

	const char *get_type_string()
	{
		return (computed_field_not_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		return (0 != dynamic_cast<Computed_field_not*>(other_field));
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);
};

int Computed_field_not::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] = (0.0 != sourceCache->values[i]) ? 0.0 : 1.0;
		}
		return 1;
	}
	return 0;
}

} //namespace

Cmiss_field_id Cmiss_field_module_create_not(Cmiss_field_module_id field_module,
	Cmiss_field_id source_field)
{
	Cmiss_field_id field = 0;
	if (source_field && source_field->isNumerical())
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_not());
	}
	return field;
}

/***************************************************************************//**
 * Command modifier function for creating and modifying field type
 * Computed_field_not.
 */
int define_Computed_field_type_not(struct Parse_state *state,
	void *field_modify_void, void *computed_field_logical_operators_package_void)
{
	int return_code = 1;
	USE_PARAMETER(computed_field_logical_operators_package_void);
	Computed_field_modify_data *field_modify = reinterpret_cast<Computed_field_modify_data *>(field_modify_void);
	if (state && field_modify)
	{
		Cmiss_field_id source_field = 0;
		if ((NULL != field_modify->get_field()) &&
			(NULL != (dynamic_cast<Computed_field_not*>(field_modify->get_field()->core))))
		{
			source_field = Cmiss_field_get_source_field(field_modify->get_field(), 1);
		}

		Option_table *option_table = CREATE(Option_table)();
		/* field */
		Set_Computed_field_conditional_data set_source_field_data;
		set_source_field_data.computed_field_manager = field_modify->get_field_manager();
		set_source_field_data.conditional_function = Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		Option_table_add_Computed_field_conditional_entry(option_table, "field", &source_field,
			&set_source_field_data);
		return_code = Option_table_multi_parse(option_table,state);
		DESTROY(Option_table)(&option_table);

		if (return_code)
		{
			return_code = field_modify->update_field_and_deaccess(
				Cmiss_field_module_create_not(field_modify->get_field_module(), source_field));
		}
		Cmiss_field_destroy(&source_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_not.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

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
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_not_type_string,
			define_Computed_field_type_not,
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

