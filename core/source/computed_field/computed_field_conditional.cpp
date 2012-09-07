/*******************************************************************************
FILE : computed_field_conditional.cpp

LAST MODIFIED : 16 May 2008

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
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
#include "computed_field/computed_field_conditional.h"

class Computed_field_conditional_package : public Computed_field_type_package
{
};

namespace {

char computed_field_if_type_string[] = "if";

class Computed_field_if : public Computed_field_core
{
public:
	Computed_field_if() : Computed_field_core()
	{
	};

private:
	Computed_field_core *copy()
	{
		return new Computed_field_if();
	}

	const char *get_type_string()
	{
		return(computed_field_if_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_if*>(other_field))
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

int Computed_field_if::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	RealFieldValueCache *source1Cache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (source1Cache)
	{
		/* 2. Work out whether we need to evaluate source_field_two
			or source_field_three or both. */
		int calculate_field_two = 0;
		int calculate_field_three = 0;
		for (int i = 0 ; i < field->number_of_components ; i++)
		{
			if (source1Cache->values[i])
			{
				calculate_field_two = 1;
			}
			else
			{
				calculate_field_three = 1;
			}
		}
		RealFieldValueCache *source2Cache = 0;
		RealFieldValueCache *source3Cache = 0;
		if (calculate_field_two)
		{
			source2Cache = RealFieldValueCache::cast(getSourceField(1)->evaluate(cache));
		}
		if (calculate_field_three)
		{
			source3Cache = RealFieldValueCache::cast(getSourceField(2)->evaluate(cache));
		}
		if (((!calculate_field_two) || source2Cache) && ((!calculate_field_three) || source3Cache))
		{
			int number_of_xi = cache.getRequestedDerivatives();
			FE_value *derivative = valueCache.derivatives;
			valueCache.derivatives_valid = (0 != number_of_xi);
			for (int i = 0 ; i < field->number_of_components ; i++)
			{
				RealFieldValueCache *useSourceCache;
				if (source1Cache->values[i])
				{
					useSourceCache = source2Cache;
				}
				else
				{
					useSourceCache = source3Cache;
				}
				valueCache.values[i] = useSourceCache->values[i];
				if (valueCache.derivatives_valid)
				{
					if (useSourceCache->derivatives_valid)
					{
						for (int j = 0 ; j < number_of_xi ; j++)
						{
							*derivative = useSourceCache->derivatives[i * number_of_xi + j];
							derivative++;
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

int Computed_field_if::list()
/*******************************************************************************
LAST MODIFIED : 27 July 2007

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_if);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source fields : %s %s\n",field->source_fields[0]->name,
			field->source_fields[1]->name, field->source_fields[2]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_if.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_if */

char *Computed_field_if::get_command_string()
/*******************************************************************************
LAST MODIFIED : 27 July 2007

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_if::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_if_type_string, &error);
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
		if (GET_NAME(Computed_field)(field->source_fields[2], &field_name))
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
			"Computed_field_if::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_if::get_command_string */

} //namespace

Computed_field *Computed_field_create_if(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two,
	struct Computed_field *source_field_three)
/*******************************************************************************
LAST MODIFIED : 16 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_IF with the supplied
fields, <source_field_one>, <source_field_two> and <source_field_three>.
Sets the number of components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	Computed_field *field = NULL;

	ENTER(Computed_field_create_if);
	if (source_field_one && source_field_one->isNumerical() &&
		source_field_two && source_field_two->isNumerical() &&
		source_field_three && source_field_three->isNumerical() &&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components)&&
		(source_field_one->number_of_components ==
			source_field_three->number_of_components))
	{
		Computed_field *source_fields[3];
		source_fields[0] = source_field_one;
		source_fields[1] = source_field_two;
		source_fields[2] = source_field_three;
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field_one->number_of_components,
			/*number_of_source_fields*/3, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_if());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_if.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* Computed_field_create_if */

int Computed_field_get_type_if(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two,
	struct Computed_field **source_field_three)
/*******************************************************************************
LAST MODIFIED : 27 July 2007

DESCRIPTION :
If the field is of type COMPUTED_FIELD_IF, the 
<source_field_one>, <source_field_two> and <source_field_three> used by it
are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_if);
	if (field&&(dynamic_cast<Computed_field_if*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		*source_field_three = field->source_fields[2];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_if.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_if */

