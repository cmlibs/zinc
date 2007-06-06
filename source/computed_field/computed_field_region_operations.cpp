/*******************************************************************************
FILE : computed_field_region_operations.cpp

LAST MODIFIED : 24 August 2006

DESCRIPTION :
Implements a computed_field that uses evaluates one field, does a
"find element_xi" look up on a field in a host element group to find the same 
values and then evaluates a third field at that location.
Essentially it is used to embed one mesh in the elements of another.
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
#include "region/cmiss_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_region_operations.h"
#include "finite_element/finite_element_region.h"
}
#include <iostream>
using namespace std;

class Computed_field_region_operations_package : public Computed_field_type_package
{
public:
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Cmiss_region *root_region;
};

namespace {

char computed_field_region_sum_type_string[] = "region_sum";

class Computed_field_region_sum : public Computed_field_core
{
//These parameters are protected to this file using a NULL namespace
public:
	char *region_path;
	struct Cmiss_region *region;
	FE_region *fe_region;
	FE_value current_time;

	Computed_field_region_sum(Computed_field* field, char *region_path,
		Cmiss_region* region) : 
		Computed_field_core(field), region_path(duplicate_string(region_path)),
		region(ACCESS(Cmiss_region)(region)), fe_region(Cmiss_region_get_FE_region(region))
	{		
		current_time = 0.0;
	};

	~Computed_field_region_sum();

private:
	Computed_field_core *copy(Computed_field* new_parent);

	char *get_type_string()
	{
		return(computed_field_region_sum_type_string);
	}

	int compare(Computed_field_core* other_field);

	int is_defined_at_location(Field_location* location);

	static int accumulate_nodal_values(struct FE_node *node,
		void *computed_field_region_sum_void);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

Computed_field_region_sum::~Computed_field_region_sum()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	ENTER(Computed_field_region_sum::~Computed_field_region_sum);
	if (field)
	{
		if (region)
		{
			DEACCESS(Cmiss_region)(&region);
		}
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_region_sum::~Computed_field_region_sum.  Invalid argument(s)");
	}
	LEAVE;
} /* Computed_field_region_sum::~Computed_field_region_sum */

Computed_field_core *Computed_field_region_sum::copy(Computed_field *new_parent)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_region_sum* core;
	ENTER(Computed_field_region_sum::copy_type_specific);
	if (new_parent)
	{
		core = new Computed_field_region_sum(new_parent,
			region_path, region);
	}
	else
	{
		core = (Computed_field_region_sum*)NULL;
	}
	LEAVE;

	return (core);
} /* Computed_field_region_sum::copy */

int Computed_field_region_sum::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Compare the type specific data.
==============================================================================*/
{
	int return_code;
	struct Computed_field_region_sum *other;

	ENTER(Computed_field_region_sum::type_specific_contents_match);
	if (field && (other = dynamic_cast<Computed_field_region_sum*>(other_core)))
	{
		if ((region == other->region) &&
			(!strcmp(region_path, other->region_path)))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_region_sum::compare */

int Computed_field_region_sum::is_defined_at_location(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Checks if <field->source_fields> exists at the first node in <fe_region>
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_region_sum::is_defined_at_location);
	if (field && location)
	{
		return_code=1;
		if (FE_region_get_first_FE_node_that(fe_region,
			 FE_node_has_Computed_field_defined, field->source_fields[0]))
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_region_sum::is_defined_at_location */

int Computed_field_region_sum::accumulate_nodal_values(struct FE_node *node,
	void *computed_field_region_sum_void)
/*******************************************************************************
LAST MODIFIED : 1 May 2007

DESCRIPTION :
For the current node, the value of the source field is added to the field
storing the sums. Each components are treated separately.
==============================================================================*/
{
	int i, return_code;
	Computed_field_region_sum *region_sum;

	ENTER(Computed_field_region_sum::accumulate_nodal_values);
	if (node && (region_sum = 
		static_cast<Computed_field_region_sum*>(computed_field_region_sum_void)))
	{
		Field_node_location nodal_location(node, region_sum->current_time);
	
		if (Computed_field_evaluate_cache_at_location(
			region_sum->field->source_fields[0], &nodal_location))
		{
			for (i = 0 ; i < region_sum->field->number_of_components ; i++)
			{
				region_sum->field->values[i] +=
					region_sum->field->source_fields[0]->values[i];			
			}
			region_sum->field->derivatives_valid = 0;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_region_sum::accumulate_nodal_values.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_region_sum::accumulate_nodal_values */

int Computed_field_region_sum::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_region_sum::evaluate_cache_at_location);
	if (field && location)
	{
		
		// initialise values
		for (i = 0 ; i < field->number_of_components ; i++)
		{
	  	field->values[i] = 0;
		}
		current_time = location->get_time();
		
		// sum field values over nodes in the region
		FE_region_for_each_FE_node(fe_region,
		   Computed_field_region_sum::accumulate_nodal_values, this);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_region_sum::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_region_sum::evaluate_cache_at_location */


int Computed_field_region_sum::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Lists a description of the region_sum inputs
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_region_sum);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field being summed :");
		display_message(INFORMATION_MESSAGE," %s\n", field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    region being summed :");
		display_message(INFORMATION_MESSAGE," %s\n", region_path);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_region_sum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_region_sum */

char *Computed_field_region_sum::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *group_name, *field_name;
	int error;

	ENTER(Computed_field_region_sum::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_region_sum_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " region ", &error);
		if (group_name = duplicate_string(region_path))
		{
			make_valid_token(&group_name);
			append_string(&command_string, group_name, &error);
			DEALLOCATE(group_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_region_sum::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_region_sum::get_command_string */

} //namespace

int Computed_field_set_type_region_sum(struct Computed_field *field,
	struct Computed_field *operate_field,
	struct Cmiss_region *operate_region, char *region_path)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_REGION_SUM, this field calculates the
sum of a field over all nodes in a region. If the field has components, the
sum of each component is calculated.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_region_sum);
	if (field&&operate_field&&operate_region&&region_path)
	{
		return_code = 1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields = 1;
		if (ALLOCATE(source_fields, struct Computed_field *,
			number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components=operate_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(operate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;

			field->core = new Computed_field_region_sum(field,
				region_path, operate_region);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_region_sum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_region_sum */

int Computed_field_get_type_region_sum(struct Computed_field *field,
	struct Computed_field **operate_field,
	struct Cmiss_region **operate_region, char **region_path)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_REGION_SUM, the function returns the field
and region to operate on.
==============================================================================*/
{
	int return_code;
	Computed_field_region_sum* region_sum_core;

	ENTER(Computed_field_get_type_region_sum);
	if (field && (region_sum_core = dynamic_cast<Computed_field_region_sum*>(field->core)) &&
		operate_field && operate_region)
	{
		*operate_field = field->source_fields[0];
		*operate_region = region_sum_core->region;
		*region_path = region_sum_core->region_path;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_region_sum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_region_sum */

int define_Computed_field_type_region_sum(struct Parse_state *state,
	void *field_void, void *computed_field_region_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_OPERATION_SUM (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *operate_region_path;
	int return_code;
	struct Computed_field *field, *operate_field;
	Computed_field_region_operations_package *computed_field_region_operations_package;
	struct Coordinate_system *coordinate_system_ptr;
	struct Cmiss_region *operate_region;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_operate_field_data;

	ENTER(define_Computed_field_type_region_sum);
	if (state && (field = (struct Computed_field *)field_void) &&
		(computed_field_region_operations_package =
			(Computed_field_region_operations_package *)
			computed_field_region_operations_package_void))
	{
		return_code = 1;
		operate_region = (struct Cmiss_region *)NULL;
		operate_region_path = (char *)NULL;
		operate_field = (struct Computed_field *)NULL;
		/* get valid parameters for composite field */
		if (computed_field_region_sum_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_region_sum(field, 
				&operate_field, &operate_region, &operate_region_path);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			
			if (operate_field)
			{
				ACCESS(Computed_field)(operate_field);
			}

			option_table = CREATE(Option_table)();
			/* group */
			Option_table_add_set_Cmiss_region_path(option_table, "region", 
				 computed_field_region_operations_package->root_region, &operate_region_path);
			/* texture_coordinates_field */
			set_operate_field_data.computed_field_manager =
				computed_field_region_operations_package->computed_field_manager;
			set_operate_field_data.conditional_function = 
				Computed_field_has_numerical_components;
			set_operate_field_data.conditional_function_user_data = 
				(void *)NULL;
			Option_table_add_entry(option_table, "field", 
				&operate_field, &set_operate_field_data, 
				set_Computed_field_conditional);
			
			return_code = Option_table_multi_parse(option_table, state);
			
			/* no errors,not asking for help */
			if (return_code)
			{
				if (operate_region_path)
				{
					if (!(Cmiss_region_get_region_from_path(
						computed_field_region_operations_package->root_region, 
						operate_region_path, &operate_region)))
					{
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_region_sum.  Unable to find region %s",
							operate_region_path);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "You must specify a region path (group)");
				}
			}
			if (return_code)
			{
				if (return_code=Computed_field_set_type_region_sum(field,
					operate_field, operate_region, operate_region_path))
				{
					/* Set default coordinate system */
					/* Inherit from third source field */
					coordinate_system_ptr = 
						Computed_field_get_coordinate_system(operate_field);
					Computed_field_set_coordinate_system(field, coordinate_system_ptr);
				}
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_region_sum.  Failed");
				}
			}
			if (operate_region_path)
			{
				DEALLOCATE(operate_region_path);
			}
			if (operate_field)
			{
				DEACCESS(Computed_field)(&operate_field);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_region_sum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_region_sum */




namespace {

char computed_field_region_mean_type_string[] = "region_mean";

class Computed_field_region_mean : public Computed_field_core
{
//These parameters are protected to this file using a NULL namespace
public:
	char *region_path;
	struct Cmiss_region *region;
	FE_region *fe_region;
	FE_value current_time;
	int number_of_values;

	Computed_field_region_mean(Computed_field* field, char *region_path,
		Cmiss_region* region) : 
		Computed_field_core(field), region_path(duplicate_string(region_path)),
		region(ACCESS(Cmiss_region)(region)), fe_region(Cmiss_region_get_FE_region(region))
	{		
		current_time = 0.0;
		number_of_values = 0;
	};

	~Computed_field_region_mean();

private:
	Computed_field_core *copy(Computed_field* new_parent);

	char *get_type_string()
	{
		return(computed_field_region_mean_type_string);
	}

	int compare(Computed_field_core* other_field);

	int is_defined_at_location(Field_location* location);

	static int accumulate_nodal_values(struct FE_node *node,
		void *computed_field_region_mean_void);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

Computed_field_region_mean::~Computed_field_region_mean()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	ENTER(Computed_field_region_mean::~Computed_field_region_mean);
	if (field)
	{
		if (region)
		{
			DEACCESS(Cmiss_region)(&region);
		}
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_region_mean::~Computed_field_region_mean.  Invalid argument(s)");
	}
	LEAVE;
} /* Computed_field_region_mean::~Computed_field_region_mean */

Computed_field_core *Computed_field_region_mean::copy(Computed_field *new_parent)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_region_mean* core;
	ENTER(Computed_field_region_mean::copy_type_specific);
	if (new_parent)
	{
		core = new Computed_field_region_mean(new_parent,
			region_path, region);
	}
	else
	{
		core = (Computed_field_region_mean*)NULL;
	}
	LEAVE;

	return (core);
} /* Computed_field_region_mean::copy */

int Computed_field_region_mean::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Compare the type specific data.
==============================================================================*/
{
	int return_code;
	struct Computed_field_region_mean *other;

	ENTER(Computed_field_region_mean::type_specific_contents_match);
	if (field && (other = dynamic_cast<Computed_field_region_mean*>(other_core)))
	{
		if ((region == other->region) &&
			(!strcmp(region_path, other->region_path)))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_region_mean::compare */

int Computed_field_region_mean::is_defined_at_location(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Checks if <field->source_fields> exists at the first node in <fe_region>
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_region_mean::is_defined_at_location);
	if (field && location)
	{
		return_code=1;
		if (FE_region_get_first_FE_node_that(fe_region,
			 FE_node_has_Computed_field_defined, field->source_fields[0]))
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_region_mean::is_defined_at_location */

int Computed_field_region_mean::accumulate_nodal_values(struct FE_node *node,
	void *computed_field_region_mean_void)
/*******************************************************************************
LAST MODIFIED : 1 May 2007

DESCRIPTION :
For the current node, the value of the source field is added to the field
storing the sums. Each components are treated separately. Als the number of
nodes summed is recorded.
==============================================================================*/
{
	int i, return_code;
	Computed_field_region_mean *region_mean;

	ENTER(Computed_field_region_mean::accumulate_nodal_values);
	if (node && (region_mean = 
		static_cast<Computed_field_region_mean*>(computed_field_region_mean_void)))
	{
		Field_node_location nodal_location(node, region_mean->current_time);
	
		if (Computed_field_evaluate_cache_at_location(
			region_mean->field->source_fields[0], &nodal_location))
		{
			for (i = 0 ; i < region_mean->field->number_of_components ; i++)
			{
				region_mean->field->values[i] +=
					region_mean->field->source_fields[0]->values[i];			
			}
			region_mean->number_of_values ++;
			region_mean->field->derivatives_valid = 0;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_region_mean::accumulate_nodal_values.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_region_mean::accumulate_nodal_values */

int Computed_field_region_mean::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_region_mean::evaluate_cache_at_location);
	if (field && location)
	{
		// initialise values
		number_of_values = 0;
		for (i = 0 ; i < field->number_of_components ; i++)
		{
	  	field->values[i] = 0;
		}
		current_time = location->get_time();
		
		// Sum the field values over a region
		FE_region_for_each_FE_node(fe_region,
		   Computed_field_region_mean::accumulate_nodal_values, this);
		
		// Divide the total sums for each component by the total number of nodes
		// to get the mean for the region
		for (i = 0 ; i < field->number_of_components ; i++)
		{
	  	field->values[i] = field->values[i]/number_of_values;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_region_mean::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_region_mean::evaluate_cache_at_location */


int Computed_field_region_mean::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Lists a description of the region_mean inputs
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_region_mean);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field being averaged :");
		display_message(INFORMATION_MESSAGE," %s\n", field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    region being averaged :");
		display_message(INFORMATION_MESSAGE," %s\n", region_path);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_region_mean.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_region_mean */

char *Computed_field_region_mean::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field.
==============================================================================*/
{
	char *command_string, *group_name, *field_name;
	int error;

	ENTER(Computed_field_region_mean::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_region_mean_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " region ", &error);
		if (group_name = duplicate_string(region_path))
		{
			make_valid_token(&group_name);
			append_string(&command_string, group_name, &error);
			DEALLOCATE(group_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_region_mean::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_region_mean::get_command_string */

} //namespace

int Computed_field_set_type_region_mean(struct Computed_field *field,
	struct Computed_field *operate_field,
	struct Cmiss_region *operate_region, char *region_path)
/*******************************************************************************
LAST MODIFIED : 6 June 2007

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_REGION_MEAN this field calculates the
mean of a field over all nodes in a region. If the field has components, the
mean of each component is calculated.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_region_mean);
	if (field&&operate_field&&operate_region&&region_path)
	{
		return_code = 1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields = 1;
		if (ALLOCATE(source_fields, struct Computed_field *,
			number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components=operate_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(operate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;

			field->core = new Computed_field_region_mean(field,
				region_path, operate_region);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_region_mean.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_region_mean */

int Computed_field_get_type_region_mean(struct Computed_field *field,
	struct Computed_field **operate_field,
	struct Cmiss_region **operate_region, char **region_path)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_REGION_SUM, the function returns the field
and region to operate on.
==============================================================================*/
{
	int return_code;
	Computed_field_region_mean* region_mean_core;

	ENTER(Computed_field_get_type_region_mean);
	if (field && (region_mean_core = dynamic_cast<Computed_field_region_mean*>(field->core)) &&
		operate_field && operate_region)
	{
		*operate_field = field->source_fields[0];
		*operate_region = region_mean_core->region;
		*region_path = region_mean_core->region_path;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_region_mean.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_region_mean */

int define_Computed_field_type_region_mean(struct Parse_state *state,
	void *field_void, void *computed_field_region_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_region_mean (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *operate_region_path;
	int return_code;
	struct Computed_field *field, *operate_field;
	Computed_field_region_operations_package *computed_field_region_operations_package;
	struct Coordinate_system *coordinate_system_ptr;
	struct Cmiss_region *operate_region;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_operate_field_data;

	ENTER(define_Computed_field_type_region_mean);
	if (state && (field = (struct Computed_field *)field_void) &&
		(computed_field_region_operations_package =
			(Computed_field_region_operations_package *)
			computed_field_region_operations_package_void))
	{
		return_code = 1;
		operate_region = (struct Cmiss_region *)NULL;
		operate_region_path = (char *)NULL;
		operate_field = (struct Computed_field *)NULL;
		/* get valid parameters for composite field */
		if (computed_field_region_mean_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_region_mean(field, 
				&operate_field, &operate_region, &operate_region_path);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			
			if (operate_field)
			{
				ACCESS(Computed_field)(operate_field);
			}

			option_table = CREATE(Option_table)();
			/* group */
			Option_table_add_set_Cmiss_region_path(option_table, "region", 
				 computed_field_region_operations_package->root_region, &operate_region_path);
			/* texture_coordinates_field */
			set_operate_field_data.computed_field_manager =
				computed_field_region_operations_package->computed_field_manager;
			set_operate_field_data.conditional_function = 
				Computed_field_has_numerical_components;
			set_operate_field_data.conditional_function_user_data = 
				(void *)NULL;
			Option_table_add_entry(option_table, "field", 
				&operate_field, &set_operate_field_data, 
				set_Computed_field_conditional);
			
			return_code = Option_table_multi_parse(option_table, state);
			
			/* no errors,not asking for help */
			if (return_code)
			{
				if (operate_region_path)
				{
					if (!(Cmiss_region_get_region_from_path(
						computed_field_region_operations_package->root_region, 
						operate_region_path, &operate_region)))
					{
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_region_mean.  Unable to find region %s",
							operate_region_path);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "You must specify a region path (group)");
				}
			}
			if (return_code)
			{
				if (return_code=Computed_field_set_type_region_mean(field,
					operate_field, operate_region, operate_region_path))
				{
					/* Set default coordinate system */
					/* Inherit from third source field */
					coordinate_system_ptr = 
						Computed_field_get_coordinate_system(operate_field);
					Computed_field_set_coordinate_system(field, coordinate_system_ptr);
				}
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_region_mean.  Failed");
				}
			}
			if (operate_region_path)
			{
				DEALLOCATE(operate_region_path);
			}
			if (operate_field)
			{
				DEACCESS(Computed_field)(&operate_field);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_region_mean.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_region_mean */




namespace {

char computed_field_region_rms_type_string[] = "region_rms";

class Computed_field_region_rms : public Computed_field_core
{
//These parameters are protected to this file using a NULL namespace
public:
	char *region_path;
	struct Cmiss_region *region;
	FE_region *fe_region;
	FE_value current_time;
	int number_of_values;

	Computed_field_region_rms(Computed_field* field, char *region_path,
		Cmiss_region* region) : 
		Computed_field_core(field), region_path(duplicate_string(region_path)),
		region(ACCESS(Cmiss_region)(region)), fe_region(Cmiss_region_get_FE_region(region))
	{		
		current_time = 0.0;
		number_of_values = 0;
	};

	~Computed_field_region_rms();

private:
	Computed_field_core *copy(Computed_field* new_parent);

	char *get_type_string()
	{
		return(computed_field_region_rms_type_string);
	}

	int compare(Computed_field_core* other_field);

	int is_defined_at_location(Field_location* location);

	static int accumulate_nodal_values(struct FE_node *node,
		void *computed_field_region_rms_void);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

Computed_field_region_rms::~Computed_field_region_rms()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	ENTER(Computed_field_region_rms::~Computed_field_region_rms);
	if (field)
	{
		if (region)
		{
			DEACCESS(Cmiss_region)(&region);
		}
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_region_rms::~Computed_field_region_rms.  Invalid argument(s)");
	}
	LEAVE;
} /* Computed_field_region_rms::~Computed_field_region_rms */

Computed_field_core *Computed_field_region_rms::copy(Computed_field *new_parent)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_region_rms* core;
	ENTER(Computed_field_region_rms::copy_type_specific);
	if (new_parent)
	{
		core = new Computed_field_region_rms(new_parent,
			region_path, region);
	}
	else
	{
		core = (Computed_field_region_rms*)NULL;
	}
	LEAVE;

	return (core);
} /* Computed_field_region_rms::copy */

int Computed_field_region_rms::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Compare the type specific data.
==============================================================================*/
{
	int return_code;
	struct Computed_field_region_rms *other;

	ENTER(Computed_field_region_rms::type_specific_contents_match);
	if (field && (other = dynamic_cast<Computed_field_region_rms*>(other_core)))
	{
		if ((region == other->region) &&
			(!strcmp(region_path, other->region_path)))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_region_rms::compare */

int Computed_field_region_rms::is_defined_at_location(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Checks if <field->source_fields> exists at the first node in <fe_region>
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_region_rms::is_defined_at_location);
	if (field && location)
	{
		return_code=1;
		if (FE_region_get_first_FE_node_that(fe_region,
			 FE_node_has_Computed_field_defined, field->source_fields[0]))
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_region_rms::is_defined_at_location */

int Computed_field_region_rms::accumulate_nodal_values(struct FE_node *node,
	void *computed_field_region_rms_void)
/*******************************************************************************
LAST MODIFIED : 1 May 2007

DESCRIPTION :
For the current node, the value of the source field is added to the field
storing the sums. Each components are treated separately. Als the number of
nodes summed is recorded.
==============================================================================*/
{
	int i, return_code;
	Computed_field_region_rms *region_rms;

	ENTER(Computed_field_region_rms::accumulate_nodal_values);
	if (node && (region_rms = 
		static_cast<Computed_field_region_rms*>(computed_field_region_rms_void)))
	{
		Field_node_location nodal_location(node, region_rms->current_time);
	
		if (Computed_field_evaluate_cache_at_location(
			region_rms->field->source_fields[0], &nodal_location))
		{

			for (i=0; i<region_rms->field->source_fields[0]->number_of_components;
				i++)
			{
				region_rms->field->values[0] +=
					region_rms->field->source_fields[0]->values[i] *
					region_rms->field->source_fields[0]->values[i];			
			}
			region_rms->number_of_values ++;
			region_rms->field->derivatives_valid = 0;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_region_rms::accumulate_nodal_values.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_region_rms::accumulate_nodal_values */

int Computed_field_region_rms::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_region_rms::evaluate_cache_at_location);
	if (field && location)
	{
		// initialise values
		number_of_values = 0;
		field->values[0] = 0;
		current_time = location->get_time();
		
		// Sum the field values over a region
		FE_region_for_each_FE_node(fe_region,
		   Computed_field_region_rms::accumulate_nodal_values, this);
		
		// Calculates the RMS
		field->values[0] = sqrt(field->values[0]/number_of_values);
		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_region_rms::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_region_rms::evaluate_cache_at_location */


int Computed_field_region_rms::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Lists a description of the region_rms inputs
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_region_rms);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field being root-mean-squared :");
		display_message(INFORMATION_MESSAGE," %s\n", field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    region being root-mean-squared :");
		display_message(INFORMATION_MESSAGE," %s\n", region_path);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_region_rms.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_region_rms */

char *Computed_field_region_rms::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field.
==============================================================================*/
{
	char *command_string, *group_name, *field_name;
	int error;

	ENTER(Computed_field_region_rms::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_region_rms_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " region ", &error);
		if (group_name = duplicate_string(region_path))
		{
			make_valid_token(&group_name);
			append_string(&command_string, group_name, &error);
			DEALLOCATE(group_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_region_rms::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_region_rms::get_command_string */

} //namespace

int Computed_field_set_type_region_rms(struct Computed_field *field,
	struct Computed_field *operate_field,
	struct Cmiss_region *operate_region, char *region_path)
/*******************************************************************************
LAST MODIFIED : 6 June 2007

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_region_rms this field calculates the
mean of a field over all nodes in a region. If the field has components, the
components are treated as a vector, i.e., the rms of each component field is NOT
calculated.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_region_rms);
	if (field&&operate_field&&operate_region&&region_path)
	{
		return_code = 1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields = 1;
		if (ALLOCATE(source_fields, struct Computed_field *,
			number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->number_of_components=1;
			source_fields[0]=ACCESS(Computed_field)(operate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			field->source_fields[0]->number_of_components = 
				operate_field->number_of_components;
				
			field->core = new Computed_field_region_rms(field,
				region_path, operate_region);
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_region_rms.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_region_rms */

int Computed_field_get_type_region_rms(struct Computed_field *field,
	struct Computed_field **operate_field,
	struct Cmiss_region **operate_region, char **region_path)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_REGION_SUM, the function returns the field
and region to operate on.
==============================================================================*/
{
	int return_code;
	Computed_field_region_rms* region_rms_core;

	ENTER(Computed_field_get_type_region_rms);
	if (field && (region_rms_core = dynamic_cast<Computed_field_region_rms*>(field->core)) &&
		operate_field && operate_region)
	{
		*operate_field = field->source_fields[0];
		*operate_region = region_rms_core->region;
		*region_path = region_rms_core->region_path;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_region_rms.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_region_rms */

int define_Computed_field_type_region_rms(struct Parse_state *state,
	void *field_void, void *computed_field_region_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_region_rms (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *operate_region_path;
	int return_code;
	struct Computed_field *field, *operate_field;
	Computed_field_region_operations_package *computed_field_region_operations_package;
	struct Coordinate_system *coordinate_system_ptr;
	struct Cmiss_region *operate_region;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_operate_field_data;

	ENTER(define_Computed_field_type_region_rms);
	if (state && (field = (struct Computed_field *)field_void) &&
		(computed_field_region_operations_package =
			(Computed_field_region_operations_package *)
			computed_field_region_operations_package_void))
	{
		return_code = 1;
		operate_region = (struct Cmiss_region *)NULL;
		operate_region_path = (char *)NULL;
		operate_field = (struct Computed_field *)NULL;
		/* get valid parameters for composite field */
		if (computed_field_region_rms_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_region_rms(field, 
				&operate_field, &operate_region, &operate_region_path);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			
			if (operate_field)
			{
				ACCESS(Computed_field)(operate_field);
			}

			option_table = CREATE(Option_table)();
			/* group */
			Option_table_add_set_Cmiss_region_path(option_table, "region", 
				 computed_field_region_operations_package->root_region, &operate_region_path);
			/* texture_coordinates_field */
			set_operate_field_data.computed_field_manager =
				computed_field_region_operations_package->computed_field_manager;
			set_operate_field_data.conditional_function = 
				Computed_field_has_numerical_components;
			set_operate_field_data.conditional_function_user_data = 
				(void *)NULL;
			Option_table_add_entry(option_table, "field", 
				&operate_field, &set_operate_field_data, 
				set_Computed_field_conditional);
			
			return_code = Option_table_multi_parse(option_table, state);
			
			/* no errors,not asking for help */
			if (return_code)
			{
				if (operate_region_path)
				{
					if (!(Cmiss_region_get_region_from_path(
						computed_field_region_operations_package->root_region, 
						operate_region_path, &operate_region)))
					{
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_region_rms.  Unable to find region %s",
							operate_region_path);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "You must specify a region path (group)");
				}
			}
			if (return_code)
			{
				if (return_code=Computed_field_set_type_region_rms(field,
					operate_field, operate_region, operate_region_path))
				{
					/* Set default coordinate system */
					/* Inherit from third source field */
					coordinate_system_ptr = 
						Computed_field_get_coordinate_system(operate_field);
					Computed_field_set_coordinate_system(field, coordinate_system_ptr);
				}
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_region_rms.  Failed");
				}
			}
			if (operate_region_path)
			{
				DEALLOCATE(operate_region_path);
			}
			if (operate_field)
			{
				DEACCESS(Computed_field)(&operate_field);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_region_rms.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_region_rms */




int Computed_field_register_types_region_operations(
	struct Computed_field_package *computed_field_package, 
	struct Cmiss_region *root_region)
/*******************************************************************************
LAST MODIFIED : 01 May 2007

DESCRIPTION :
Registering the region operations.
==============================================================================*/
{
	int return_code;
	Computed_field_region_operations_package
		*computed_field_region_operations_package = 
		new Computed_field_region_operations_package;

	ENTER(Computed_field_register_types_region_sum);
	if (computed_field_package && root_region)
	{
		computed_field_region_operations_package->computed_field_manager =
			Computed_field_package_get_computed_field_manager(
			computed_field_package);
		computed_field_region_operations_package->root_region = root_region;
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_region_sum_type_string, 
			define_Computed_field_type_region_sum,
			computed_field_region_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_region_mean_type_string, 
			define_Computed_field_type_region_mean,
			computed_field_region_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_region_rms_type_string, 
			define_Computed_field_type_region_rms,
			computed_field_region_operations_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_region_sum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_region_sum */
