/*******************************************************************************
FILE : computed_field_string_constant.c

LAST MODIFIED : 25 August 2006

DESCRIPTION :
Implements a constant string field.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <math.h>
#include "zinc/fieldconstant.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"

namespace {

char computed_field_string_constant_type_string[] = "string_constant";

class Computed_field_string_constant : public Computed_field_core
{
public:
	char *string_value;

	Computed_field_string_constant(const char *string_value_in) :
		Computed_field_core(),
		string_value(duplicate_string(string_value_in))
	{
	};

	virtual bool attach_to_field(Computed_field *parent)
	{
		if (Computed_field_core::attach_to_field(parent))
		{
			if (string_value)
			{
				return true;
			}
		}
		return false;
	}

	~Computed_field_string_constant()
	{
		DEALLOCATE(string_value);
	}

private:
	Computed_field_core *copy()
	{
		return new Computed_field_string_constant(string_value);
	}

	const char *get_type_string()
	{
		return(computed_field_string_constant_type_string);
	}

	int compare(Computed_field_core* other_field);

	virtual FieldValueCache *createValueCache(cmzn_field_cache& /*parentCache*/)
	{
		return new StringFieldValueCache();
	}

	int evaluate(cmzn_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	virtual bool is_defined_at_location(cmzn_field_cache&)
	{
		return true;
	}

	int has_numerical_components()
	{
		return 0;
	}

	virtual enum FieldAssignmentResult assign(cmzn_field_cache& /*cache*/, StringFieldValueCache& /*valueCache*/);

	virtual cmzn_field_value_type get_value_type() const
	{
		return CMZN_FIELD_VALUE_TYPE_STRING;
	}

};

int Computed_field_string_constant::compare(Computed_field_core* other_field)
{
	Computed_field_string_constant* other;
	int return_code;

	ENTER(Computed_field_string_constant::compare);
	if (field && (other = dynamic_cast<Computed_field_string_constant*>(other_field)))
	{
		return_code = (0 == strcmp(string_value, other->string_value));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_string_constant::compare.  Invalid argument(s)");
		return_code = 0;
	}

	return (return_code);
}

int Computed_field_string_constant::evaluate(cmzn_field_cache&, FieldValueCache& inValueCache)
{
	StringFieldValueCache& stringValueCache = StringFieldValueCache::cast(inValueCache);
	if (stringValueCache.stringValue)
		DEALLOCATE(stringValueCache.stringValue);
	stringValueCache.stringValue = duplicate_string(string_value);
	return 1;
}

int Computed_field_string_constant::list()
{
	display_message(INFORMATION_MESSAGE,
		"    string constant : %s\n", string_value);
	return 1;
}

char *Computed_field_string_constant::get_command_string()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string = 0;
	int error = 0;
	append_string(&command_string,
		computed_field_string_constant_type_string, &error);
	append_string(&command_string, " ", &error);
	char *string_token = duplicate_string(string_value);
	make_valid_token(&string_token);
	append_string(&command_string, string_token, &error);
	DEALLOCATE(string_token);
	return (command_string);
}

enum FieldAssignmentResult Computed_field_string_constant::assign(cmzn_field_cache& cache, StringFieldValueCache& valueCache)
{
	// avoid setting values in field if only assigning to cache
	if (cache.assignInCacheOnly())
	{
		return FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
	}
	if (string_value)
	{
		DEALLOCATE(string_value);
	}
	string_value = duplicate_string(valueCache.stringValue);
	Computed_field_changed(field);
	return FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
}

} //namespace

struct Computed_field *cmzn_field_module_create_string_constant(
	struct cmzn_field_module *field_module, const char *string_value_in)
{
	Computed_field *field = NULL;
	if (string_value_in)
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/false, /*number_of_components*/1,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_string_constant(string_value_in));
	}
	return (field);
}

