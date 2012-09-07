/*******************************************************************************
FILE : computed_field_composite.c

LAST MODIFIED : 18 May 2008

DESCRIPTION :
Implements a "composite" computed_field which converts fields, field components
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
//-- extern "C" {
#include <stdlib.h>
#include "api/cmiss_field_module.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_composite.h"
//-- }
#include "computed_field/computed_field_private.hpp"
//-- extern "C" {
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
//-- }

/*
Module types
------------
*/

struct Computed_field_component
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Used to specify a component of a Computed_field with function
set_Computed_field_component.
???RC Note that in its current use the field is NOT assumed to be accessed by
this structure in set_Computed_field_component.
==============================================================================*/
{
	struct Computed_field *field;
	int component_no;
}; /* struct Computed_field_component */

class Computed_field_composite_package : public Computed_field_type_package
{
};

namespace {

char computed_field_composite_type_string[] = "composite";

class Computed_field_composite : public Computed_field_core
{
public:
	/* following are allocated with enough space for the number of components
		 in the field. <source_field_numbers> are indices into the source fields
		 for the field, with <source_values_numbers> referring to component number,
		 with both arrays starting at 0. If <source_field_numbers> is -1 then
		 the <source_value_numbers> at the same index contains the index into the
		 source values for the field. */
	int *source_field_numbers;
	int *source_value_numbers;

	Computed_field_composite(int number_of_components,
		const int *source_field_numbers_in, const int *source_value_numbers_in) : Computed_field_core()
	{
		int i;
		source_field_numbers = new int[number_of_components];
		source_value_numbers = new int[number_of_components];
		for (i = 0 ; i < number_of_components ; i++)
		{
			source_field_numbers[i] = source_field_numbers_in[i];
			source_value_numbers[i] = source_value_numbers_in[i];
		}
	}

	~Computed_field_composite();

	char *get_source_string(int commands);

private:

	Computed_field_core* copy()
	{
		return (new Computed_field_composite(field->number_of_components,
			source_field_numbers, source_value_numbers));
	}

	const char* get_type_string()
	{
		return(computed_field_composite_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	virtual enum FieldAssignmentResult assign(Cmiss_field_cache& /*cache*/, RealFieldValueCache& /*valueCache*/);

	virtual int propagate_find_element_xi(Cmiss_field_cache& field_cache,
		const FE_value *values, int number_of_values,
		struct FE_element **element_address, FE_value *xi,
		Cmiss_mesh_id mesh);
};

Computed_field_composite::~Computed_field_composite()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	ENTER(Computed_field_composite::~Computed_field_composite);
	if (field)
	{
		if (source_field_numbers)
		{
			delete [] source_field_numbers;
		}
		if (source_value_numbers)
		{
			delete [] source_value_numbers;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_composite::~Computed_field_composite.  "
			"Invalid arguments.");
	}
	LEAVE;
} /* Computed_field_composite::~Computed_field_composite */

int Computed_field_composite::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int i, return_code;
	Computed_field_composite* other;

	ENTER(Computed_field_composite::compare);
	if (field && (other = dynamic_cast<Computed_field_composite*>(other_core)))
	{
		return_code=1;
		for (i=0;return_code&&(i<field->number_of_components);i++)
		{
			if ((source_field_numbers[i] !=
				other->source_field_numbers[i]) ||
				(source_value_numbers[i] !=
					other->source_value_numbers[i]))
			{
				return_code=0;
			}
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_composite::compare */

int Computed_field_composite::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	// try to avoid allocating cache array
	const int CacheStackSize = 10;
	RealFieldValueCache *fixedValueCache[CacheStackSize];
	RealFieldValueCache **sourceValueCache = (field->number_of_source_fields <= CacheStackSize) ?
		fixedValueCache : new RealFieldValueCache*[field->number_of_source_fields];
	int return_code = 1;
	int number_of_derivatives = cache.getRequestedDerivatives();
	for (int i = 0; i < field->number_of_source_fields; ++i)
	{
		sourceValueCache[i] = RealFieldValueCache::cast(getSourceField(i)->evaluate(cache));
		if (!sourceValueCache[i])
		{
			return_code = 0;
			break;
		}
		if (number_of_derivatives && !sourceValueCache[i]->derivatives_valid)
			number_of_derivatives = 0;
	}
	if (return_code)
	{
		RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
		valueCache.derivatives_valid = number_of_derivatives;
		FE_value *destination = number_of_derivatives ? valueCache.derivatives : 0;
		for (int i=0;i<field->number_of_components;i++)
		{
			if (0 <= source_field_numbers[i])
			{
				valueCache.values[i] = sourceValueCache[source_field_numbers[i]]->
					values[source_value_numbers[i]];
				if (valueCache.derivatives_valid)
				{
					/* source field component */
					FE_value *source = sourceValueCache[source_field_numbers[i]]->derivatives +
						source_value_numbers[i]*number_of_derivatives;
					for (int j=0;j<number_of_derivatives;j++)
					{
						*destination = *source;
						destination++;
						source++;
					}
				}
			}
			else
			{
				valueCache.values[i] = field->source_values[source_value_numbers[i]];
				if (valueCache.derivatives_valid)
				{
					for (int j=0;j<number_of_derivatives;j++)
					{
						*destination = 0.0;
						destination++;
					}
				}
			}
		}
	}
	if (sourceValueCache != fixedValueCache)
		delete[] sourceValueCache;
	return (return_code);
}

enum FieldAssignmentResult Computed_field_composite::assign(Cmiss_field_cache& cache, RealFieldValueCache& valueCache)
{
	/* go through each source field, getting current values, changing values
		for components that are used in the composite field, then setting the
		whole field in one hit */
	enum FieldAssignmentResult result = FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
	for (int source_field_number=0;
		  (source_field_number<field->number_of_source_fields);
		  source_field_number++)
	{
		RealFieldValueCache *sourceValueCache = RealFieldValueCache::cast(field->evaluate(cache));
		if (!sourceValueCache)
			return FIELD_ASSIGNMENT_RESULT_FAIL;
		for (int i=0;i<field->number_of_components;i++)
		{
			if (source_field_numbers[i] == source_field_number)
			{
				sourceValueCache->values[source_value_numbers[i]] = valueCache.values[i];
			}
		}
		enum FieldAssignmentResult thisResult = field->assign(cache, *sourceValueCache);
		if (thisResult == FIELD_ASSIGNMENT_RESULT_FAIL)
			return FIELD_ASSIGNMENT_RESULT_FAIL;
		if ((result == FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET) &&
			(thisResult == FIELD_ASSIGNMENT_RESULT_PARTIAL_VALUES_SET))
		{
			result = FIELD_ASSIGNMENT_RESULT_PARTIAL_VALUES_SET;
		}
	}
	// don't assign values to constants if assigning to cache
	if (!cache.assignInCacheOnly())
	{
		bool changed = false;
		for (int i = 0; i < field->number_of_components; i++)
		{
			if (-1 == source_field_numbers[i])
			{
				field->source_values[source_value_numbers[i]] = valueCache.values[i];
				changed = true;
			}
		}
		if (changed)
		{
			Computed_field_changed(field);
		}
	}
	return result;
}

int Computed_field_composite::propagate_find_element_xi(Cmiss_field_cache& field_cache,
	const FE_value *values, int number_of_values, struct FE_element **element_address,
	FE_value *xi, Cmiss_mesh_id mesh)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Currently only tries to work if there is only one and exactly one source field.
Zero is used for any source field values that aren't set from the composite field.
==============================================================================*/
{
	int i, number_of_source_values, return_code, source_field_number;
	FE_value *source_values;
	
	ENTER(Computed_field_composite::propagate_find_element_xi);
	if (field && values && (number_of_values == field->number_of_components))
	{
		return_code=1;
		if (field->number_of_source_fields == 1)
		{
			source_field_number = 0;
			number_of_source_values = field->source_fields[source_field_number]->number_of_components;
			if (ALLOCATE(source_values,FE_value,number_of_source_values))
			{
				/* Put zero into every value initially */
				for (i=0;i<number_of_source_values;i++)
				{
					source_values[i] = 0.0;
				}
				for (i=0;i<field->number_of_components;i++)
				{
					if (source_field_number == source_field_numbers[i])
					{
						source_values[source_value_numbers[i]] = values[i];
					}
				}
				return_code = Computed_field_find_element_xi(
					field->source_fields[source_field_number], &field_cache, source_values,
					number_of_values, element_address, xi, mesh,
					/*propagate_field*/1, /*find_nearest*/0);
				DEALLOCATE(source_values);
			}
			else
			{
				return_code=0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_composite::propagate_find_element_xi.  Failed");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_composite::propagate_find_element_xi.  "
				"Unable to find element xi on a composite field involving more than one source field.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_composite::propagate_find_element_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_composite::propagate_find_element_xi */

char *Computed_field_composite::get_source_string(int commands)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns the field components and constants that make up the composite field in
a single string. If all components of a field are used consecutively, just the
field name is put in the final string, otherwise individual components appear.
Field/component names are made into valid tokens for commands.
If <commands> is set, field/components are made into valid tokens.
==============================================================================*/
{
	char *component_name, *source_string, tmp_string[40], *token;
	int error, i, j, source_number_of_components, token_error, whole_field;
	struct Computed_field *source_field;

	ENTER(Computed_field_composite_get_source_string);
	source_string = (char *)NULL;
	if (field)
	{
		error = 0;
		for (i = 0; (i < field->number_of_components) && !error; i++)
		{
			if (0 < i)
			{
				append_string(&source_string, " ", &error);
			}
			if (0 <= source_field_numbers[i])
			{
				source_field = field->source_fields[source_field_numbers[i]];
				/* source field component */
				if (GET_NAME(Computed_field)(source_field, &token))
				{
					token_error = 0;
					source_number_of_components = source_field->number_of_components;
					whole_field = 1;
					for (j = 0; whole_field && (j < source_number_of_components); j++)
					{
						whole_field = ((i + j) < field->number_of_components) &&
							(j == source_value_numbers[i + j]);
					}
					if (whole_field)
					{
						i += source_number_of_components - 1;
					}
					else
					{
						component_name = Computed_field_get_component_name(source_field,
							source_value_numbers[i]);
						if (component_name != 0)
						{
							append_string(&token, ".", &token_error);
							append_string(&token, component_name, &token_error);
							DEALLOCATE(component_name);
						}
					}
					if (commands)
					{
						make_valid_token(&token);
					}
					append_string(&source_string, token, &error);
					DEALLOCATE(token);
				}
			}
			else
			{
				/* source value */
				sprintf(tmp_string, "%g",
					field->source_values[source_value_numbers[i]]);
				append_string(&source_string, tmp_string, &error);
			}
		}
		if (error)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_composite_get_source_string.  Invalid argument(s)");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_composite_get_source_string.  Invalid argument(s)");
	}
	LEAVE;

	return (source_string);
} /* Computed_field_composite_get_source_string */

int Computed_field_composite::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	char *source_string;
	int return_code;

	ENTER(List_Computed_field_composite);
	if (field)
	{
		source_string = get_source_string(/*commands*/0);
		if (source_string != 0)
		{
			display_message(INFORMATION_MESSAGE,"    source data : %s\n",
				source_string);
			DEALLOCATE(source_string);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"list_Computed_field_composite.  Failed");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_composite.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_composite */

char *Computed_field_composite::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *source_string;
	int error;

	ENTER(Computed_field_composite::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		/* could restore constant/component types here too */
		append_string(&command_string,
			computed_field_composite_type_string, &error);
		append_string(&command_string, " ", &error);
		source_string = get_source_string(/*commands*/1);
		if (source_string != 0)
		{
			append_string(&command_string, source_string, &error);
			DEALLOCATE(source_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_composite::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_composite::get_command_string */

} //namespace

struct Computed_field *Computed_field_create_composite(
	struct Cmiss_field_module *field_module,
	int number_of_components,
	int number_of_source_fields,struct Computed_field **source_fields,
	int number_of_source_values, const double *source_values,
	const int *source_field_numbers, const int *source_value_numbers)
{
	int i, j, return_code, source_field_number, source_value_number;
	Computed_field *field, *source_field;

	ENTER(Computed_field_create_composite);
	field = (Computed_field *)NULL;
	if ((0<number_of_components) &&
		((0==number_of_source_fields) ||
			((0<number_of_source_fields) && source_fields)) &&
		((0==number_of_source_values) ||
			((0<number_of_source_values) && source_values)) &&
		source_field_numbers && source_value_numbers)
	{
		return_code=1;
		/* check all source fields exist and are not repeated */
		for (i=0;i<number_of_source_fields;i++)
		{
			if (source_fields[i] && source_fields[i]->isNumerical())
			{
				for (j=0;j<i;j++)
				{
					if (source_fields[j] == source_fields[i])
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_create_composite.  Repeated source field");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_create_composite.  Missing or non-numerical source field");
				return 0;
			}
		}
		if (return_code)
		{
			/* Check source_field_numbers refer to existing source_fields or equal -1.
				 Check components or source_values exist for the source_value_numbers
				 depending on whether field component or value specified. Also
				 ensure that values and fields are first used in the order they
				 appear in the source_values and source_fields */
			source_field_number=0;
			source_value_number=0;
			for (i=0;i<number_of_components;i++)
			{
				if (-1 == source_field_numbers[i])
				{
					/* source value numbers must be non-repeating and consecutive */
					if (source_value_number != source_value_numbers[i])
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_create_composite.  "
							"Source value numbers out of order");
						return_code=0;
					}
					source_value_number++;
				}
				else if ((0 <= source_field_numbers[i]) &&
					(source_field_numbers[i] < number_of_source_fields))
				{
					if (source_field_numbers[i] <= source_field_number)
					{
						if (source_field_numbers[i] == source_field_number)
						{
							source_field_number++;
						}
						source_field = source_fields[source_field_numbers[i]];
						if ((0 > source_value_numbers[i]) ||
							(source_value_numbers[i] >= source_field->number_of_components))
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_create_composite.  "
								"Component %d is out of range for field %s",
								source_value_numbers[i],source_field->name);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_create_composite.  "
							"source fields not used in order");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_create_composite.  "
						"Invalid source field number %d",source_field_numbers[i]);
					return_code=0;
				}
			}
			if (source_field_number < number_of_source_fields)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_create_composite.  Not all source fields used");
				return_code=0;
			}
			if (source_value_number < number_of_source_values)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_create_composite.  Not all source values used");
				return_code=0;
			}
		}
		if (return_code)
		{
			field = Computed_field_create_generic(field_module,
				/*check_source_field_regions*/true,
				number_of_components,
				number_of_source_fields, source_fields,
				number_of_source_values, source_values,
				new Computed_field_composite(number_of_components,
					source_field_numbers, source_value_numbers));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_composite.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* Computed_field_create_composite */

int Computed_field_get_type_composite(struct Computed_field *field,
	int *number_of_components,
	int *number_of_source_fields,struct Computed_field ***source_fields,
	int *number_of_source_values, double **source_values,
	int **source_field_numbers,int **source_value_numbers)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type composite, its source data is returned.
Note that <source_fields> and <source_values> are ALLOCATED (or NULL if none),
and <source_field_numbers> and <source_value_numbers> are always allocated on
return, and it is therefore up to the calling function to deallocate.
Note returned fields are not allocated in arrays.
==============================================================================*/
{
	int i, return_code;
	Computed_field_composite* composite_core;

	ENTER(Computed_field_get_type_composite);
	if (field && 
		(composite_core = dynamic_cast<Computed_field_composite*>(field->core)) &&
		number_of_components &&
		number_of_source_fields && source_fields &&
		number_of_source_values && source_values &&
		source_field_numbers && source_value_numbers)
	{
		*number_of_components = field->number_of_components;
		*number_of_source_fields = field->number_of_source_fields;
		*source_fields = (struct Computed_field **)NULL;
		*number_of_source_values = field->number_of_source_values;
		*source_values = (double *)NULL;
		*source_field_numbers = (int *)NULL;
		*source_value_numbers = (int *)NULL;
		if (((0 == field->number_of_source_fields) || ALLOCATE(*source_fields,
			struct Computed_field *, field->number_of_source_fields)) &&
			((0 == field->number_of_source_values) || ALLOCATE(*source_values,
				double, field->number_of_source_values)) &&
			ALLOCATE(*source_field_numbers, int, field->number_of_components) &&
			ALLOCATE(*source_value_numbers, int, field->number_of_components))
		{
			for (i = 0; i < field->number_of_source_fields; i++)
			{
				(*source_fields)[i] = field->source_fields[i];
			}
			for (i = 0; i < field->number_of_source_values; i++)
			{
				(*source_values)[i] = field->source_values[i];
			}
			for (i = 0; i < field->number_of_components; i++)
			{
				(*source_field_numbers)[i] = composite_core->source_field_numbers[i];
				(*source_value_numbers)[i] = composite_core->source_value_numbers[i];
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_composite.  Not enough memory");
			DEALLOCATE(*source_fields);
			DEALLOCATE(*source_values);
			DEALLOCATE(*source_field_numbers);
			DEALLOCATE(*source_value_numbers);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_composite.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_composite */

struct Computed_field_composite_source_data
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Data structure filled by set_Computed_field_composite_source_data.
==============================================================================*/
{
	int number_of_components;
	int number_of_source_fields;
	struct Computed_field **source_fields;
	int number_of_source_values;
	double *source_values;
	int *source_field_numbers;
	int *source_value_numbers;
}; /* struct Computed_field_composite_source_data */

struct Computed_field *Computed_field_create_constant(
	struct Cmiss_field_module *field_module,
	int number_of_values, const double *values)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Creates a constructor for COMPUTED_FIELD_COMPOSITE which has constant
components of the <number_of_values> listed in <values> array.
Since a constant field performs a subset of the capabilities of the composite
field type but the latter is somewhat complicated to set up, this is a
convenience function for building a composite field which has <number_of_values>
<values>.
==============================================================================*/
{
	int i, *source_field_numbers, *source_value_numbers;
	Computed_field *field = NULL;

	ENTER(Computed_field_create_constant);
	if ((0 < number_of_values) && values)
	{
		ALLOCATE(source_field_numbers, int, number_of_values);
		ALLOCATE(source_value_numbers, int, number_of_values);
		if (source_field_numbers && source_value_numbers)
		{
			for (i = 0; i < number_of_values; i++)
			{
				source_field_numbers[i] = -1;
				source_value_numbers[i] = i;
			}
			field = Computed_field_create_composite(field_module,
				/*number_of_components*/number_of_values,
				/*number_of_source_fields*/0,
				/*source_fields*/(struct Computed_field **)NULL,
				/*number_of_source_values*/number_of_values,
				/*source_values*/values,
				source_field_numbers, source_value_numbers);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_create_constant.  Not enough memory");
		}
		DEALLOCATE(source_field_numbers);
		DEALLOCATE(source_value_numbers);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_constant.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* Computed_field_create_constant */

FE_value *Computed_field_constant_get_values_storage(struct Computed_field *field)
{
	if (Computed_field_is_constant(field))
		return field->source_values;
	return 0;
}

int Computed_field_is_constant(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns true if the field is of type composite but with no source_fields, only
source_values.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_constant);
	if (field)
	{
		return_code =
			(dynamic_cast<Computed_field_composite*>(field->core)) &&
			(0 == field->number_of_source_fields);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_constant.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_constant */

int Computed_field_is_constant_scalar(struct Computed_field *field,
	FE_value scalar)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns true if the field is of type composite but with no source_fields, only
a single source value, equal to <scalar>.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_constant_scalar);
	if (field)
	{
		return_code =
			(dynamic_cast<Computed_field_composite*>(field->core)) &&
			(0 == field->number_of_source_fields) &&
			(1 == field->number_of_components) &&
			(scalar == field->source_values[0]);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_constant_scalar.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_constant_scalar */

int Computed_field_is_component_wrapper(struct Computed_field *field,
	void *field_component_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns true if the field is of type composite with a single component matching
the passed Computed_field_component.
==============================================================================*/
{
	Computed_field_composite *composite_core;
	int return_code;
	struct Computed_field_component *field_component;

	ENTER(Computed_field_is_component_wrapper);
	if (field && (field_component =
		(struct Computed_field_component *)field_component_void))
	{
		return_code = 
			(composite_core = dynamic_cast<Computed_field_composite*>(field->core)) &&
			(1 == field->number_of_components) &&
			(1 == field->number_of_source_fields) &&
			(field_component->field == field->source_fields[0]) &&
			(0 == composite_core->source_field_numbers[0]) &&
			(field_component->component_no == composite_core->source_value_numbers[0]);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_component_wrapper.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_component_wrapper */

struct Computed_field *Computed_field_manager_get_component_wrapper(
	struct MANAGER(Computed_field) *computed_field_manager,
	struct Computed_field *field, int component_number)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns a composite field that computes <component_number> of <field>. First
tries to find one in the manager that does this, otherwise makes one of name
'field.component_name', adds it to the manager and returns it.
Returned field is ACCESSed once.
==============================================================================*/
{
	char *component_field_name, *component_name;
	struct Computed_field *component_field;
	struct Computed_field_component field_component;

	ENTER(Computed_field_manager_get_component_wrapper);
	component_field = (struct Computed_field *)NULL;
	if (computed_field_manager && field && (0 <= component_number) &&
		(component_number < field->number_of_components))
	{
		field_component.field = field;
		field_component.component_no = component_number;
		/* try to find an existing wrapper for this component */
		component_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
			Computed_field_is_component_wrapper, &field_component,
			computed_field_manager);
		if (component_field != 0)
		{
			ACCESS(Computed_field)(component_field);
		}
		else
		{
			component_name =
				Computed_field_get_component_name(field, component_number);
			if (component_name != 0)
			{
				if (ALLOCATE(component_field_name, char,
					strlen(field->name) + strlen(component_name) + 2))
				{
					sprintf(component_field_name, "%s.%s", field->name, component_name);
					Cmiss_region* region = Computed_field_get_region(field);
					Cmiss_field_module *field_module = Cmiss_field_module_create(region);
					Cmiss_field_module_set_field_name(field_module, component_field_name);
					component_field = Computed_field_create_component(field_module, field, component_number);
					Cmiss_field_module_destroy(&field_module);
					DEALLOCATE(component_field_name);
				}
				DEALLOCATE(component_name);
			}
			if (!component_field)
			{
				display_message(WARNING_MESSAGE,
					"Computed_field_manager_get_component_wrapper.  Failed");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_manager_get_component_wrapper.  Invalid argument(s)");
	}
	LEAVE;

	return (component_field);
} /* Computed_field_manager_get_component_wrapper */

Computed_field *Cmiss_field_module_create_identity(
	struct Cmiss_field_module *field_module,
	Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Changes <field> into type composite with one input field, the <source_field>.
==============================================================================*/
{
	int i, number_of_values, *source_field_numbers,
		*source_value_numbers;
	Computed_field *field = NULL;

	ENTER(Computed_field_create_identity);
	if (source_field && source_field->isNumerical())
	{
		number_of_values = source_field->number_of_components;
		ALLOCATE(source_field_numbers, int, number_of_values);
		ALLOCATE(source_value_numbers, int, number_of_values);
		if (source_field_numbers && source_value_numbers)
		{
			for (i = 0; i < number_of_values; i++)
			{
				source_field_numbers[i] = 0;
				source_value_numbers[i] = i;
			}
			field =
				Computed_field_create_composite(field_module,
				/*number_of_components*/number_of_values,
				/*number_of_source_fields*/1, /*source_fields*/&source_field,
				/*number_of_source_values*/0, /*source_values*/(double *)NULL,
				source_field_numbers, source_value_numbers);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_create_identity.  Not enough memory");
		}
		DEALLOCATE(source_field_numbers);
		DEALLOCATE(source_value_numbers);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_identity.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* Computed_field_create_identity */

Computed_field *Cmiss_field_module_create_component(
	struct Cmiss_field_module *field_module,
	Computed_field *source_field, int component_number)
/*******************************************************************************

DESCRIPTION :
Changes <field> into type composite with one input field, the <source_field>
and the component index <component_number>.
==============================================================================*/
{
	int number_of_components, *source_field_numbers,
		*source_value_numbers;
	Computed_field *field = NULL;

	ENTER(Computed_field_create_component);
	if (source_field && source_field->isNumerical())
	{
		number_of_components = 1;
		ALLOCATE(source_field_numbers, int, number_of_components);
		ALLOCATE(source_value_numbers, int, number_of_components);
		if (source_field_numbers && source_value_numbers)
		{
			source_field_numbers[0] = 0;
			source_value_numbers[0] = component_number;
			field =
				Computed_field_create_composite(field_module,
				number_of_components,
				/*number_of_source_fields*/1, /*source_fields*/&source_field,
				/*number_of_source_values*/0, /*source_values*/(double *)NULL,
				source_field_numbers, source_value_numbers);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_create_component.  Not enough memory");
		}
		DEALLOCATE(source_field_numbers);
		DEALLOCATE(source_value_numbers);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_component.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* Computed_field_create_component */

struct Computed_field *Computed_field_create_concatenate(
	struct Cmiss_field_module *field_module,
	int number_of_source_fields, struct Computed_field **source_fields)
{
	Computed_field *field = NULL;
	
	if (source_fields && number_of_source_fields > 0)
	{
		int *source_field_numbers, *source_value_numbers, i, j, k,
			number_of_components_per_field, number_of_components;
		number_of_components = 0;
		for (i = 0; i < number_of_source_fields; i++)
		{
			if (!(source_fields[i] && source_fields[i]->isNumerical()))
				return 0;
			number_of_components += 
				Computed_field_get_number_of_components(source_fields[i]);
		}
		ALLOCATE(source_field_numbers, int, number_of_components);
		ALLOCATE(source_value_numbers, int, number_of_components);
		if (source_field_numbers && source_value_numbers)
		{
			k = 0;
			for (i = 0; i < number_of_source_fields; i++)
			{
				number_of_components_per_field = 
					Computed_field_get_number_of_components(source_fields[i]);
				for (j = 0; j < number_of_components_per_field; j++)
				{
					source_field_numbers[k + j] = i;
					source_value_numbers[k + j] = j;
				}
				k += number_of_components_per_field;
			}
			field = Computed_field_create_composite(field_module,
				number_of_components,
				number_of_source_fields, source_fields,
				/*number_of_source_values*/0, /*source_values*/(double *)NULL,
				source_field_numbers, source_value_numbers);
		}
		DEALLOCATE(source_field_numbers);
		DEALLOCATE(source_value_numbers);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_concatenate.  Invalid argument(s)");
	}

	return(field);
}

