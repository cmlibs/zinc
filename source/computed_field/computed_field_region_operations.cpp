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
};

namespace {

char computed_field_region_sum_type_string[] = "region_sum";

class Computed_field_region_sum : public Computed_field_core
{
protected:
	Cmiss_region *region;
	int number_of_values;
	FE_value current_time;

public:
	Computed_field_region_sum(Cmiss_region* region) : 
		Computed_field_core(),
		region(ACCESS(Cmiss_region)(region)),
		number_of_values(0)
	{
		current_time = 0.0;
	};

	~Computed_field_region_sum();

	virtual void inherit_source_field_attributes()
	{
		if (field)
		{
			Computed_field_set_coordinate_system_from_sources(field);
		}
	}
	
	Cmiss_region *get_region()
	{
		return region;
	}

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_region_sum_type_string);
	}

	int compare(Computed_field_core* other_field);

	int is_defined_at_location(Field_location* location);

	static int accumulate_nodal_values(struct FE_node *node,
		void *computed_field_region_sum_void);

	int list();

	char* get_command_string();
	
protected:
	int evaluate_cache_at_location(Field_location* location);
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_region_sum::~Computed_field_region_sum.  Invalid argument(s)");
	}
	LEAVE;
} /* Computed_field_region_sum::~Computed_field_region_sum */

Computed_field_core *Computed_field_region_sum::copy()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_region_sum* core = new Computed_field_region_sum(region);

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

	ENTER(Computed_field_region_mean::type_specific_contents_match);
	Computed_field_region_sum *other =
		dynamic_cast<Computed_field_region_sum*>(other_core);
	if (other)
	{
		if (region == other->region)
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
		// GRC this is expensive; better to always return 1
		if (FE_region_get_first_FE_node_that(Cmiss_region_get_FE_region(region),
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
			region_sum->number_of_values++;
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
		number_of_values = 0;
		// initialise values
		for (i = 0 ; i < field->number_of_components ; i++)
		{
	  	field->values[i] = 0;
		}
		current_time = location->get_time();
		
		// sum field values over nodes in the region
		return_code = FE_region_for_each_FE_node(Cmiss_region_get_FE_region(region),
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
	int return_code = 0;

	ENTER(List_Computed_field_region_sum);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field being summed :");
		display_message(INFORMATION_MESSAGE," %s\n", field->source_fields[0]->name);
		if (region != Computed_field_get_region(field))
		{
			char *group_name = Cmiss_region_get_name(region);
			display_message(INFORMATION_MESSAGE,"    group being summed :");
			display_message(INFORMATION_MESSAGE," %s\n", group_name);
			DEALLOCATE(group_name);
		}
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
	char *command_string, *field_name;
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
		if (region != Computed_field_get_region(field))
		{
			char *group_name = Cmiss_region_get_name(region);
			append_string(&command_string, " group ", &error);
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

struct Computed_field *Computed_field_create_region_sum(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field, struct Cmiss_region *group)
{
	Computed_field *field = NULL;
	Cmiss_region *region = Computed_field_get_region(source_field);
	if (group && (group != region))
	{
		Cmiss_region *group_parent = Cmiss_region_get_parent(group);
		if (group_parent != region)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_create_region_sum.  Invalid group");
			region = NULL;
		}
		DEACCESS(Cmiss_region)(&group_parent);
	}
	if (region)
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_region_sum(group));
	}
	return (field);
}

int Computed_field_get_type_region_sum(struct Computed_field *field,
	struct Computed_field **source_field, struct Cmiss_region **group)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_REGION_SUM, the function returns the
source field and group to sum.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_region_sum);
	Computed_field_region_sum* region_sum_core =
		dynamic_cast<Computed_field_region_sum*>(field->core);
	if (field && region_sum_core && source_field && group)
	{
		*source_field = field->source_fields[0];
		if (Computed_field_get_region(field) != region_sum_core->get_region())
		{
			*group = region_sum_core->get_region();
		}
		else
		{
			*group = NULL;
		}
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
}

int define_Computed_field_type_region_sum(struct Parse_state *state,
	void *field_modify_void, void *computed_field_region_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_OPERATION_SUM (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *group_name;
	int return_code;
	struct Computed_field *source_field;
	Computed_field_region_operations_package *computed_field_region_operations_package;
	Computed_field_modify_data *field_modify;
	struct Cmiss_region *group;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_region_sum);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
		(computed_field_region_operations_package =
			(Computed_field_region_operations_package *)
			computed_field_region_operations_package_void))
	{
		return_code = 1;
		group = field_modify->get_region();
		group_name = (char *)NULL;
		source_field = (struct Computed_field *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_region_sum_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code = Computed_field_get_type_region_sum(field_modify->get_field(), 
				&source_field, &group);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			if (group)
			{
				group_name = Cmiss_region_get_name(group);
			}
			
			option_table = CREATE(Option_table)();
			/* group */
			Option_table_add_set_Cmiss_region_path(option_table, "group",
				/*root_region*/field_modify->get_region(), &group_name);
			/* texture_coordinates_field */
			set_source_field_data.computed_field_manager =
				field_modify->get_field_manager();
			set_source_field_data.conditional_function = 
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data = 
				(void *)NULL;
			Option_table_add_entry(option_table, "field", 
				&source_field, &set_source_field_data, 
				set_Computed_field_conditional);
			
			return_code = Option_table_multi_parse(option_table, state);
			
			/* no errors,not asking for help */
			if (return_code)
			{
				if (group_name)
				{
					if (!(Cmiss_region_get_region_from_path_deprecated(
						/*root_region*/field_modify->get_region(), group_name, &group)))
					{
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_region_sum.  Unable to find group %s",
							group_name);
						return_code = 0;
					}
				}
				else
				{
					group = NULL;
				}
			}
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_region_sum(field_modify->get_field_module(),
						source_field, group));
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
			if (group_name)
			{
				DEALLOCATE(group_name);
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
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

class Computed_field_region_mean : public Computed_field_region_sum
{
public:
	Computed_field_region_mean(Cmiss_region* region) :
		Computed_field_region_sum(region)
	{		
	};

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return (computed_field_region_mean_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

Computed_field_core *Computed_field_region_mean::copy()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_region_mean* core = new Computed_field_region_mean(region);

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

	ENTER(Computed_field_region_mean::type_specific_contents_match);
	Computed_field_region_mean *other =
		dynamic_cast<Computed_field_region_mean*>(other_core);
	if (other)
	{
		if (region == other->region)
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

int Computed_field_region_mean::evaluate_cache_at_location(
	Field_location* location)
{
	int return_code =
		Computed_field_region_sum::evaluate_cache_at_location(location);
	if (return_code)
	{
		if (number_of_values > 0)
		{
			for (int i = 0 ; i < field->number_of_components ; i++)
			{
		  	field->values[i] = field->values[i]/number_of_values;
			}
		}
	}
	return (return_code);
}

int Computed_field_region_mean::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Lists a description of the region_mean inputs
==============================================================================*/
{
	int return_code = 0;

	ENTER(List_Computed_field_region_mean);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field being averaged :");
		display_message(INFORMATION_MESSAGE," %s\n", field->source_fields[0]->name);
		if (region != Computed_field_get_region(field))
		{
			char *group_name = Cmiss_region_get_name(region);
			display_message(INFORMATION_MESSAGE,"    group being averaged :");
			display_message(INFORMATION_MESSAGE," %s\n", group_name);
			DEALLOCATE(group_name);
		}
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
	char *command_string, *field_name;
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
		if (region != Computed_field_get_region(field))
		{
			char *group_name = Cmiss_region_get_name(region);
			append_string(&command_string, " group ", &error);
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

struct Computed_field *Computed_field_create_region_mean(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field, struct Cmiss_region *group)
{
	Computed_field *field = NULL;
	Cmiss_region *region = Computed_field_get_region(source_field);
	if (group && (group != region))
	{
		Cmiss_region *group_parent = Cmiss_region_get_parent(group);
		if (group_parent != region)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_create_region_mean.  Invalid group");
			region = NULL;
		}
		DEACCESS(Cmiss_region)(&group_parent);
	}
	if (region)
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_region_mean(group));
	}

	return (field);
}

int Computed_field_get_type_region_mean(struct Computed_field *field,
	struct Computed_field **source_field, struct Cmiss_region **group)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_REGION_MEAN, the function returns the
source field and group to sum.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_region_mean);
	Computed_field_region_mean* region_mean_core =
		dynamic_cast<Computed_field_region_mean*>(field->core);
	if (field && region_mean_core && source_field && group)
	{
		*source_field = field->source_fields[0];
		if (Computed_field_get_region(field) != region_mean_core->get_region())
		{
			*group = region_mean_core->get_region();
		}
		else
		{
			*group = NULL;
		}
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
}

int define_Computed_field_type_region_mean(struct Parse_state *state,
	void *field_modify_void, void *computed_field_region_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_OPERATION_SUM (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *group_name;
	int return_code;
	struct Computed_field *source_field;
	Computed_field_region_operations_package *computed_field_region_operations_package;
	Computed_field_modify_data *field_modify;
	struct Cmiss_region *group;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_region_mean);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
		(computed_field_region_operations_package =
			(Computed_field_region_operations_package *)
			computed_field_region_operations_package_void))
	{
		return_code = 1;
		group = field_modify->get_region();
		group_name = (char *)NULL;
		source_field = (struct Computed_field *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_region_mean_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code = Computed_field_get_type_region_mean(field_modify->get_field(), 
				&source_field, &group);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			if (group)
			{
				group_name = Cmiss_region_get_name(group);
			}
			
			option_table = CREATE(Option_table)();
			/* group */
			Option_table_add_set_Cmiss_region_path(option_table, "group",
				/*root_region*/field_modify->get_region(), &group_name);
			/* texture_coordinates_field */
			set_source_field_data.computed_field_manager =
				field_modify->get_field_manager();
			set_source_field_data.conditional_function = 
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data = 
				(void *)NULL;
			Option_table_add_entry(option_table, "field", 
				&source_field, &set_source_field_data, 
				set_Computed_field_conditional);
			
			return_code = Option_table_multi_parse(option_table, state);
			
			/* no errors,not asking for help */
			if (return_code)
			{
				if (group_name)
				{
					if (!(Cmiss_region_get_region_from_path_deprecated(
						/*root_region*/field_modify->get_region(), group_name, &group)))
					{
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_region_mean.  Unable to find group %s",
							group_name);
						return_code = 0;
					}
				}
				else
				{
					group = NULL;
				}
			}
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_region_mean(field_modify->get_field_module(),
						source_field, group));
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
			if (group_name)
			{
				DEALLOCATE(group_name);
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
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

int Computed_field_register_types_region_operations(
	struct Computed_field_package *computed_field_package)
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

	ENTER(Computed_field_register_types_region_operations);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_region_sum_type_string, 
			define_Computed_field_type_region_sum,
			computed_field_region_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_region_mean_type_string, 
			define_Computed_field_type_region_mean,
			computed_field_region_operations_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_region_operations.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}
