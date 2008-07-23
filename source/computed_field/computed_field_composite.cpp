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
extern "C" {
#include <stdlib.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_composite.h"
}
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
}

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
	FE_value** source_pointers;

	Computed_field_composite(Computed_field *field,
		int *source_field_numbers_in, int *source_value_numbers_in) : Computed_field_core(field)
	{
		int i;
		source_field_numbers = new int[field->number_of_components];
		source_value_numbers = new int[field->number_of_components];
		source_pointers = (FE_value**)NULL;
		for (i = 0 ; i < field->number_of_components ; i++)
		{
			source_field_numbers[i] = source_field_numbers_in[i];
			source_value_numbers[i] = source_value_numbers_in[i];
		}
	}

	~Computed_field_composite();

	char *get_source_string(int commands);

private:

	Computed_field_core* copy(Computed_field* new_field)
	{
		return (new Computed_field_composite(new_field,
			source_field_numbers, source_value_numbers));
	}

	char* get_type_string()
	{
		return(computed_field_composite_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int clear_cache()
	{
		if (source_pointers)
		{
			delete [] source_pointers;
			source_pointers = (FE_value**)NULL;
		}
		return (1);
	}

	int set_values_at_location(Field_location* location, FE_value *values);

	int find_element_xi(
		FE_value *values, int number_of_values, 
		struct FE_element **element, FE_value *xi, int element_dimension,
		struct Cmiss_region *search_region);
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
		if (source_pointers)
		{
			delete [] source_pointers;
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

int Computed_field_composite::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	FE_value *destination, *source;
	int component_number, number_of_derivatives, i, j, return_code,
		source_field_number;

#if ! defined (OPTIMISED)
	ENTER(Computed_field_composite::evaluate_cache_at_location);
	if (field && location)
	{
#endif /* ! defined (OPTIMISED) */
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Cache value pointers directly into source fields. */
			if (!source_pointers)
			{
				source_pointers = new FE_value*[field->number_of_components];
				for (i=0;i<field->number_of_components;i++)
				{
					if (0 <= source_field_numbers[i])
					{
						source_pointers[i] =
							&(field->source_fields[source_field_numbers[i]]->
								values[source_value_numbers[i]]);
					}
					else
					{
						source_pointers[i] =
							&field->source_values[source_value_numbers[i]];
					}
				}
			}
			/* 2. Calculate the field */
			if (number_of_derivatives = location->get_number_of_derivatives())
			{
				field->derivatives_valid = 1;
				destination=field->derivatives;
			}
			else
			{
				field->derivatives_valid = 0;
			}
			for (i=0;i<field->number_of_components;i++)
			{
				field->values[i] = *(source_pointers[i]);

				if (field->derivatives_valid)
				{
					source_field_number = source_field_numbers[i];
					component_number = source_value_numbers[i];
					if (0 <= source_field_number)
					{
						if (field->source_fields[source_field_number]->derivatives_valid)
						{
							/* source field component */
							source = field->source_fields[source_field_number]->derivatives +
								component_number*number_of_derivatives;
							for (j=0;j<number_of_derivatives;j++)
							{
								*destination = *source;
								destination++;
								source++;
							}
						}
						else
						{
							field->derivatives_valid = 0;
						}
					}
					else
					{
						/* source value */
						field->values[i] =
							field->source_values[component_number];
						if (number_of_derivatives)
						{
							for (j=0;j<number_of_derivatives;j++)
							{
								*destination = 0.0;
								destination++;
							}
						}
					}
				}
			}
		}
#if ! defined (OPTIMISED)
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_composite::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
#endif /* ! defined (OPTIMISED) */

	return (return_code);
} /* Computed_field_composite::evaluate_cache_at_location */

int Computed_field_composite::set_values_at_location(
	Field_location* location, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Sets the <values> of the computed <field> over the <element>.
==============================================================================*/
{
	FE_value *source_values;
	int i, return_code, source_field_number;
	
	ENTER(Computed_field_composite::set_values_at_location);
	if (field && location && values)
	{
		return_code=1;
		/* go through each source field, getting current values, changing values
			for components that are used in the composite field, then setting the
			whole field in one hit */
		for (source_field_number=0;
			  (source_field_number<field->number_of_source_fields)&&return_code;
			  source_field_number++)
		{
			if (ALLOCATE(source_values,FE_value,field->number_of_components))
			{
				if (Computed_field_evaluate_cache_at_location(
						 field->source_fields[source_field_number], location))
				{
					for (i=0;i<field->number_of_components;i++)
					{
						if (source_field_number == source_field_numbers[i])
						{
							source_values[source_value_numbers[i]] = values[i];
						}
						else
						{
							source_values[source_value_numbers[i]] = 
								field->source_fields[source_field_number]->values[i];	
						}
					}
					return_code=Computed_field_set_values_at_location(
						field->source_fields[source_field_number], location, 
						source_values);
				}
				else
				{
					return_code=0;
				}
				DEALLOCATE(source_values);
			}
			else
			{
				return_code = 0;
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_composite::set_values_at_location.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_composite::set_values_at_location.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_composite::set_values_at_location */

int Computed_field_composite::find_element_xi(
	 FE_value *values, int number_of_values, 
	struct FE_element **element, FE_value *xi, int element_dimension,
	struct Cmiss_region *search_region)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Currently only tries to work if there is only one and exactly one source field.
Zero is used for any source field values that aren't set from the composite field.
==============================================================================*/
{
	int i, number_of_source_values, return_code, source_field_number;
	FE_value *source_values;
	
	ENTER(Computed_field_composite::find_element_xiset_values_at_node);
	if (field && element && xi && values && (number_of_values == field->number_of_components))
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
				return_code=Computed_field_find_element_xi(
					field->source_fields[source_field_number],source_values,
					number_of_source_values,element, xi, element_dimension, search_region,
					/*propagate_field*/1, /*find_nearest_location*/0);
				DEALLOCATE(source_values);
			}
			else
			{
				return_code=0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_composite::find_element_xi.  Failed");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_composite::find_element_xi.  "
				"Unable to find element xi on a composite field involving more than one source field.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_composite::find_element_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_composite::find_element_xi */

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
						if (component_name = Computed_field_get_component_name(source_field,
							source_value_numbers[i]))
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
		if (source_string = get_source_string(/*commands*/0))
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
		if (source_string = get_source_string(/*commands*/1))
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
	int number_of_components,
	int number_of_source_fields,struct Computed_field **source_fields,
	int number_of_source_values,FE_value *source_values,
	int *source_field_numbers,int *source_value_numbers)
/*******************************************************************************
LAST MODIFIED : 13 May 2008

DESCRIPTION :
Converts <field> into a composite field which returns a combination of field
components and constants in a given order.
The <number_of_source_fields> <source_fields> may be zero if the field is
purely constant, but fields may not be repeated.
The <number_of_source_values> <source_values> may similarly be omitted.
The <source_field_numbers> and <source_value_numbers> must each contain
<number_of_components> integers. For each component the <source_field_number>
says which <source_field> to use, and the <source_value_number> specifies the
component number. If the <source_field_number> is -1 for any component, the
<source_value_number> is the offset into the <source_values>.

Note: rigorous checking is performed to ensure that no values are invalid and
that the <source_fields> are presented in the order they are first used in the
<source_field_numbers>, and also that <source_values> are used in the order
they are given and that none are used more than once.

Some of these restrictions are enforced to ensure type-specific contents can be
compared easily -- otherwise there would be more than one way to describe the
same source data.

If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	FE_value *temp_source_values;
	int i, j, return_code, source_field_number, source_value_number;
	Computed_field *field, *source_field, **temp_source_fields;

	ENTER(Computed_field_set_type_composite);

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
			if (source_fields[i])
			{
				for (j=0;j<i;j++)
				{
					if (source_fields[j] == source_fields[i])
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_set_type_composite.  Repeated source field");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_composite.  Missing source field");
				return_code=0;
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
							"Computed_field_set_type_composite.  "
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
								"Computed_field_set_type_composite.  "
								"Component %d is out of range for field %s",
								source_value_numbers[i],source_field->name);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_set_type_composite.  "
							"source fields not used in order");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_set_type_composite.  "
						"Invalid source field number %d",source_field_numbers[i]);
					return_code=0;
				}
			}
			if (source_field_number < number_of_source_fields)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_composite.  Not all source fields used");
				return_code=0;
			}
			if (source_value_number < number_of_source_values)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_composite.  Not all source values used");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* 1. make dynamic allocations for any new type-specific data */
			temp_source_fields = (struct Computed_field **)NULL;
			temp_source_values = (FE_value *)NULL;
			if (((0 == number_of_source_fields) || ALLOCATE(temp_source_fields,
				struct Computed_field *,number_of_source_fields)) &&
				((0 == number_of_source_values) || ALLOCATE(temp_source_values,
					FE_value,number_of_source_values)))
			{
				field = CREATE(Computed_field)("");
				/* 3. establish the new type */
				field->number_of_components = number_of_components;
				for (i=0;i<number_of_source_fields;i++)
				{
					temp_source_fields[i] = ACCESS(Computed_field)(source_fields[i]);
				}
				field->source_fields = temp_source_fields;
				field->number_of_source_fields = number_of_source_fields;			
				for (i=0;i<number_of_source_values;i++)
				{
					temp_source_values[i] = source_values[i];
				}
				field->source_values = temp_source_values;
				field->number_of_source_values = number_of_source_values;			

				field->core = new Computed_field_composite(field,
					source_field_numbers, source_value_numbers);

				ACCESS(Computed_field)(field);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_composite.  Not enough memory");
				DEALLOCATE(temp_source_fields);
				DEALLOCATE(temp_source_values);
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_composite.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* Computed_field_set_type_composite */

int Computed_field_get_type_composite(struct Computed_field *field,
	int *number_of_components,
	int *number_of_source_fields,struct Computed_field ***source_fields,
	int *number_of_source_values,FE_value **source_values,
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
		*source_values = (FE_value *)NULL;
		*source_field_numbers = (int *)NULL;
		*source_value_numbers = (int *)NULL;
		if (((0 == field->number_of_source_fields) || ALLOCATE(*source_fields,
			struct Computed_field *, field->number_of_source_fields)) &&
			((0 == field->number_of_source_values) || ALLOCATE(*source_values,
				FE_value, field->number_of_source_values)) &&
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
	FE_value *source_values;
	int *source_field_numbers;
	int *source_value_numbers;
}; /* struct Computed_field_composite_source_data */

int set_Computed_field_composite_source_data(struct Parse_state *state,
	void *source_data_void,void *computed_field_manager_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Note that fields are not ACCESSed by this function and should not be
ACCESSed in the initial source_data.
==============================================================================*/
{
	char *current_token, *field_component_name, *source_string, *temp_name;
	FE_value *temp_source_values, value;
	int component_no, components_to_add, i, number_of_characters, return_code,
		source_field_number, source_value_number, *temp_source_field_numbers,
		*temp_source_value_numbers;
	struct Computed_field *field, **temp_source_fields;
	struct Computed_field_composite_source_data *source_data;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(set_Computed_field_composite_source_data);
	if (state && (source_data =
		(struct Computed_field_composite_source_data *)source_data_void) &&
		(computed_field_manager=
			(struct MANAGER(Computed_field) *)computed_field_manager_void))
	{
		return_code=1;
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				/* clear source_data */
				source_data->number_of_components = 0;
				source_data->number_of_source_fields = 0;
				if (source_data->source_fields)
				{
					DEALLOCATE(source_data->source_fields);
					source_data->source_fields = (struct Computed_field **)NULL;
				}
				source_data->number_of_source_values = 0;
				if (source_data->source_values)
				{
					DEALLOCATE(source_data->source_values);
					source_data->source_values = (FE_value *)NULL;
				}
				if (source_data->source_field_numbers)
				{
					DEALLOCATE(source_data->source_field_numbers);
					source_data->source_field_numbers = (int *)NULL;
				}
				if (source_data->source_value_numbers)
				{
					DEALLOCATE(source_data->source_value_numbers);
					source_data->source_value_numbers = (int *)NULL;
				}
				while (return_code && (current_token = state->current_token))
				{
					/* first try to find a number in the token */
					if (strchr("+-0123456789",current_token[0]))
					{
						if ((1 == sscanf(current_token, FE_VALUE_INPUT_STRING "%n",
							&value, &number_of_characters)) &&
							((int)strlen(current_token) == number_of_characters))
						{
							shift_Parse_state(state,1);
							if (REALLOCATE(temp_source_values, source_data->source_values,
								FE_value, source_data->number_of_source_values+1))
							{
								source_data->source_values = temp_source_values;
								temp_source_values[source_data->number_of_source_values] =
									value;
								components_to_add = 1;
								source_field_number = -1;
								source_value_number = source_data->number_of_source_values;
								(source_data->number_of_source_values)++;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_Computed_field_composite_source_data.  "
									"Not enough memory");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Invalid characters in composite value: %s",current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						if (field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
							current_token,computed_field_manager))
						{
							components_to_add = field->number_of_components;
							/* following is the first source_value_number to add */
							source_value_number = 0;
							shift_Parse_state(state,1);
						}
						else if (field_component_name=strchr(current_token,'.'))
						{
							*field_component_name='\0';
							field_component_name++;
							if (field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
								current_token,computed_field_manager))
							{
								component_no = -1;
								for (i=0;(0>component_no)&&(i<field->number_of_components)&&
									return_code;i++)
								{
									if (temp_name = Computed_field_get_component_name(field,i))
									{
										if (0 == strcmp(field_component_name,temp_name))
										{
											component_no = i;
										}
										DEALLOCATE(temp_name);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"set_Computed_field_composite_source_data.  "
											"Could not get component name");
										return_code=0;
									}
								}
								if (return_code)
								{
									if (0 <= component_no)
									{
										components_to_add = 1;
										source_value_number = component_no;
										shift_Parse_state(state,1);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"Unknown field component %s.%s",current_token,
											field_component_name);
										display_parse_state_location(state);
										return_code=0;
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"Unknown field: %s",
									current_token);
								display_parse_state_location(state);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown field: %s",current_token);
							display_parse_state_location(state);
							return_code=0;
						}
						if (return_code)
						{
							/* try to find field in current source_fields */
							source_field_number = -1;
							for (i=0;(i<source_data->number_of_source_fields)&&
								(0>source_field_number);i++)
							{
								if (field == source_data->source_fields[i])
								{
									source_field_number = i;
								}
							}
							if (-1 == source_field_number)
							{
								if (REALLOCATE(temp_source_fields,
									source_data->source_fields,struct Computed_field *,
									source_data->number_of_source_fields+1))
								{
									source_data->source_fields = temp_source_fields;
									temp_source_fields[source_data->number_of_source_fields] =
										field;
									source_field_number = source_data->number_of_source_fields;
									(source_data->number_of_source_fields)++;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"set_Computed_field_composite_source_data.  "
										"Not enough memory");
									return_code=0;
								}
							}
						}
					}
					if (return_code)
					{
						if (REALLOCATE(temp_source_field_numbers,
							source_data->source_field_numbers,int,
							source_data->number_of_components+components_to_add))
						{
							source_data->source_field_numbers = temp_source_field_numbers;
							if (REALLOCATE(temp_source_value_numbers,
								source_data->source_value_numbers,int,
								source_data->number_of_components+components_to_add))
							{
								source_data->source_value_numbers = temp_source_value_numbers;
								for (i=0;i<components_to_add;i++)
								{
									source_data->source_field_numbers
										[source_data->number_of_components] = source_field_number;
									source_data->source_value_numbers
										[source_data->number_of_components] =
										source_value_number + i;
									(source_data->number_of_components)++;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_Computed_field_composite_source_data.  "
									"Not enough memory");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_Computed_field_composite_source_data.  Not enough memory");
							return_code=0;
						}
					}
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #|FIELD[.COMPONENT_NAME] "
					"#|FIELD[.COMPONENT_NAME] #|FIELD[.COMPONENT_NAME] ... ");
				if (0<source_data->number_of_components)
				{
					Computed_field_composite* composite_core;
					
					if ((field = Computed_field_create_composite(
						source_data->number_of_components,
						source_data->number_of_source_fields,source_data->source_fields,
						source_data->number_of_source_values,source_data->source_values,
						source_data->source_field_numbers,
						source_data->source_value_numbers))&&
						(composite_core = dynamic_cast<Computed_field_composite*>(field->core)) &&
						(source_string=composite_core->get_source_string(/*commands*/1)))
					{
						display_message(INFORMATION_MESSAGE,"[%s]",source_string);
						DEALLOCATE(source_string);

						DESTROY(Computed_field)(&field);
					}
				}
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"Missing composition fields/components and values");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Computed_field_composite_source_data.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Computed_field_composite_source_data */

int define_Computed_field_type_composite(struct Parse_state *state,
	void *field_modify_void,void *computed_field_composite_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_COMPOSITE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field;
	Computed_field_composite_package *computed_field_composite_package;
	Computed_field_modify_data *field_modify;
	struct Computed_field_composite_source_data source_data;
	struct Option_table *option_table;

	ENTER(define_Computed_field_type_composite);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void) &&
			(field=field_modify->field)&&
		(computed_field_composite_package=
			(Computed_field_composite_package *)
			computed_field_composite_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_data.number_of_components = 0;
		source_data.number_of_source_fields = 0;
		source_data.source_fields = (struct Computed_field **)NULL;
		source_data.number_of_source_values = 0;
		source_data.source_values = (FE_value *)NULL;
		source_data.source_field_numbers = (int *)NULL;
		source_data.source_value_numbers = (int *)NULL;
		if (computed_field_composite_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_composite(field,
				&(source_data.number_of_components),
				&(source_data.number_of_source_fields), &(source_data.source_fields),
				&(source_data.number_of_source_values), &(source_data.source_values),
				&(source_data.source_field_numbers),
				&(source_data.source_value_numbers));
		}
		if (return_code)
		{
			/*???RC begin temporary code */
			/*???RC swallow up old "number_of_scalars # scalars" tokens used by old
				composite field command for backward compatibility */
			if (((state->current_index+2) < state->number_of_tokens) &&
				(fuzzy_string_compare(state->tokens[state->current_index],
					"number_of_scalars")) &&
				(0<atoi(state->tokens[state->current_index+1])) &&
				(fuzzy_string_compare(state->tokens[state->current_index+2],
					"scalars")))
			{
				shift_Parse_state(state,3);
			}
			/*???RC end temporary code */

			option_table = CREATE(Option_table)();
			/* default option: composite field definition */
			Option_table_add_entry(option_table, (char *)NULL, &source_data,
				Cmiss_region_get_Computed_field_manager(field_modify->region),
				set_Computed_field_composite_source_data);
			return_code = Option_table_multi_parse(option_table,state) &&
				Computed_field_copy_type_specific_and_deaccess(field, 
				Computed_field_create_composite(
				source_data.number_of_components,
				source_data.number_of_source_fields, source_data.source_fields,
				source_data.number_of_source_values, source_data.source_values,
				source_data.source_field_numbers,
				source_data.source_value_numbers));
			DESTROY(Option_table)(&option_table);
		}
		/* clean up the source data */
		if (source_data.source_fields)
		{
			DEALLOCATE(source_data.source_fields);
		}
		if (source_data.source_values)
		{
			DEALLOCATE(source_data.source_values);
		}
		if (source_data.source_field_numbers)
		{
			DEALLOCATE(source_data.source_field_numbers);
		}
		if (source_data.source_value_numbers)
		{
			DEALLOCATE(source_data.source_value_numbers);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_composite.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_composite */

struct Computed_field *Computed_field_create_constant(
	int number_of_values, double *values)
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
	FE_value *fe_value_values;
	int i, *source_field_numbers, *source_value_numbers;
	Computed_field *field;

	ENTER(Computed_field_create_constant);
	if ((0 < number_of_values) && values)
	{
		ALLOCATE(source_field_numbers, int, number_of_values);
		ALLOCATE(source_value_numbers, int, number_of_values);
		ALLOCATE(fe_value_values, FE_value, number_of_values);
		if (source_field_numbers && source_value_numbers)
		{
			for (i = 0; i < number_of_values; i++)
			{
				source_field_numbers[i] = -1;
				source_value_numbers[i] = i;
				fe_value_values[i] = values[i];
			}
			field = Computed_field_create_composite(
				/*number_of_components*/number_of_values,
				/*number_of_source_fields*/0,
				/*source_fields*/(struct Computed_field **)NULL,
				/*number_of_source_values*/number_of_values,
				/*source_values*/fe_value_values,
				source_field_numbers, source_value_numbers);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_create_constant.  Not enough memory");
			field = (Computed_field *)NULL;
		}
		DEALLOCATE(source_field_numbers);
		DEALLOCATE(source_value_numbers);
		DEALLOCATE(fe_value_values);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_constant.  Invalid argument(s)");
		field = (Computed_field *)NULL;
	}
	LEAVE;

	return (field);
} /* Computed_field_create_constant */

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

int define_Computed_field_type_constant(struct Parse_state *state,
	void *field_modify_void, void *computed_field_composite_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CONSTANT (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	double *values, *temp_values;
	int i, number_of_values, previous_number_of_values, return_code;
	struct Computed_field *field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;

	ENTER(define_Computed_field_type_constant);
	USE_PARAMETER(computed_field_composite_package_void);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void) &&
			(field=field_modify->field))
	{
		return_code = 1;
		previous_number_of_values = 0;
		values = (double *)NULL;
		if (Computed_field_is_constant(field))
		{
			previous_number_of_values = field->number_of_source_values;
			if (ALLOCATE(values, double, previous_number_of_values))
			{
				for (i = 0; i < previous_number_of_values; i++)
				{
					values[i] = field->source_values[i];
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_constant.  Could not allocate values");
				return_code = 0;
			}
		}
		if (return_code)
		{
			/*???RC begin temporary code */
			/*???RC swallow up old "number_of_values # values" tokens used by old
				constant field command for backward compatibility */
			if (((state->current_index + 2) < state->number_of_tokens) &&
				(fuzzy_string_compare(state->tokens[state->current_index],
					"number_of_values")) &&
				(0<atoi(state->tokens[state->current_index+1])) &&
				(fuzzy_string_compare(state->tokens[state->current_index+2],
					"values")))
			{
				shift_Parse_state(state,3);
			}
			/*???RC swallow up old "value N" tokens used by old constant field if
				default number_of_values=1 used ... for backward compatibility */
			else if (((state->current_index + 2) == state->number_of_tokens) &&
				fuzzy_string_compare(state->tokens[state->current_index], "values"))
			{
				shift_Parse_state(state, 1);
			}
			/*???RC end temporary code */

			if ((current_token = state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING, current_token) &&
					strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))))
			{
				number_of_values = previous_number_of_values;
			}
			else if (0 < (number_of_values =
				state->number_of_tokens - state->current_index))
			{
				if (REALLOCATE(temp_values, values, double, number_of_values))
				{
					values = temp_values;
					/* clear new values to 0.0 */
					for (i = previous_number_of_values; i < number_of_values; i++)
					{
						values[i] = 0.0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_constant.  "
						"Could not reallocate values");
					return_code = 0;
				}
			}
			else
			{
				number_of_values = 0;
			}
			if (return_code)
			{
				option_table = CREATE(Option_table)();
				/* default option: composite field definition */
				Option_table_add_help(option_table,
		 		  "A constant field may be defined as having one or more components.  Each of the <values> listed is used to asign a constant value to the corresponding field component. Fields with more than 1 component can be used to represent vectors or matrices.  An m by n matrix requires (m*n) components and the components of the matrix are listed row by row.");
				Option_table_add_entry(option_table, (char *)NULL, values,
					&number_of_values, set_double_vector);
				if (return_code = Option_table_multi_parse(option_table, state))
				{
					return_code = Computed_field_copy_type_specific_and_deaccess(field,
						Computed_field_create_constant(
						number_of_values, values));
				}
				DESTROY(Option_table)(&option_table);
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING,state->current_token) &&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_constant.  Failed");
				}
			}
		}
		if (values)
		{
			DEALLOCATE(values);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_constant.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_constant */

int Computed_field_set_type_component(struct Computed_field *field,
	struct Computed_field *source_field, int component_number)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Changes <field> into type composite returning the single field component.
Since a component field performs a subset of the capabilities of the composite
field type but the latter is somewhat complicated to set up, this is a
convenience function for building a composite field for a scalar component.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int return_code, source_field_number;

	ENTER(Computed_field_set_type_component);
	if (field && source_field && (0 <= component_number) &&
		(component_number < source_field->number_of_components))
	{
		source_field_number = 0;
		return_code = Computed_field_copy_type_specific_and_deaccess(field,
			Computed_field_create_composite(/*number_of_components*/1,
			/*number_of_source_fields*/1, /*source_fields*/&source_field,
			/*number_of_source_values*/0, /*source_values*/(FE_value *)NULL,
			&source_field_number, &component_number));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_component.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_component */

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
		if (!(component_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
			Computed_field_is_component_wrapper, &field_component,
			computed_field_manager)))
		{
			if (component_name =
				Computed_field_get_component_name(field, component_number))
			{
				if (ALLOCATE(component_field_name, char,
					strlen(field->name) + strlen(component_name) + 2))
				{
					sprintf(component_field_name, "%s.%s", field->name, component_name);
					if (component_field = CREATE(Computed_field)(component_field_name))
					{
						if (!(Computed_field_set_type_component(component_field, field,
							component_number) &&
							ADD_OBJECT_TO_MANAGER(Computed_field)(component_field,
								computed_field_manager)))
						{
							DESTROY(Computed_field)(&component_field);
						}
					}
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

Computed_field *Cmiss_field_create_identity(Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Changes <field> into type composite with one input field, the <source_field>.
==============================================================================*/
{
	int i, number_of_values, *source_field_numbers,
		*source_value_numbers;
	Computed_field *field;

	ENTER(Computed_field_create_identity);
	if (source_field)
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
				Computed_field_create_composite(
				/*number_of_components*/number_of_values,
				/*number_of_source_fields*/1, /*source_fields*/&source_field,
				/*number_of_source_values*/0, /*source_values*/(FE_value *)NULL,
				source_field_numbers, source_value_numbers);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_create_identity.  Not enough memory");
			field = (Computed_field *)NULL;
		}
		DEALLOCATE(source_field_numbers);
		DEALLOCATE(source_value_numbers);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_identity.  Invalid argument(s)");
		field = (Computed_field *)NULL;
	}
	LEAVE;

	return (field);
} /* Computed_field_create_identity */

int Computed_field_register_types_composite(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_composite_package
		*computed_field_composite_package = 
		new Computed_field_composite_package;

	ENTER(Computed_field_register_types_composite);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_composite_type_string,
			define_Computed_field_type_composite,
			computed_field_composite_package);
		/* "constant" = alias for composite included for backward compatibility */
		return_code = Computed_field_package_add_type(computed_field_package,
			"constant", define_Computed_field_type_constant,
			computed_field_composite_package);
		/* "component" = alias for composite included for backward compatibility */
		return_code = Computed_field_package_add_type(computed_field_package,
			"component", define_Computed_field_type_composite,
			computed_field_composite_package);
		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_composite.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_composite */
