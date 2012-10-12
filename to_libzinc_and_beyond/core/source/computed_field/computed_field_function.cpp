/*******************************************************************************
FILE : computed_field_function.c

LAST MODIFIED : 31 March 2008

DESCRIPTION :
Implements a "function" computed_field which converts fields, field components
and real values in any order into a single vector field.
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
#include <stdlib.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_function.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"

/*
Module types
------------
*/

class Computed_field_function_package : public Computed_field_type_package
{
};

namespace {

const char computed_field_function_type_string[] = "function";

class Computed_field_function : public Computed_field_core
{
public:

	Computed_field_function() : Computed_field_core()
	{
	}

	~Computed_field_function()
	{
	}

private:
	Computed_field_core* copy();

	const char* get_type_string()
	{
		return(computed_field_function_type_string);
	}

	int compare(Computed_field_core* other_field);

	virtual FieldValueCache *createValueCache(Cmiss_field_cache& parentCache)
	{
		RealFieldValueCache *valueCache = new RealFieldValueCache(field->number_of_components);
		valueCache->createExtraCache(parentCache, Computed_field_get_region(field));
		return valueCache;
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

};

Computed_field_core* Computed_field_function::copy()
{
	Computed_field_function* core = new Computed_field_function();

	return (core);
}

int Computed_field_function::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_function::compare);
	if (field && dynamic_cast<Computed_field_function*>(other_core))
	{
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_function::compare */

int Computed_field_function::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	Cmiss_field_id sourceField = getSourceField(0);
	Cmiss_field_id resultField = getSourceField(1);
	Cmiss_field_id referenceField = getSourceField(2);
	RealFieldValueCache *sourceCache = RealFieldValueCache::cast(sourceField->evaluate(cache));
	if (sourceCache)
	{
		RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
		Cmiss_field_cache& extraCache = *valueCache.getExtraCache();
		extraCache.setTime(cache.getTime());
		int number_of_xi = cache.getRequestedDerivatives();
		if ((sourceField->number_of_components == referenceField->number_of_components))
		{
			RealFieldValueCache *resultCache = 0;
			if (number_of_xi && sourceCache->derivatives_valid)
			{
				extraCache.setFieldRealWithDerivatives(referenceField, referenceField->number_of_components,
					sourceCache->values, number_of_xi, sourceCache->derivatives);
				resultCache = RealFieldValueCache::cast(resultField->evaluateWithDerivatives(extraCache, number_of_xi));
			}
			else
			{
				extraCache.setFieldReal(referenceField, referenceField->number_of_components, sourceCache->values);
				resultCache = RealFieldValueCache::cast(resultField->evaluate(extraCache));
			}
			if (resultCache)
			{
				valueCache.copyValues(*resultCache);
				return 1;
			}
		}
		else
		{
			/* Apply the scalar function operation to each source field component */
			valueCache.derivatives_valid = sourceCache->derivatives_valid;
			for (int i = 0; i < field->number_of_components; i++)
			{
				RealFieldValueCache *resultCache = 0;
				if (valueCache.derivatives_valid)
				{
					extraCache.setFieldRealWithDerivatives(referenceField, 1,
						sourceCache->values + i, number_of_xi, sourceCache->derivatives + i*number_of_xi);
					resultCache = RealFieldValueCache::cast(resultField->evaluateWithDerivatives(extraCache, number_of_xi));
				}
				else
				{
					extraCache.setFieldReal(referenceField, 1, sourceCache->values + i);
					resultCache = RealFieldValueCache::cast(resultField->evaluate(extraCache));
				}
				if (!resultCache)
					return 0;
				valueCache.values[i] = resultCache->values[0];
				if (valueCache.derivatives_valid)
				{
					if (resultCache->derivatives_valid)
					{
						for (int j=0;j<number_of_xi;j++)
						{
							valueCache.derivatives[i*number_of_xi+j] = resultCache->derivatives[j];
						}
					}
					else
					{
						valueCache.derivatives_valid = 0;
					}
				}
			}
			return 1;
		}
	}
	return 0;
}

int Computed_field_function::list()
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_function);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    result field : %s\n",field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,
			"    reference field : %s\n",field->source_fields[2]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_function.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_function */

char *Computed_field_function::get_command_string()
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_function::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_function_type_string, &error);

		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " result_field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}

		append_string(&command_string, " reference_field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[2], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_function::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_function::get_command_string */

} //namespace

struct Computed_field *Computed_field_create_function(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field, struct Computed_field *result_field,
	struct Computed_field *reference_field)
{
	Computed_field *field = NULL;
	if (source_field && result_field && reference_field &&
		((source_field->number_of_components ==
			reference_field->number_of_components) ||
			((1 == reference_field->number_of_components) &&
			 (1 == result_field->number_of_components))))
	{
		int number_of_components = 0;
		if ((source_field->number_of_components ==
			reference_field->number_of_components))
		{
			number_of_components = result_field->number_of_components;
		}
		else
		{
			number_of_components = source_field->number_of_components;
		}
		Computed_field *source_fields[3];
		source_fields[0] = source_field;
		source_fields[1] = result_field;
		source_fields[2] = reference_field;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			number_of_components,
			/*number_of_source_fields*/3, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_function());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_function.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
}

int Computed_field_get_type_function(Computed_field *field,
	Computed_field **source_field, Computed_field **result_field,
	Computed_field **reference_field)
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
If the field is of type COMPUTED_FIELD_FUNCTION, the function returns the three
fields which define the field.
Note that the fields are not ACCESSed and the <region_path> points to the
internally used path.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_function);
	if (field && dynamic_cast<Computed_field_function*>(field->core) &&
		source_field && result_field && reference_field)
	{
		*source_field = field->source_fields[0];
		*result_field = field->source_fields[1];
		*reference_field = field->source_fields[2];
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_function.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_function */

