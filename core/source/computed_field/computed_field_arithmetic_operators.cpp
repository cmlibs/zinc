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
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_cache.hpp"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
#include "computed_field/computed_field_arithmetic_operators.h"

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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

int Computed_field_power::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		int i, j;
		/* 2. Calculate the field */
		for (i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] =
				(FE_value)pow((double)(source1Cache->values[i]),
				(double)(source2Cache->values[i]));
		}
		int number_of_xi = cache.getRequestedDerivatives();
		if (number_of_xi && source1Cache->derivatives_valid && source2Cache->derivatives_valid)
		{
			FE_value *derivative = valueCache.derivatives;
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				for (j = 0 ; j < number_of_xi ; j++)
				{
					/* d(u^v)/dx =
					 *   v * u^(v-1) * du/dx   +   u^v * ln(u) * dv/dx
					 */
					*derivative =
						source2Cache->values[i] *
						(FE_value)pow((double)(source1Cache->values[i]),
							(double)(source2Cache->values[i]-1)) *
							source1Cache->derivatives[i * number_of_xi + j] +
						(FE_value)pow((double)(source1Cache->values[i]),
							(double)(source2Cache->values[i])) *
						(FE_value)log((double)(source1Cache->values[i])) *
						source2Cache->derivatives[i * number_of_xi + j];
					derivative++;
				}
			}
			valueCache.derivatives_valid = 1;
		}
		else
		{
			valueCache.derivatives_valid = 0;
		}
		return 1;
	}
	return 0;
}


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
	if (field_module && source_field_one && source_field_one->isNumerical() &&
		source_field_two && source_field_two->isNumerical() &&
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

int Computed_field_multiply_components::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		int i, j;
		for (i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] = source1Cache->values[i] *
				source2Cache->values[i];
		}
		int number_of_xi = cache.getRequestedDerivatives();
		if (number_of_xi && source1Cache->derivatives_valid && source2Cache->derivatives_valid)
		{
			FE_value *derivative = valueCache.derivatives;
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				for (j = 0 ; j < number_of_xi ; j++)
				{
					*derivative =
						source1Cache->derivatives[i * number_of_xi + j] *
						source2Cache->values[i] +
						source2Cache->derivatives[i * number_of_xi + j] *
						source1Cache->values[i];
					derivative++;
				}
			}
			valueCache.derivatives_valid = 1;
		}
		else
		{
			valueCache.derivatives_valid = 0;
		}
		return 1;
	}
	return 0;
}

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
	if (field_module && source_field_one && source_field_one->isNumerical() &&
		source_field_two && source_field_two->isNumerical() &&
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

int Computed_field_divide_components::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		int i, j;
		for (i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] = source1Cache->values[i] /
				source2Cache->values[i];
		}
		int number_of_xi = cache.getRequestedDerivatives();
		if (number_of_xi && source1Cache->derivatives_valid && source2Cache->derivatives_valid)
		{
			FE_value *derivative = valueCache.derivatives;
			FE_value vsquared;
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				vsquared = source2Cache->values[i]
					* source2Cache->values[i];
				for (j = 0 ; j < number_of_xi ; j++)
				{
					*derivative =
						(source1Cache->derivatives[i * number_of_xi + j] *
						source2Cache->values[i] -
						source2Cache->derivatives[i * number_of_xi + j] *
						source1Cache->values[i]) / vsquared;
					derivative++;
				}
			}
			valueCache.derivatives_valid = 1;
		}
		else
		{
			valueCache.derivatives_valid = 0;
		}
		return 1;
	}
	return 0;
}

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
	if (field_module && source_field_one && source_field_one->isNumerical() &&
		source_field_two && source_field_two->isNumerical() &&
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

	virtual int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

int Computed_field_add::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		for (int i = 0; i < field->number_of_components; i++)
		{
			valueCache.values[i] =
				field->source_values[0] * source1Cache->values[i] +
				field->source_values[1] * source2Cache->values[i];
		}
		int number_of_xi = cache.getRequestedDerivatives();
		if (number_of_xi && source1Cache->derivatives_valid && source2Cache->derivatives_valid)
		{
			FE_value *temp = valueCache.derivatives;
			FE_value *temp1 = source1Cache->derivatives;
			FE_value *temp2 = source2Cache->derivatives;
			for (int i = (field->number_of_components*number_of_xi); 0 < i; i--)
			{
				(*temp) = field->source_values[0]*(*temp1) + field->source_values[1]*(*temp2);
				temp++;
				temp1++;
				temp2++;
			}
			valueCache.derivatives_valid = 1;
		}
		else
		{
			valueCache.derivatives_valid = 0;
		}
		return 1;
	}
	return 0;
}

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
	if (field_module && source_field_one && source_field_one->isNumerical() &&
		source_field_two && source_field_two->isNumerical() &&
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	virtual enum FieldAssignmentResult assign(Cmiss_field_cache& cache, RealFieldValueCache& valueCache);

	virtual int propagate_find_element_xi(Cmiss_field_cache& field_cache,
		const FE_value *values, int number_of_values,
		struct FE_element **element_address, FE_value *xi,
		Cmiss_mesh_id mesh);
};

int Computed_field_scale::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		int i, j;
		for (i=0;i<field->number_of_components;i++)
		{
			valueCache.values[i]=
				field->source_values[i]*sourceCache->values[i];
		}
		int number_of_xi = cache.getRequestedDerivatives();
		if (number_of_xi && sourceCache->derivatives_valid)
		{
			FE_value *temp = valueCache.derivatives;
			FE_value *temp2 = sourceCache->derivatives;
			for (i=0;i<field->number_of_components;i++)
			{
				for (j=0;j<number_of_xi;j++)
				{
					(*temp)=field->source_values[i]*(*temp2);
					temp++;
					temp2++;
				}
			}
			valueCache.derivatives_valid = 1;
		}
		else
		{
			valueCache.derivatives_valid = 0;
		}
		return 1;
	}
	return 0;
}

enum FieldAssignmentResult Computed_field_scale::assign(Cmiss_field_cache& cache, RealFieldValueCache& valueCache)
{
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->getValueCache(cache));
	for (int i = 0; i < field->number_of_components; i++)
	{
		FE_value scale_value = valueCache.values[i];
		if (0.0 == scale_value)
			return FIELD_ASSIGNMENT_RESULT_FAIL;
		sourceCache->values[i] = valueCache.values[i] / scale_value;
	}
	sourceCache->derivatives_valid = 0;
	return getSourceField(0)->assign(cache, *sourceCache);
}

int Computed_field_scale::propagate_find_element_xi(Cmiss_field_cache& field_cache,
	const FE_value *values, int number_of_values, struct FE_element **element_address,
	FE_value *xi, Cmiss_mesh_id mesh)
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
						"Computed_field_scale::propagate_find_element_xi.  "
						"Cannot invert scale field %s with zero scale factor",
						field->name);
					return_code = 0;
				}
			}
			if (return_code)
			{
				return_code = Computed_field_find_element_xi(
					field->source_fields[0], &field_cache, source_values, number_of_values,
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
	Cmiss_field_id field = 0;
	if (source_field && source_field->isNumerical())
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/source_field->number_of_components, scale_factors,
			new Computed_field_scale());
	}
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	enum FieldAssignmentResult assign(Cmiss_field_cache& cache, RealFieldValueCache& valueCache);

};

int Computed_field_clamp_maximum::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		FE_value *temp = 0, *temp2 = 0;
		int i, j;
		int number_of_xi = cache.getRequestedDerivatives();
		if (number_of_xi && sourceCache->derivatives_valid)
		{
			temp=valueCache.derivatives;
			temp2=sourceCache->derivatives;
			valueCache.derivatives_valid = 1;
		}
		else
		{
			valueCache.derivatives_valid = 0;
		}
		for (i=0;i<field->number_of_components;i++)
		{
			if (sourceCache->values[i] < field->source_values[i])
			{
				valueCache.values[i]=sourceCache->values[i];
				if (valueCache.derivatives_valid)
				{
					for (j=0;j<number_of_xi;j++)
					{
						(*temp)=(*temp2);
						temp++;
						temp2++;
					}
				}
			}
			else
			{
				valueCache.values[i]=field->source_values[i];
				if (valueCache.derivatives_valid)
				{
					for (j=0;j<number_of_xi;j++)
					{
						(*temp)=0.0;
						temp++;
						temp2++; /* To ensure that the following components match */
					}
				}
			}
		}
		return 1;
	}
	return 0;
}

/** clamps to limits of maximums when setting values too */
enum FieldAssignmentResult Computed_field_clamp_maximum::assign(Cmiss_field_cache& cache, RealFieldValueCache& valueCache)
{
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->getValueCache(cache));
	enum FieldAssignmentResult result1 = FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
	for (int i = 0; i < field->number_of_components; i++)
	{
		FE_value max = field->source_values[i];
		if (valueCache.values[i] > max)
		{
			sourceCache->values[i] = max;
			result1 = FIELD_ASSIGNMENT_RESULT_PARTIAL_VALUES_SET;
		}
		else
		{
			sourceCache->values[i] = valueCache.values[i];
		}
	}
	sourceCache->derivatives_valid = 0;
	enum FieldAssignmentResult result2 = getSourceField(0)->assign(cache, *sourceCache);
	if (result2 == FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET)
		return result1;
	return result2;
}

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
	Cmiss_field_id field = 0;
	if (source_field && source_field->isNumerical())
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/source_field->number_of_components, maximums,
			new Computed_field_clamp_maximum());
	}
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	enum FieldAssignmentResult assign(Cmiss_field_cache& cache, RealFieldValueCache& valueCache);
};

int Computed_field_clamp_minimum::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		FE_value *temp = 0, *temp2 = 0;
		int i, j;
		int number_of_xi = cache.getRequestedDerivatives();
		if (number_of_xi && sourceCache->derivatives_valid)
		{
			temp=valueCache.derivatives;
			temp2=sourceCache->derivatives;
			valueCache.derivatives_valid = 1;
		}
		else
		{
			valueCache.derivatives_valid = 0;
		}
		for (i=0;i<field->number_of_components;i++)
		{
			if (sourceCache->values[i] > field->source_values[i])
			{
				valueCache.values[i]=sourceCache->values[i];
				if (valueCache.derivatives_valid)
				{
					for (j=0;j<number_of_xi;j++)
					{
						(*temp)=(*temp2);
						temp++;
						temp2++;
					}
				}
			}
			else
			{
				valueCache.values[i]=field->source_values[i];
				if (valueCache.derivatives_valid)
				{
					for (j=0;j<number_of_xi;j++)
					{
						(*temp)=0.0;
						temp++;
						temp2++; /* To ensure that the following components match */
					}
				}
			}
		}
		return 1;
	}
	return 0;
}

/** clamps to limits of minimums when setting values too */
enum FieldAssignmentResult Computed_field_clamp_minimum::assign(Cmiss_field_cache& cache, RealFieldValueCache& valueCache)
{
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->getValueCache(cache));
	enum FieldAssignmentResult result1 = FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
	for (int i = 0; i < field->number_of_components; i++)
	{
		FE_value min = field->source_values[i];
		if (valueCache.values[i] < min)
		{
			sourceCache->values[i] = min;
			result1 = FIELD_ASSIGNMENT_RESULT_PARTIAL_VALUES_SET;
		}
		else
		{
			sourceCache->values[i] = valueCache.values[i];
		}
	}
	sourceCache->derivatives_valid = 0;
	enum FieldAssignmentResult result2 = getSourceField(0)->assign(cache, *sourceCache);
	if (result2 == FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET)
		return result1;
	return result2;
}

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
	Cmiss_field_id field = 0;
	if (source_field && source_field->isNumerical())
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/source_field->number_of_components, minimums,
			new Computed_field_clamp_minimum());
	}
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	enum FieldAssignmentResult assign(Cmiss_field_cache& cache, RealFieldValueCache& valueCache);

	virtual int propagate_find_element_xi(Cmiss_field_cache& field_cache,
		const FE_value *values, int number_of_values,
		struct FE_element **element_address, FE_value *xi,
		Cmiss_mesh_id mesh);
};

int Computed_field_offset::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		int i, j;
		for (i=0;i<field->number_of_components;i++)
		{
			valueCache.values[i]=
				field->source_values[i]+sourceCache->values[i];
		}
		int number_of_xi = cache.getRequestedDerivatives();
		if (number_of_xi && sourceCache->derivatives_valid)
		{
			FE_value *temp=valueCache.derivatives;
			FE_value *temp2=sourceCache->derivatives;
			for (i=0;i<field->number_of_components;i++)
			{
				for (j=0;j<number_of_xi;j++)
				{
					(*temp)=(*temp2);
					temp++;
					temp2++;
				}
			}
			valueCache.derivatives_valid = 1;
		}
		else
		{
			valueCache.derivatives_valid = 0;
		}
		return 1;
	}
	return 0;
}

enum FieldAssignmentResult Computed_field_offset::assign(Cmiss_field_cache& cache, RealFieldValueCache& valueCache)
{
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->getValueCache(cache));
	for (int i = 0; i < field->number_of_components; i++)
	{
		sourceCache->values[i] = valueCache.values[i] - field->source_values[i];
	}
	sourceCache->derivatives_valid = 0;
	return getSourceField(0)->assign(cache, *sourceCache);
}

int Computed_field_offset::propagate_find_element_xi(Cmiss_field_cache& field_cache,
	const FE_value *values, int number_of_values, struct FE_element **element_address,
	FE_value *xi, Cmiss_mesh_id mesh)
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
				field->source_fields[0], &field_cache, source_values, number_of_values,
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
	Cmiss_field_id field = 0;
	if (source_field && source_field->isNumerical())
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/source_field->number_of_components, offsets,
			new Computed_field_offset());
	}
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

int Computed_field_sum_components::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		int i, j;
		/* weighted sum of components of source field */
		FE_value *temp = sourceCache->values;
		FE_value sum = 0.0;
		for (i = 0; i < field->number_of_source_values; i++)
		{
			sum += (*temp)*field->source_values[i];
			temp++;
		}
		valueCache.values[0] = sum;
		int number_of_xi = cache.getRequestedDerivatives();
		if (number_of_xi && sourceCache->derivatives_valid)
		{
			for (j = 0; j < number_of_xi; j++)
			{
				temp = sourceCache->derivatives + j;
				sum = 0.0;
				for (i = 0; i < field->number_of_source_values; i++)
				{
					sum += (*temp)*field->source_values[i];
					temp += number_of_xi;
				}
				valueCache.derivatives[j] = sum;
			}
			valueCache.derivatives_valid = 1;
		}
		else
		{
			valueCache.derivatives_valid = 0;
		}
		return 1;
	}
	return 0;
}

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
	Cmiss_field_id field = 0;
	if (source_field && source_field->isNumerical())
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			/*number_of_components*/1,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/source_field->number_of_components, weights,
			new Computed_field_sum_components());
	}
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	enum FieldAssignmentResult assign(Cmiss_field_cache& cache, RealFieldValueCache& valueCache);
};

int Computed_field_edit_mask::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		/* exact copy of source field */
		valueCache.copyValues(*sourceCache);
		return 1;
	}
	return 0;
}

/* assigns only components for which edit mask value is non-zero */
enum FieldAssignmentResult Computed_field_edit_mask::assign(Cmiss_field_cache& cache, RealFieldValueCache& valueCache)
{
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		for (int i = 0; i < field->number_of_components; i++)
		{
			if (field->source_values[i])
			{
				sourceCache->values[i] = valueCache.values[i];
			}
		}
		sourceCache->derivatives_valid = 0;
		enum FieldAssignmentResult result = getSourceField(0)->assign(cache, *sourceCache);
		if (result != FIELD_ASSIGNMENT_RESULT_FAIL)
			return FIELD_ASSIGNMENT_RESULT_PARTIAL_VALUES_SET;
	}
	return FIELD_ASSIGNMENT_RESULT_FAIL;
}

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
	Cmiss_field_id field = 0;
	if (source_field && source_field->isNumerical())
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/source_field->number_of_components, edit_mask,
			new Computed_field_edit_mask());
	}
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

int Computed_field_log::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		int i, j;
		for (i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] =
				(FE_value)log((double)(sourceCache->values[i]));
		}
		int number_of_xi = cache.getRequestedDerivatives();
		if (number_of_xi && sourceCache->derivatives_valid)
		{
			FE_value *derivative = valueCache.derivatives;
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				for (j = 0 ; j < number_of_xi ; j++)
				{
					/* d(log u)/dx = 1 / u * du/dx */
					*derivative = 1.0 / sourceCache->values[i] *
						sourceCache->derivatives[i * number_of_xi + j];
					derivative++;
				}
			}
			valueCache.derivatives_valid = 1;
		}
		else
		{
			valueCache.derivatives_valid = 0;
		}
		return 1;
	}
	return 0;
}

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
	Cmiss_field_id field = 0;
	if (source_field && source_field->isNumerical())
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_log());
	}
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

int Computed_field_sqrt::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		int i, j;
		for (i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] =
				(FE_value)sqrt((double)(sourceCache->values[i]));
		}
		int number_of_xi = cache.getRequestedDerivatives();
		if (number_of_xi && sourceCache->derivatives_valid)
		{
			FE_value *derivative = valueCache.derivatives;
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				for (j = 0 ; j < number_of_xi ; j++)
				{
					/* d(sqrt u)/dx = du/dx / 2 sqrt(u) */
					*derivative =
						sourceCache->derivatives[i * number_of_xi + j]
						/ ( 2 * valueCache.values[i] );
					derivative++;
				}
			}
			valueCache.derivatives_valid = 1;
		}
		else
		{
			valueCache.derivatives_valid = 0;
		}
		return 1;
	}
	return 0;
}

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
	Cmiss_field_id field = 0;
	if (source_field && source_field->isNumerical())
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_sqrt());
	}
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

int Computed_field_exp::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		int i, j;
		for (i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] =
				(FE_value)exp((double)(sourceCache->values[i]));
		}
		int number_of_xi = cache.getRequestedDerivatives();
		if (number_of_xi && sourceCache->derivatives_valid)
		{
			FE_value *derivative = valueCache.derivatives;
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				for (j = 0 ; j < number_of_xi ; j++)
				{
					/* d(exp u)/dx = du/dx exp(u) */
					*derivative =
						sourceCache->derivatives[i * number_of_xi + j]
						* valueCache.values[i];
					derivative++;
				}
			}
			valueCache.derivatives_valid = 1;
		}
		else
		{
			valueCache.derivatives_valid = 0;
		}
		return 1;
	}
	return 0;
}

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
	Cmiss_field_id field = 0;
	if (source_field && source_field->isNumerical())
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_exp());
	}
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

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

int Computed_field_abs::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		int i, j;
		for (i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] =
				(FE_value)fabs((double)(sourceCache->values[i]));
		}
		int number_of_xi = cache.getRequestedDerivatives();
		if (number_of_xi && sourceCache->derivatives_valid)
		{
			FE_value *derivative = valueCache.derivatives;
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				/* d(abs u)/dx = du/dx u>0
				 *               -du/dx u<0
				 *               and lets just put 0 at u=0 */
				if (sourceCache->values[i] > 0)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						*derivative = sourceCache->derivatives[i * number_of_xi + j];
						derivative++;
					}
				}
				else if (sourceCache->values[i] < 0)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						*derivative = -sourceCache->derivatives[i * number_of_xi + j];
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
			valueCache.derivatives_valid = 1;
		}
		else
		{
			valueCache.derivatives_valid = 0;
		}
		return 1;
	}
	return 0;
}

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
	Cmiss_field_id field = 0;
	if (source_field && source_field->isNumerical())
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_abs());
	}
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

