/*******************************************************************************
FILE : computed_field_arithmetic_operators.cpp

LAST MODIFIED : 15 May 2008

DESCRIPTION :
Implements a number of basic component wise operators on computed fields.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
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

const char computed_field_power_type_string[] = "power";

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

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_POWER;
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

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	int list();

	char* get_command_string();
};

int Computed_field_power::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	const RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	const RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
		for (int i = 0 ; i < field->number_of_components ; ++i)
			valueCache.values[i] = pow(source1Cache->values[i], source2Cache->values[i]);
		return 1;
	}
	return 0;
}

int Computed_field_power::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	if (fieldDerivative.getOrder() > 1)
		return 0;  // fall back to numerical derivatives
	const RealFieldValueCache *source1Cache = getSourceField(0)->evaluateDerivativeTree(cache, fieldDerivative);
	const RealFieldValueCache *source2Cache = getSourceField(1)->evaluateDerivativeTree(cache, fieldDerivative);
	if (source1Cache && source2Cache)
	{
		const FE_value *source1values = source1Cache->values;
		const FE_value *source2values = source2Cache->values;
		const FE_value *source1Derivatives = source1Cache->getDerivativeValueCache(fieldDerivative)->values;
		const FE_value *source2Derivatives = source2Cache->getDerivativeValueCache(fieldDerivative)->values;
		FE_value *derivative = inValueCache.getDerivativeValueCache(fieldDerivative)->values;
		const int number_of_xi = fieldDerivative.getTermCount();
		for (int i = 0; i < field->number_of_components; ++i)
		{
			for (int j = 0; j < number_of_xi; ++j)
			{
				/* d(u^v)/dx =
					*   v * u^(v-1) * du/dx   +   u^v * ln(u) * dv/dx
					*/
				*derivative =
					source2values[i] * pow(source1values[i], source2Cache->values[i] - 1.0) * source1Derivatives[i * number_of_xi + j] +
					pow(source1values[i], source2Cache->values[i]) * log(source1values[i]) * source2Derivatives[i * number_of_xi + j];
				derivative++;
			}
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

cmzn_field *cmzn_fieldmodule_create_field_power(cmzn_fieldmodule *field_module,
	cmzn_field *source_field_one,
	cmzn_field *source_field_two)
{
	cmzn_field *field, *source_fields[2];

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
			"cmzn_fieldmodule_create_field_power.  Invalid argument(s)");
		field = nullptr;
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);

	return (field);
}

int Computed_field_get_type_power(cmzn_field *field,
	cmzn_field **source_field_one,
	cmzn_field **source_field_two)
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

const char computed_field_multiply_components_type_string[] = "multiply_components";

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

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_MULTIPLY;
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

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	int list();

	char* get_command_string();
};

int Computed_field_multiply_components::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	const RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	const RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
		for (int i = 0 ; i < field->number_of_components ; i++)
			valueCache.values[i] = source1Cache->values[i] * source2Cache->values[i];
		return 1;
	}
	return 0;
}

int Computed_field_multiply_components::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	if (fieldDerivative.getOrder() > 1)
		return 0;  // fall back to numerical derivatives
	const RealFieldValueCache *source1Cache = getSourceField(0)->evaluateDerivativeTree(cache, fieldDerivative);
	const RealFieldValueCache *source2Cache = getSourceField(1)->evaluateDerivativeTree(cache, fieldDerivative);
	if (source1Cache && source2Cache)
	{
		const FE_value *source1values = source1Cache->values;
		const FE_value *source2values = source2Cache->values;
		const FE_value *source1Derivatives = source1Cache->getDerivativeValueCache(fieldDerivative)->values;
		const FE_value *source2Derivatives = source2Cache->getDerivativeValueCache(fieldDerivative)->values;
		FE_value *derivative = inValueCache.getDerivativeValueCache(fieldDerivative)->values;
		const int number_of_xi = fieldDerivative.getTermCount();
		for (int i = 0; i < field->number_of_components; ++i)
		{
			for (int j = 0; j < number_of_xi; ++j)
			{
				*derivative =
					source1Derivatives[i * number_of_xi + j] * source2values[i] +
					source2Derivatives[i * number_of_xi + j] * source1values[i];
				derivative++;
			}
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

cmzn_field *cmzn_fieldmodule_create_field_multiply(
	cmzn_fieldmodule *field_module,
	cmzn_field *source_field_one, cmzn_field *source_field_two)
{
	cmzn_field *field, *source_fields[2];

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
			"cmzn_fieldmodule_create_field_multiply.  Invalid argument(s)");
		field = nullptr;
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);

	return (field);
}

int Computed_field_get_type_multiply_components(cmzn_field *field,
	cmzn_field **source_field_one,
	cmzn_field **source_field_two)
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

const char computed_field_divide_components_type_string[] = "divide_components";

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

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_DIVIDE;
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

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	int list();

	char* get_command_string();
};

int Computed_field_divide_components::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	const RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	const RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
		for (int i = 0 ; i < field->number_of_components ; i++)
			valueCache.values[i] = source1Cache->values[i] / source2Cache->values[i];
		return 1;
	}
	return 0;
}

int Computed_field_divide_components::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	if (fieldDerivative.getOrder() > 1)
		return 0;  // fall back to numerical derivatives
	const RealFieldValueCache *source1Cache = getSourceField(0)->evaluateDerivativeTree(cache, fieldDerivative);
	const RealFieldValueCache *source2Cache = getSourceField(1)->evaluateDerivativeTree(cache, fieldDerivative);
	if (source1Cache && source2Cache)
	{
		const FE_value *source1values = source1Cache->values;
		const FE_value *source2values = source2Cache->values;
		const FE_value *source1Derivatives = source1Cache->getDerivativeValueCache(fieldDerivative)->values;
		const FE_value *source2Derivatives = source2Cache->getDerivativeValueCache(fieldDerivative)->values;
		FE_value *derivative = inValueCache.getDerivativeValueCache(fieldDerivative)->values;
		const int number_of_xi = fieldDerivative.getTermCount();
		for (int i = 0; i < field->number_of_components; ++i)
		{
			const FE_value vsquared = source2values[i] * source2values[i];
			for (int j = 0; j < number_of_xi; ++j)
			{
				*derivative = (
					source1Derivatives[i * number_of_xi + j] * source2values[i] -
					source2Derivatives[i * number_of_xi + j] * source1values[i]) / vsquared;
				derivative++;
			}
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

cmzn_field *cmzn_fieldmodule_create_field_divide(cmzn_fieldmodule *field_module,
	cmzn_field *source_field_one,
	cmzn_field *source_field_two)
{
	cmzn_field *field, *source_fields[2];

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
			"cmzn_fieldmodule_create_field_divide.  Invalid argument(s)");
		field = nullptr;
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);

	return (field);
}

int Computed_field_get_type_divide_components(cmzn_field *field,
	cmzn_field **source_field_one,
	cmzn_field **source_field_two)
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

const char computed_field_add_type_string[] = "add";

class Computed_field_add : public Computed_field_core
{
public:

	enum cmzn_field_type type;

	Computed_field_add() : Computed_field_core()
	{
		type = CMZN_FIELD_TYPE_ADD;
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

	virtual enum cmzn_field_type get_type()
	{
		return type;
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

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	int list();

	char* get_command_string();
};

int Computed_field_add::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	const RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	const RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
		for (int i = 0; i < field->number_of_components; ++i)
			valueCache.values[i] =
				field->source_values[0] * source1Cache->values[i] +
				field->source_values[1] * source2Cache->values[i];
		return 1;
	}
	return 0;
}

int Computed_field_add::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	const DerivativeValueCache *source1DerivativeCache = getSourceField(0)->evaluateDerivative(cache, fieldDerivative);
	const DerivativeValueCache *source2DerivativeCache = getSourceField(1)->evaluateDerivative(cache, fieldDerivative);
	if (source1DerivativeCache && source2DerivativeCache)
	{
		const FE_value *source1Derivatives = source1DerivativeCache->values;
		const FE_value *source2Derivatives = source2DerivativeCache->values;
		FE_value *derivative = inValueCache.getDerivativeValueCache(fieldDerivative)->values;
		const int valuesCount = field->number_of_components*fieldDerivative.getTermCount();
		for (int i = 0; i < valuesCount; ++i)
			derivative[i] = field->source_values[0]*source1Derivatives[i] + field->source_values[1]*source2Derivatives[i];
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


/**
 * Create field of type COMPUTED_FIELD_ADD with the supplied
 * fields, <source_field_one> and <source_field_two>.  Sets the number of
 * components equal to the source_fields.
 * Automatic scalar broadcast will apply, see cmiss_field.h.
*/
cmzn_field *cmzn_fieldmodule_create_field_weighted_add(cmzn_fieldmodule *fieldmodule,
	cmzn_field *source_field_one, double scale_factor1,
	cmzn_field *source_field_two, double scale_factor2)
{
	cmzn_field *field, *source_fields[2];
	double source_values[2];

	/* Access and broadcast before checking components match,
		the local source_field_one and source_field_two will
		get replaced if necessary. */
	ACCESS(Computed_field)(source_field_one);
	ACCESS(Computed_field)(source_field_two);
	if (fieldmodule && source_field_one && source_field_one->isNumerical() &&
		source_field_two && source_field_two->isNumerical() &&
		Computed_field_broadcast_field_components(fieldmodule,
			&source_field_one, &source_field_two) &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		source_values[0] = scale_factor1;
		source_values[1] = scale_factor2;
		field = Computed_field_create_generic(fieldmodule,
			/*check_source_field_regions*/true,
			source_field_one->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/2, source_values,
			new Computed_field_add());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_fieldmodule_create_field_weighted_add.  Invalid argument(s)");
		field = nullptr;
	}
	DEACCESS(Computed_field)(&source_field_one);
	DEACCESS(Computed_field)(&source_field_two);

	return (field);
}

cmzn_field *cmzn_fieldmodule_create_field_add(cmzn_fieldmodule *field_module,
	cmzn_field *source_field_one,
	cmzn_field *source_field_two)
{
	return(cmzn_fieldmodule_create_field_weighted_add(field_module,
		source_field_one, 1.0, source_field_two, 1.0));
}

cmzn_field *cmzn_fieldmodule_create_field_subtract(cmzn_fieldmodule *field_module,
	cmzn_field *source_field_one,
	cmzn_field *source_field_two)
{
	cmzn_field *field = cmzn_fieldmodule_create_field_weighted_add(field_module,
		source_field_one, 1.0, source_field_two, -1.0);
	if (field && field->core)
	{
		Computed_field_add *fieldAdd= static_cast<Computed_field_add*>(
			field->core);
		fieldAdd->type = CMZN_FIELD_TYPE_SUBTRACT;
	}
	return field;
}

int Computed_field_get_type_weighted_add(cmzn_field *field,
	cmzn_field **source_field_one, FE_value *scale_factor1,
	cmzn_field **source_field_two, FE_value *scale_factor2)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_WEIGHTED_ADD, the
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

const char computed_field_scale_type_string[] = "scale";

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

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	int list();

	char* get_command_string();

	virtual enum FieldAssignmentResult assign(cmzn_fieldcache& cache, RealFieldValueCache& valueCache);

	virtual int propagate_find_element_xi(cmzn_fieldcache& field_cache,
		const FE_value *values, int number_of_values,
		struct FE_element **element_address, FE_value *xi,
		cmzn_mesh_id mesh);
};

int Computed_field_scale::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	const RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
		for (int i = 0; i < field->number_of_components; ++i)
			valueCache.values[i] = field->source_values[i]*sourceCache->values[i];
		return 1;
	}
	return 0;
}

int Computed_field_scale::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	const DerivativeValueCache *sourceDerivativeCache = getSourceField(0)->evaluateDerivative(cache, fieldDerivative);
	if (sourceDerivativeCache)
	{
		const FE_value *sourceDerivative = sourceDerivativeCache->values;
		FE_value *derivative = inValueCache.getDerivativeValueCache(fieldDerivative)->values;
		const int number_of_xi = fieldDerivative.getTermCount();
		for (int i = 0; i < field->number_of_components; ++i)
		{
			for (int j = 0; j < number_of_xi; ++j)
			{
				*derivative = (*sourceDerivative) * field->source_values[i];
				++derivative;
				++sourceDerivative;
			}
		}
		return 1;
	}
	return 0;
}

enum FieldAssignmentResult Computed_field_scale::assign(cmzn_fieldcache& cache, RealFieldValueCache& valueCache)
{
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->getValueCache(cache));
	for (int i = 0; i < field->number_of_components; ++i)
	{
		FE_value scale_value = valueCache.values[i];
		if (0.0 == scale_value)
			return FIELD_ASSIGNMENT_RESULT_FAIL;
		sourceCache->values[i] = valueCache.values[i] / scale_value;
	}
	return getSourceField(0)->assign(cache, *sourceCache);
}

int Computed_field_scale::propagate_find_element_xi(cmzn_fieldcache& field_cache,
	const FE_value *values, int number_of_values, struct FE_element **element_address,
	FE_value *xi, cmzn_mesh_id mesh)
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

/**
 * Create field of type COMPUTED_FIELD_SCALE which scales the values of the
 * <source_field> by <scale_factors>.
 * Sets the number of components equal to that of <source_field>.
 * Not exposed in the API as this is really just a multiply with constant
 */
cmzn_field *cmzn_fieldmodule_create_field_scale(cmzn_fieldmodule *field_module,
	cmzn_field *source_field, double *scale_factors)
{
	cmzn_field_id field = 0;
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
}

int Computed_field_get_type_scale(cmzn_field *field,
	cmzn_field **source_field, double **scale_factors)
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

const char computed_field_clamp_maximum_type_string[] = "clamp_maximum";

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

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	int list();

	char* get_command_string();

	enum FieldAssignmentResult assign(cmzn_fieldcache& cache, RealFieldValueCache& valueCache);

};

int Computed_field_clamp_maximum::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	const RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
		for (int i = 0; i < field->number_of_components; ++i)
			valueCache.values[i] = (sourceCache->values[i] < field->source_values[i]) ? sourceCache->values[i] : field->source_values[i];
		return 1;
	}
	return 0;
}

int Computed_field_clamp_maximum::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	const RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	const DerivativeValueCache *sourceDerivativeCache = getSourceField(0)->evaluateDerivative(cache, fieldDerivative);
	if (sourceCache && sourceDerivativeCache)
	{
		FE_value *derivative = inValueCache.getDerivativeValueCache(fieldDerivative)->values;
		const int termCount = fieldDerivative.getTermCount();
		for (int i = 0; i < field->number_of_components; ++i)
		{
			if (sourceCache->values[i] < field->source_values[i])
			{
				const FE_value *sourceDerivative = sourceDerivativeCache->values + i*termCount;
				for (int j = 0; j < termCount; ++j)
				{
					*derivative = *sourceDerivative;
					++sourceDerivative;
					++derivative;
				}
			}
			else
			{
				for (int j = 0; j < termCount; ++j)
				{
					*derivative = 0.0;
					++derivative;
				}
			}
		}
		return 1;
	}
	return 0;
}

/** clamps to limits of maximums when setting values too */
enum FieldAssignmentResult Computed_field_clamp_maximum::assign(cmzn_fieldcache& cache, RealFieldValueCache& valueCache)
{
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->getValueCache(cache));
	enum FieldAssignmentResult result1 = FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
	for (int i = 0; i < field->number_of_components; ++i)
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

/**
 * Create field of type COMPUTED_FIELD_CLAMP_MAXIMUM with the supplied
 * <source_field> and <maximums>.  Each component is clamped by its respective limit
 * in <maximums>.
 * The <maximums> array must therefore contain as many FE_values as there are
 * components in <source_field>.
 * SAB.  I think this should be changed so that the maximums come from a source
 * field rather than constant maximums before it is exposed in the API.
 */
cmzn_field *cmzn_fieldmodule_create_field_clamp_maximum(cmzn_fieldmodule *fieldmodule,
	cmzn_field *source_field, double *maximums)
{
	cmzn_field_id field = 0;
	if (source_field && source_field->isNumerical())
	{
		field = Computed_field_create_generic(fieldmodule,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/source_field->number_of_components, maximums,
			new Computed_field_clamp_maximum());
	}
	return (field);
}

int Computed_field_get_type_clamp_maximum(cmzn_field *field,
	cmzn_field **source_field, double **maximums)
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

const char computed_field_clamp_minimum_type_string[] = "clamp_minimum";

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

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	int list();

	char* get_command_string();

	enum FieldAssignmentResult assign(cmzn_fieldcache& cache, RealFieldValueCache& valueCache);
};

int Computed_field_clamp_minimum::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	const RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
		for (int i = 0; i < field->number_of_components; ++i)
			valueCache.values[i] = (sourceCache->values[i] > field->source_values[i]) ? sourceCache->values[i] : field->source_values[i];
		return 1;
	}
	return 0;
}

int Computed_field_clamp_minimum::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	const RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	const DerivativeValueCache *sourceDerivativeCache = getSourceField(0)->evaluateDerivative(cache, fieldDerivative);
	if (sourceCache && sourceDerivativeCache)
	{
		FE_value *derivative = inValueCache.getDerivativeValueCache(fieldDerivative)->values;
		const int termCount = fieldDerivative.getTermCount();
		for (int i = 0; i < field->number_of_components; ++i)
		{
			if (sourceCache->values[i] > field->source_values[i])
			{
				const FE_value *sourceDerivative = sourceDerivativeCache->values + i*termCount;
				for (int j = 0; j < termCount; ++j)
				{
					*derivative = *sourceDerivative;
					++sourceDerivative;
					++derivative;
				}
			}
			else
			{
				for (int j = 0; j < termCount; ++j)
				{
					*derivative = 0.0;
					++derivative;
				}
			}
		}
		return 1;
	}
	return 0;
}

/** clamps to limits of minimums when setting values too */
enum FieldAssignmentResult Computed_field_clamp_minimum::assign(cmzn_fieldcache& cache, RealFieldValueCache& valueCache)
{
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->getValueCache(cache));
	enum FieldAssignmentResult result1 = FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
	for (int i = 0; i < field->number_of_components; ++i)
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

/**
 * Create field of type COMPUTED_FIELD_CLAMP_MINIMUM with the supplied
 * <source_field> and <minimums>.  Each component is clamped by its respective limit
 * in <minimums>.
 * The <minimums> array must therefore contain as many FE_values as there are
 * components in <source_field>.
 * SAB.  I think this should be changed so that the minimums come from a source
 * field rather than constant minimums before it is exposed in the API.
 */
cmzn_field *cmzn_fieldmodule_create_field_clamp_minimum(cmzn_fieldmodule *fieldmodule,
	cmzn_field *source_field, double *minimums)
{
	cmzn_field_id field = 0;
	if (source_field && source_field->isNumerical())
	{
		field = Computed_field_create_generic(fieldmodule,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/source_field->number_of_components, minimums,
			new Computed_field_clamp_minimum());
	}
	return (field);
}

int Computed_field_get_type_clamp_minimum(cmzn_field *field,
	cmzn_field **source_field, double **minimums)
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

const char computed_field_offset_type_string[] = "offset";

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

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	int list();

	char* get_command_string();

	enum FieldAssignmentResult assign(cmzn_fieldcache& cache, RealFieldValueCache& valueCache);

	virtual int propagate_find_element_xi(cmzn_fieldcache& field_cache,
		const FE_value *values, int number_of_values,
		struct FE_element **element_address, FE_value *xi,
		cmzn_mesh_id mesh);
};

int Computed_field_offset::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	const RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
		for (int i = 0; i < field->number_of_components; ++i)
			valueCache.values[i] = sourceCache->values[i] + field->source_values[i];
		return 1;
	}
	return 0;
}

int Computed_field_offset::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	const DerivativeValueCache *sourceDerivativeCache = getSourceField(0)->evaluateDerivative(cache, fieldDerivative);
	if (sourceDerivativeCache)
	{
		inValueCache.getDerivativeValueCache(fieldDerivative)->copyValues(*sourceDerivativeCache);
		return 1;
	}
	return 0;
}

enum FieldAssignmentResult Computed_field_offset::assign(cmzn_fieldcache& cache, RealFieldValueCache& valueCache)
{
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->getValueCache(cache));
	for (int i = 0; i < field->number_of_components; ++i)
	{
		sourceCache->values[i] = valueCache.values[i] - field->source_values[i];
	}
	return getSourceField(0)->assign(cache, *sourceCache);
}

int Computed_field_offset::propagate_find_element_xi(cmzn_fieldcache& field_cache,
	const FE_value *values, int number_of_values, struct FE_element **element_address,
	FE_value *xi, cmzn_mesh_id mesh)
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

/**
 * Create type COMPUTED_FIELD_OFFSET which returns the values of the
 * <source_field> plus the <offsets>.
 * The <offsets> array must therefore contain as many FE_values as there are
 * components in <source_field>; this is the number of components in the field.
 * Not exposed in the API is this is just an add with constant field.
 */
cmzn_field *cmzn_fieldmodule_create_field_offset(cmzn_fieldmodule *fieldmodule,
	cmzn_field *source_field, double *offsets)
{
	cmzn_field_id field = nullptr;
	if (source_field && source_field->isNumerical())
	{
		field = Computed_field_create_generic(fieldmodule,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/source_field->number_of_components, offsets,
			new Computed_field_offset());
	}
	return (field);
}

int Computed_field_get_type_offset(cmzn_field *field,
	cmzn_field **source_field, double **offsets)
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

const char computed_field_edit_mask_type_string[] = "edit_mask";

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

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	int list();

	char* get_command_string();

	enum FieldAssignmentResult assign(cmzn_fieldcache& cache, RealFieldValueCache& valueCache);
};

int Computed_field_edit_mask::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	const RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		/* exact copy of source field */
		valueCache.copyValues(*sourceCache);
		return 1;
	}
	return 0;
}

int Computed_field_edit_mask::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	const DerivativeValueCache *sourceDerivativeCache = getSourceField(0)->evaluateDerivative(cache, fieldDerivative);
	if (sourceDerivativeCache)
	{
		inValueCache.getDerivativeValueCache(fieldDerivative)->copyValues(*sourceDerivativeCache);
		return 1;
	}
	return 0;
}

/* assigns only components for which edit mask value is non-zero */
enum FieldAssignmentResult Computed_field_edit_mask::assign(cmzn_fieldcache& cache, RealFieldValueCache& valueCache)
{
	if (getSourceField(0)->evaluate(cache))
	{
		// get non-const sourceCache to modify:
		RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->getValueCache(cache));
		for (int i = 0; i < field->number_of_components; ++i)
		{
			if (field->source_values[i])
			{
				sourceCache->values[i] = valueCache.values[i];
			}
		}
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

/**
 * Create field of to type COMPUTED_FIELD_EDIT_MASK, returning the <source_field>
 * with each component edit_masked by its respective FE_value in <edit_mask>, ie.
 * if the edit_mask value for a component is non-zero, the component is editable.
 * The <edit_mask> array must therefore contain as many FE_values as there are
 * components in <source_field>.
 * Sets the number of components to the same as <source_field>.
 * If function fails, field is guaranteed to be unchanged from its original state,
 * although its cache may be lost.
 */
cmzn_field *cmzn_fieldmodule_create_field_edit_mask(cmzn_fieldmodule *field_module,
	cmzn_field *source_field, double *edit_mask)
{
	cmzn_field_id field = 0;
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
}

int Computed_field_get_type_edit_mask(cmzn_field *field,
	cmzn_field **source_field, double **edit_mask)
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

const char computed_field_log_type_string[] = "log";

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

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_LOG;
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

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	int list();

	char* get_command_string();
};

int Computed_field_log::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	const RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
		for (int i = 0; i < field->number_of_components; ++i)
			valueCache.values[i] = log(sourceCache->values[i]);
		return 1;
	}
	return 0;
}

int Computed_field_log::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	if (fieldDerivative.getOrder() > 1)
		return 0;  // fall back to numerical derivatives
	const RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluateDerivativeTree(cache, fieldDerivative));
	if (sourceCache)
	{
		const FE_value *sourcevalues = sourceCache->values;
		const FE_value *sourceDerivatives = sourceCache->getDerivativeValueCache(fieldDerivative)->values;
		FE_value *derivative = inValueCache.getDerivativeValueCache(fieldDerivative)->values;
		const int number_of_xi = fieldDerivative.getTermCount();
		for (int i = 0; i < field->number_of_components; ++i)
		{
			for (int j = 0; j < number_of_xi; ++j)
			{
				/* d(log u)/dx = 1 / u * du/dx */
				*derivative = 1.0 / sourcevalues[i] * sourceDerivatives[i * number_of_xi + j];
				++derivative;
			}
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

cmzn_field *cmzn_fieldmodule_create_field_log(cmzn_fieldmodule *field_module,
	cmzn_field *source_field)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_LOG with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/
{
	cmzn_field_id field = 0;
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
}

int Computed_field_get_type_log(cmzn_field *field,
	cmzn_field **source_field)
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

const char computed_field_sqrt_type_string[] = "sqrt";

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

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_SQRT;
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

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	int list();

	char* get_command_string();
};

int Computed_field_sqrt::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	const RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
		for (int i = 0; i < field->number_of_components; ++i)
			valueCache.values[i] = sqrt(sourceCache->values[i]);
		return 1;
	}
	return 0;
}

int Computed_field_sqrt::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	if (fieldDerivative.getOrder() > 1)
		return 0;  // fall back to numerical derivatives
	const RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluateDerivativeTree(cache, fieldDerivative));
	if (sourceCache)
	{
		const FE_value *sourceValues = sourceCache->values;
		const FE_value *sourceDerivatives = sourceCache->getDerivativeValueCache(fieldDerivative)->values;
		FE_value *derivative = inValueCache.getDerivativeValueCache(fieldDerivative)->values;
		const int number_of_xi = fieldDerivative.getTermCount();
		for (int i = 0; i < field->number_of_components; ++i)
		{
			const FE_value dsqrt_u = 0.5 / sqrt(sourceValues[i]);
			for (int j = 0; j < number_of_xi; ++j)
			{
				/* d(sqrt u)/dx = du/dx / 2 sqrt(u) */
				*derivative = sourceDerivatives[i * number_of_xi + j] * dsqrt_u;
				derivative++;
			}
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

cmzn_field *cmzn_fieldmodule_create_field_sqrt(cmzn_fieldmodule *field_module,
	cmzn_field *source_field)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SQRT with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/
{
	cmzn_field_id field = 0;
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
}

int Computed_field_get_type_sqrt(cmzn_field *field,
	cmzn_field **source_field)
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

const char computed_field_exp_type_string[] = "exp";

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

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_EXP;
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

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	int list();

	char* get_command_string();
};

int Computed_field_exp::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	const RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
		for (int i = 0; i < field->number_of_components; ++i)
			valueCache.values[i] = exp(sourceCache->values[i]);
		return 1;
	}
	return 0;
}

int Computed_field_exp::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	if (fieldDerivative.getOrder() > 1)
		return 0;  // fall back to numerical derivatives
	const RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluateDerivativeTree(cache, fieldDerivative));
	if (sourceCache)
	{
		const FE_value *sourcevalues = sourceCache->values;
		const FE_value *sourceDerivatives = sourceCache->getDerivativeValueCache(fieldDerivative)->values;
		FE_value *derivative = inValueCache.getDerivativeValueCache(fieldDerivative)->values;
		const int number_of_xi = fieldDerivative.getTermCount();
		for (int i = 0; i < field->number_of_components; ++i)
		{
			for (int j = 0; j < number_of_xi; ++j)
			{
				/* d(exp u)/dx = du/dx exp(u) */
				*derivative = sourceDerivatives[i * number_of_xi + j] * exp(sourcevalues[i]);
				derivative++;
			}
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

cmzn_field *cmzn_fieldmodule_create_field_exp(cmzn_fieldmodule *field_module,
	cmzn_field *source_field)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_EXP with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/
{
	cmzn_field_id field = 0;
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
}

int Computed_field_get_type_exp(cmzn_field *field,
	cmzn_field **source_field)
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

const char computed_field_abs_type_string[] = "abs";

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

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_ABS;
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

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	int list();

	char* get_command_string();
};

int Computed_field_abs::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	const RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
		for (int i = 0; i < field->number_of_components; ++i)
			valueCache.values[i] = fabs(sourceCache->values[i]);
		return 1;
	}
	return 0;
}

int Computed_field_abs::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	const RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	const DerivativeValueCache *sourceDerivativeCache = getSourceField(0)->evaluateDerivative(cache, fieldDerivative);
	if (sourceCache && sourceDerivativeCache)
	{
		const FE_value *sourceDerivative = sourceDerivativeCache->values;
		FE_value *derivative = inValueCache.getDerivativeValueCache(fieldDerivative)->values;
		const int termCount = fieldDerivative.getTermCount();
		for (int i = 0; i < field->number_of_components; ++i)
		{
			/* d(abs u)/dx =  du/dx u>0
			 *               -du/dx u<0
			 *               and lets just put 0 at u=0 */
			if (sourceCache->values[i] > 0.0)
			{
				for (int j = 0; j < termCount; ++j)
				{
					*derivative = *sourceDerivative;
					++sourceDerivative;
					++derivative;
				}
			}
			else if (sourceCache->values[i] < 0.0)
			{
				for (int j = 0; j < termCount; ++j)
				{
					*derivative = -(*sourceDerivative);
					++sourceDerivative;
					++derivative;
				}
			}
			else
			{
				for (int j = 0; j < termCount; ++j)
				{
					*derivative = 0.0;
					++sourceDerivative;
					derivative++;
				}
			}
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

cmzn_field *cmzn_fieldmodule_create_field_abs(cmzn_fieldmodule *field_module,
	cmzn_field *source_field)
/*******************************************************************************
DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_EXP with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
==============================================================================*/
{
	cmzn_field_id field = 0;
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
}

int Computed_field_get_type_abs(cmzn_field *field,
	cmzn_field **source_field)
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

