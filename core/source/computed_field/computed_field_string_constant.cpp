/*******************************************************************************
FILE : computed_field_string_constant.c

LAST MODIFIED : 25 August 2006

DESCRIPTION :
Implements a constant string field.
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
#include <math.h>
#include "api/cmiss_field_constant.h"
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

	virtual FieldValueCache *createValueCache(Cmiss_field_cache& /*parentCache*/)
	{
		return new StringFieldValueCache();
	}

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	virtual bool is_defined_at_location(Cmiss_field_cache&)
	{
		return true;
	}

	int has_numerical_components()
	{
		return 0;
	}

	virtual enum FieldAssignmentResult assign(Cmiss_field_cache& /*cache*/, StringFieldValueCache& /*valueCache*/);

	virtual Cmiss_field_value_type get_value_type() const
	{
		return CMISS_FIELD_VALUE_TYPE_STRING;
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

int Computed_field_string_constant::evaluate(Cmiss_field_cache&, FieldValueCache& inValueCache)
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

enum FieldAssignmentResult Computed_field_string_constant::assign(Cmiss_field_cache& cache, StringFieldValueCache& valueCache)
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

struct Computed_field *Cmiss_field_module_create_string_constant(
	struct Cmiss_field_module *field_module, const char *string_value_in)
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

