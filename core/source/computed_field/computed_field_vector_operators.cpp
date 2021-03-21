/***************************************************************************//**
 * FILE : computed_field_vector_operators.cpp
 *
 * Implements a number of basic vector operations on computed fields.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <math.h>
#include "opencmiss/zinc/fieldvectoroperators.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_vector_operators.hpp"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "general/message.h"

class Computed_field_vector_operators_package : public Computed_field_type_package
{
};

namespace {

char computed_field_normalise_type_string[] = "normalise";

class Computed_field_normalise : public Computed_field_core
{
public:
	Computed_field_normalise() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_normalise();
	}

	const char *get_type_string()
	{
		return(computed_field_normalise_type_string);
	}

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_NORMALISE;
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_normalise*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

int Computed_field_normalise::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		FE_value size = 0.0;
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
			size += sourceCache->values[i] *	sourceCache->values[i];
		}
		size = sqrt(size);
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
			valueCache.values[i] = sourceCache->values[i] / size;
		}
		int number_of_xi = cache.getRequestedDerivatives();
		if (number_of_xi && sourceCache->derivatives_valid)
		{
			FE_value *derivative = valueCache.derivatives;
			FE_value *source_derivative = sourceCache->derivatives;
			for (int i = 0 ; i < field->number_of_components * number_of_xi ; i++)
			{
				*derivative = *source_derivative / size;
				derivative++;
				source_derivative++;
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

int Computed_field_normalise::list(
	)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_normalise);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_normalise.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_normalise */

char *Computed_field_normalise::get_command_string(
	)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_normalise::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_normalise_type_string, &error);
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
			"Computed_field_normalise::get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_normalise::get_command_string */

} //namespace

struct Computed_field *cmzn_fieldmodule_create_field_normalise(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field)
{
	cmzn_field_id field = 0;
	if (source_field)
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_normalise());
	}
	return (field);
} /* Computed_field_set_type_normalise */

int Computed_field_get_type_normalise(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_NORMALISE, the
<source_field> used by it is returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_normalise);
	if (field && (dynamic_cast<Computed_field_normalise*>(field->core)) &&
		source_field)
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_normalise.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_normalise */

namespace {

char computed_field_cross_product_type_string[] = "cross_product";

class Computed_field_cross_product : public Computed_field_core
{
public:
	Computed_field_cross_product() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_cross_product();
	}

	const char *get_type_string()
	{
		return(computed_field_cross_product_type_string);
	}

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_CROSS_PRODUCT;
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_cross_product*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

int Computed_field_cross_product::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache[3];
	int derivatives_valid = 1;
	for (int i = 0; i < field->number_of_source_fields; ++i)
	{
		sourceCache[i] = RealFieldValueCache::cast(getSourceField(i)->evaluate(cache));
		if (!sourceCache[i])
			return 0;
		if (!sourceCache[i]->derivatives_valid)
			derivatives_valid = 0;
	}
	switch (field->number_of_components)
	{
		case 1:
		{
			valueCache.values = 0;
		} break;
		case 2:
		{
			valueCache.values[0] = -sourceCache[0]->values[1];
			valueCache.values[1] = sourceCache[0]->values[0];
		} break;
		case 3:
		{
			cross_product_FE_value_vector3(sourceCache[0]->values,
				sourceCache[1]->values, valueCache.values);
		} break;
		case 4:
		{
			cross_product_FE_value_vector4(sourceCache[0]->values,
				sourceCache[1]->values, sourceCache[2]->values, valueCache.values);
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_cross_product::evaluate.  "
				"Unsupported number of components.");
			return 0;
		} break;
	}
	int number_of_xi = cache.getRequestedDerivatives();
	if (number_of_xi && derivatives_valid)
	{
		FE_value *derivative, *source_derivative;
		FE_value temp_vector[16]; // max field->number_of_components*field->number_of_components
		switch (field->number_of_components)
		{
			case 1:
			{
				derivative = valueCache.derivatives;
				for (int i = 0 ; i < number_of_xi ; i++)
				{
					*derivative = 0;
					derivative++;
				}
			} break;
			case 2:
			{
				derivative = valueCache.derivatives;
				source_derivative = valueCache.derivatives + number_of_xi;
				for (int i = 0 ; i < number_of_xi ; i++)
				{
					*derivative = -*source_derivative;
					derivative++;
					source_derivative++;
				}
				source_derivative = valueCache.derivatives + number_of_xi;
				for (int i = 0 ; i < number_of_xi ; i++)
				{
					*derivative = *source_derivative;
					derivative++;
					source_derivative++;
				}
			} break;
			case 3:
			{
				for (int j = 0 ; j < number_of_xi ; j++)
				{
					for (int i = 0 ; i < 3 ; i++)
					{
						temp_vector[i] = sourceCache[0]->derivatives[i * number_of_xi + j];
						temp_vector[i + 3] = sourceCache[1]->derivatives[i * number_of_xi + j];
					}
					cross_product_FE_value_vector3(temp_vector,
						sourceCache[1]->values, temp_vector + 6);
					for (int i = 0 ; i < 3 ; i++)
					{
						valueCache.derivatives[i * number_of_xi + j] =
							temp_vector[i + 6];
					}
					cross_product_FE_value_vector3(
						sourceCache[0]->values,
						temp_vector + 3, temp_vector + 6);
					for (int i = 0 ; i < 3 ; i++)
					{
						valueCache.derivatives[i * number_of_xi + j] +=
							temp_vector[i + 6];
					}
				}
			} break;
			case 4:
			{
				for (int j = 0 ; j < number_of_xi ; j++)
				{
					for (int i = 0 ; i < 4 ; i++)
					{
						temp_vector[i] = sourceCache[0]->derivatives[i * number_of_xi + j];
						temp_vector[i + 4] = sourceCache[1]->derivatives[i * number_of_xi + j];
						temp_vector[i + 8] = sourceCache[2]->derivatives[i * number_of_xi + j];
					}
					cross_product_FE_value_vector4(temp_vector,
						sourceCache[1]->values,
						sourceCache[2]->values, temp_vector + 12);
					for (int i = 0 ; i < 4 ; i++)
					{
						valueCache.derivatives[i * number_of_xi + j] =
							temp_vector[i + 12];
					}
					cross_product_FE_value_vector4(
						sourceCache[0]->values, temp_vector + 4,
						sourceCache[2]->values, temp_vector + 12);
					for (int i = 0 ; i < 4 ; i++)
					{
						valueCache.derivatives[i * number_of_xi + j] +=
							temp_vector[i + 12];
					}
					cross_product_FE_value_vector4(
						sourceCache[0]->values,
						sourceCache[1]->values,
						temp_vector + 8, temp_vector + 12);
					for (int i = 0 ; i < 4 ; i++)
					{
						valueCache.derivatives[i * number_of_xi + j] +=
							temp_vector[i + 12];
					}
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_cross_product::evaluate.  "
					"Unsupported number of components.");
				return 0;
			} break;
		}
		valueCache.derivatives_valid = 1;
	}
	else
	{
		valueCache.derivatives_valid = 0;
	}
	return 1;
}

int Computed_field_cross_product::list(
	)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(List_Computed_field_cross_product);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    dimension : %d\n",field->number_of_components);
		display_message(INFORMATION_MESSAGE,"    source fields :");
		for (i = 0 ; i < field->number_of_components - 1 ; i++)
		{
			display_message(INFORMATION_MESSAGE," %s",
				field->source_fields[i]->name);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_cross_product.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_cross_product */

char *Computed_field_cross_product::get_command_string(
	)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error, i;

	ENTER(Computed_field_cross_product::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_cross_product_type_string, &error);
		sprintf(temp_string, " dimension %d", field->number_of_components);
		append_string(&command_string, temp_string, &error);
		append_string(&command_string, " fields", &error);
		for (i = 0 ; i < field->number_of_components - 1 ; i++)
		{
			if (GET_NAME(Computed_field)(field->source_fields[i], &field_name))
			{
				make_valid_token(&field_name);
				append_string(&command_string, " ", &error);
				append_string(&command_string, field_name, &error);
				DEALLOCATE(field_name);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cross_product::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_cross_product::get_command_string */

} //namespace

struct Computed_field *cmzn_fieldmodule_create_field_cross_product(
	struct cmzn_fieldmodule *field_module,
	int number_of_source_fields, struct Computed_field **source_fields)
{
	Computed_field *field = NULL;
	if ((0 < number_of_source_fields) && (number_of_source_fields <= 3) && source_fields)
	{
		const int dimension = number_of_source_fields + 1;
		int return_code = 1;
		for (int i = 0 ; i < number_of_source_fields; i++)
		{
			if (!source_fields[i] ||
				(source_fields[i]->number_of_components != dimension))
			{
				display_message(ERROR_MESSAGE,
					"cmzn_fieldmodule_create_field_cross_product.  "
					"Source field %d missing or has wrong number of components", i + 1);
				return_code = 0;
			}
		}
		if (return_code)
		{
			field = Computed_field_create_generic(field_module,
				/*check_source_field_regions*/true,
				/*number_of_components*/dimension,
				/*number_of_source_fields*/number_of_source_fields, source_fields,
				/*number_of_source_values*/0, NULL,
				new Computed_field_cross_product());
		}
	}
	return (field);
}

cmzn_field_id cmzn_fieldmodule_create_field_cross_product_3d(
	cmzn_fieldmodule_id field_module, cmzn_field_id source_field_one,
	cmzn_field_id source_field_two)
{
	cmzn_field_id source_fields[2];
	source_fields[0] = source_field_one;
	source_fields[1] = source_field_two;
	return cmzn_fieldmodule_create_field_cross_product(field_module, 2, source_fields);
}

int Computed_field_get_type_cross_product(struct Computed_field *field,
	int *dimension, struct Computed_field ***source_fields)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CROSS_PRODUCT, the
<dimension> and <source_fields> used by it are returned.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_get_type_cross_product);
	if (field&&(dynamic_cast<Computed_field_cross_product*>(field->core))
		&&source_fields)
	{
		*dimension = field->number_of_components;
		if (ALLOCATE(*source_fields,struct Computed_field *,
			field->number_of_source_fields))
		{
			for (i=0;i<field->number_of_source_fields;i++)
			{
				(*source_fields)[i]=field->source_fields[i];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_cross_product.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_cross_product.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_cross_product */

namespace {

char computed_field_dot_product_type_string[] = "dot_product";

class Computed_field_dot_product : public Computed_field_core
{
public:
	Computed_field_dot_product() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_dot_product();
	}

	const char *get_type_string()
	{
		return(computed_field_dot_product_type_string);
	}

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_DOT_PRODUCT;
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_dot_product*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

int Computed_field_dot_product::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	RealFieldValueCache *source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (source1Cache && source2Cache)
	{
		int vector_number_of_components = getSourceField(0)->number_of_components;
		FE_value sum = 0.0;
		for (int i=0;i < vector_number_of_components;i++)
		{
			sum += source1Cache->values[i] * source2Cache->values[i];
		}
		valueCache.values[0] = sum;
		int number_of_xi = cache.getRequestedDerivatives();
		if (number_of_xi && source1Cache->derivatives_valid && source2Cache->derivatives_valid)
		{
			for (int j=0;j<number_of_xi;j++)
				valueCache.derivatives[j] = 0.0;
			FE_value *temp=source1Cache->values;
			FE_value *temp2=source2Cache->derivatives;
			for (int i=0;i < vector_number_of_components;i++)
			{
				for (int j=0;j<number_of_xi;j++)
				{
					valueCache.derivatives[j] += (*temp)*(*temp2);
					temp2++;
				}
				temp++;
			}
			temp=source2Cache->values;
			temp2=source1Cache->derivatives;
			for (int i=0;i < vector_number_of_components;i++)
			{
				for (int j=0;j<number_of_xi;j++)
				{
					valueCache.derivatives[j] += (*temp)*(*temp2);
					temp2++;
				}
				temp++;
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

int Computed_field_dot_product::list(
	)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_dot_product);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    field 1 : %s\n    field 2 : %s\n",
			field->source_fields[0]->name, field->source_fields[1]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_dot_product.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_dot_product */

char *Computed_field_dot_product::get_command_string(
	)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_dot_product::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_dot_product_type_string, &error);
		append_string(&command_string, " fields ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_dot_product::get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_dot_product::get_command_string */

} //namespace

struct Computed_field *cmzn_fieldmodule_create_field_dot_product(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
{
	struct Computed_field *field = NULL;
	if (source_field_one && source_field_two &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		Computed_field *source_fields[2];
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			/*number_of_components*/1,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_dot_product());
	}
	return (field);
}

int Computed_field_get_type_dot_product(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_DOT_PRODUCT, the
<source_field_one> and <source_field_two> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_dot_product);
	if (field && (dynamic_cast<Computed_field_dot_product*>(field->core)) &&
		source_field_one && source_field_two)
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_dot_product.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_dot_product */

namespace {

char computed_field_magnitude_type_string[] = "magnitude";

class Computed_field_magnitude : public Computed_field_core
{
public:
	Computed_field_magnitude() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_magnitude();
	}

	const char *get_type_string()
	{
		return(computed_field_magnitude_type_string);
	}

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_MAGNITUDE;
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_magnitude*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	virtual enum FieldAssignmentResult assign(cmzn_fieldcache& /*cache*/, RealFieldValueCache& /*valueCache*/);

};

int Computed_field_magnitude::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		int source_number_of_components = getSourceField(0)->number_of_components;
		FE_value *source_values = sourceCache->values;
		FE_value sum = 0.0;
		for (int i=0;i<source_number_of_components;i++)
		{
			sum += source_values[i]*source_values[i];
		}
		valueCache.values[0] = sqrt(sum);

		int number_of_xi = cache.getRequestedDerivatives();
		if (number_of_xi && sourceCache->derivatives_valid)
		{
			FE_value *source_derivatives=sourceCache->derivatives;
			for (int j=0;j<number_of_xi;j++)
			{
				sum = 0.0;
				for (int i=0;i<source_number_of_components;i++)
				{
					sum += source_values[i]*source_derivatives[i*number_of_xi+j];
				}
				valueCache.derivatives[j] = sum / valueCache.values[0];
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

enum FieldAssignmentResult Computed_field_magnitude::assign(cmzn_fieldcache& cache, RealFieldValueCache& valueCache)
{
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (!sourceCache)
		return FIELD_ASSIGNMENT_RESULT_FAIL;
	const int source_number_of_components = getSourceField(0)->number_of_components;
	FE_value scale_value = 0.0;
	for (int i=0;i<source_number_of_components;i++)
	{
		scale_value += sourceCache->values[i] * sourceCache->values[i];
	}
	if (0.0 >= scale_value)
		return FIELD_ASSIGNMENT_RESULT_FAIL;
	scale_value = valueCache.values[0] / sqrt(scale_value);
	for (int i=0;i<source_number_of_components;i++)
	{
		sourceCache->values[i] *= scale_value;
	}
	sourceCache->derivatives_valid = 0;
	return getSourceField(0)->assign(cache, *sourceCache);
}

int Computed_field_magnitude::list(
	)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_magnitude);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_magnitude.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_magnitude */

char *Computed_field_magnitude::get_command_string(
	)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_magnitude::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_magnitude_type_string, &error);
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
			"Computed_field_magnitude::get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_magnitude::get_command_string */

} //namespace

struct Computed_field *cmzn_fieldmodule_create_field_magnitude(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field)
{
	Computed_field *field = Computed_field_create_generic(field_module,
		/*check_source_field_regions*/true,
		/*number_of_components*/1,
		/*number_of_source_fields*/1, &source_field,
		/*number_of_source_values*/0, NULL,
		new Computed_field_magnitude());

	return (field);
} /* Computed_field_set_type_normalise */

int Computed_field_get_type_magnitude(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_MAGNITUDE, the
<source_field> used by it is returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_magnitude);
	if (field && (dynamic_cast<Computed_field_magnitude*>(field->core)) &&
		source_field)
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_magnitude.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_magnitude */

namespace {

const char computed_field_sum_components_type_string[] = "sum_components";

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

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_SUM_COMPONENTS;
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

	int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

int Computed_field_sum_components::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		int i, j;
		/* sum of components of source field */
		FE_value sum = 0.0;
		const int source_number_of_components = field->source_fields[0]->number_of_components;
		for (i = 0; i < source_number_of_components; i++)
		{
			sum += sourceCache->values[i];
		}
		valueCache.values[0] = sum;
		int number_of_xi = cache.getRequestedDerivatives();
		if (number_of_xi && sourceCache->derivatives_valid)
		{
			for (j = 0; j < number_of_xi; j++)
			{
				FE_value *temp = sourceCache->derivatives + j;
				sum = 0.0;
				for (i = 0; i < source_number_of_components; i++)
				{
					sum += (*temp);
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
	int return_code;

	ENTER(List_Computed_field_sum_components);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field : %s\n",
			field->source_fields[0]->name);
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
	char *command_string, *field_name;
	int error;

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

cmzn_field_id cmzn_fieldmodule_create_field_sum_components(
	cmzn_fieldmodule_id field_module, cmzn_field_id source_field)
{
	cmzn_field_id field = 0;
	if (source_field && source_field->isNumerical())
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			/*number_of_components*/1,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_sum_components());
	}
	return (field);
}

namespace {

char computed_field_cubic_texture_coordinates_type_string[] = "cubic_texture_coordinates";

class Computed_field_cubic_texture_coordinates : public Computed_field_core
{
public:
	Computed_field_cubic_texture_coordinates() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_cubic_texture_coordinates();
	}

	const char *get_type_string()
	{
		return(computed_field_cubic_texture_coordinates_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_cubic_texture_coordinates*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();
};

int Computed_field_cubic_texture_coordinates::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (sourceCache)
	{
		int number_of_components = field->number_of_components;
		FE_value *temp=sourceCache->values;
		valueCache.values[number_of_components - 1] = fabs(*temp);
		temp++;
		int j = 0;
		for (int i=1;i<number_of_components;i++)
		{
			if (fabs(*temp) > valueCache.values[number_of_components - 1])
			{
				valueCache.values[number_of_components - 1] = fabs(*temp);
				j = i;
			}
			temp++;
		}
		temp=sourceCache->values;
		for (int i=0;i < number_of_components - 1;i++)
		{
			if ( i == j )
			{
				/* Skip over the maximum coordinate */
				temp++;
			}
			valueCache.values[i] = *temp / valueCache.values[number_of_components - 1];
			temp++;
		}
		valueCache.derivatives_valid = 0;
		return 1;
	}
	return 0;
}

int Computed_field_cubic_texture_coordinates::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_cubic_texture_coordinates);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_cubic_texture_coordinates.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_cubic_texture_coordinates */

char *Computed_field_cubic_texture_coordinates::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_cubic_texture_coordinates::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_cubic_texture_coordinates_type_string, &error);
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
			"Computed_field_cubic_texture_coordinates::get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_cubic_texture_coordinates::get_command_string */

} // namespace

/***************************************************************************//**
 * Creates a field of type CUBIC_TEXTURE_COORDINATES with the supplied
 * <source_field>.  Sets the number of components equal to the <source_field>.
 * ???GRC Someone needs to explain what this field does.
 */
struct Computed_field *Computed_field_create_cubic_texture_coordinates(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field)
{
	cmzn_field_id field = 0;
	if (source_field)
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_cubic_texture_coordinates());
	}
	return (field);
} /* Computed_field_set_type_normalise */

int Computed_field_get_type_cubic_texture_coordinates(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type CUBIC_TEXTURE_COORDINATES, the source field used
by it is returned - otherwise an error is reported.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_cubic_texture_coordinates);
	if (field && (dynamic_cast<Computed_field_cubic_texture_coordinates*>(field->core)) &&
		source_field)
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_cubic_texture_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_cubic_texture_coordinates */

