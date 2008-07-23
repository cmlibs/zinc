/*******************************************************************************
FILE : computed_field_compose.c

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
#include "computed_field/computed_field.h"
}
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "computed_field/computed_field_set.h"
#include "region/cmiss_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_compose.h"
}

class Computed_field_compose_package : public Computed_field_type_package
{
public:
	struct Cmiss_region *root_region;
};

namespace {

char computed_field_compose_type_string[] = "compose";

class Computed_field_compose : public Computed_field_core
{
//These parameters are protected to this file using a NULL namespace
public:
	char *region_path;
	int find_nearest;
	struct Cmiss_region *region;
	int element_dimension;
	int use_point_five_when_out_of_bounds;

	Computed_field_compose(Computed_field* field, char *region_path,
		int find_nearest, Cmiss_region* region, int element_dimension = 0,
		int use_point_five_when_out_of_bounds = 0) : 
		Computed_field_core(field), region_path(duplicate_string(region_path)),
		find_nearest(find_nearest), region(ACCESS(Cmiss_region)(region)),
		element_dimension(element_dimension),
		use_point_five_when_out_of_bounds(use_point_five_when_out_of_bounds)
	{		
	};

	~Computed_field_compose();

private:
	Computed_field_core *copy(Computed_field* new_parent);

	char *get_type_string()
	{
		return(computed_field_compose_type_string);
	}

	int compare(Computed_field_core* other_field);

	int is_defined_at_location(Field_location* location);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

Computed_field_compose::~Computed_field_compose()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	ENTER(Computed_field_compose::~Computed_field_compose);
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
			"Computed_field_compose::~Computed_field_compose.  Invalid argument(s)");
	}
	LEAVE;
} /* Computed_field_compose::~Computed_field_compose */

Computed_field_core *Computed_field_compose::copy(Computed_field *new_parent)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_compose* core;
	ENTER(Computed_field_compose::copy_type_specific);
	if (new_parent)
	{
		core = new Computed_field_compose(new_parent,
			region_path, find_nearest, region, element_dimension,
			use_point_five_when_out_of_bounds);
	}
	else
	{
		core = (Computed_field_compose*)NULL;
	}
	LEAVE;

	return (core);
} /* Computed_field_compose::copy */

int Computed_field_compose::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Compare the type specific data.
==============================================================================*/
{
	int return_code;
	struct Computed_field_compose *other;

	ENTER(Computed_field_compose::type_specific_contents_match);
	if (field && (other = dynamic_cast<Computed_field_compose*>(other_core)))
	{
		if ((region == other->region) &&
			(find_nearest == other->find_nearest) &&
			(!strcmp(region_path, other->region_path)) &&
			(element_dimension == other->element_dimension) &&
			(use_point_five_when_out_of_bounds == 
				other->use_point_five_when_out_of_bounds))
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
} /* Computed_field_compose::compare */

int Computed_field_compose::is_defined_at_location(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	FE_value xi[3];
	int return_code;
	struct FE_element *element;

	ENTER(Computed_field_compose::is_defined_at_location);
	if (field && location)
	{
		return_code=1;
		if (return_code = Computed_field_is_defined_at_location(
			field->source_fields[0],location))
		{
			if (return_code=
				Computed_field_evaluate_cache_at_location(
				field->source_fields[0],location))
			{
				if (Computed_field_find_element_xi(
					field->source_fields[1], field->source_fields[0]->values,
					field->source_fields[0]->number_of_components, &element, xi,
					element_dimension, region,
					/*propagate_field*/0, find_nearest) && element)
				{
					/* calculate the third source_field at this new location */
					return_code = Computed_field_is_defined_in_element(
						field->source_fields[2],element);
				}
				else
				{
					return_code = 0;
				}
			}
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_compose::is_defined_at_location */

int Computed_field_compose::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	FE_value compose_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int i, return_code;
	struct FE_element *compose_element;

	ENTER(Computed_field_compose::evaluate_cache_at_location);
	if (field && location)
	{
		/* 1. Precalculate any source fields that this field depends on */
		/* only calculate the first source_field at this location */
		if (return_code=Computed_field_evaluate_cache_at_location(
			field->source_fields[0],location))
		{
			/* 2. Calculate the field */
			/* The values from the first source field are inverted in the
				second source field to get element_xi which is evaluated with
				the third source field */
			if ((return_code = Computed_field_find_element_xi(field->source_fields[1],
				field->source_fields[0]->values,
				field->source_fields[0]->number_of_components,
					  &compose_element, compose_xi, element_dimension,
						region, /*propagate_field*/0, find_nearest))
				&& compose_element)
			{
				/* calculate the third source_field at this new location */
				Field_element_xi_location new_location(compose_element,compose_xi,
					location->get_time());
				return_code=Computed_field_evaluate_cache_at_location(
					field->source_fields[2], &new_location);
				for (i=0;i<field->number_of_components;i++)
				{
					field->values[i]=field->source_fields[2]->values[i];
				}
				field->derivatives_valid = 0;
			}
			else
			{
				if (use_point_five_when_out_of_bounds)
				{
					/* Actually don't fail here, just make the values constant so that
						people can compose outside the valid range */
					return_code = 1;
					for (i=0;i<field->number_of_components;i++)
					{
						field->values[i]=0.5;
					}
					field->derivatives_valid = 0;
				}
				else
				{
					return_code = 0;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_compose::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_compose::evaluate_cache_at_location */


int Computed_field_compose::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_compose);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    texture coordinates field :");
		display_message(INFORMATION_MESSAGE," %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    find element xi field :");
		display_message(INFORMATION_MESSAGE," %s\n",
			field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,"    search element group :");
		display_message(INFORMATION_MESSAGE," %s\n", region_path);
		display_message(INFORMATION_MESSAGE,"    calculate values field :");
		display_message(INFORMATION_MESSAGE," %s\n",
			field->source_fields[2]->name);
		if (find_nearest)
		{
			display_message(INFORMATION_MESSAGE,"    find nearest match\n");
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"    find exact match\n");
		}
		if (use_point_five_when_out_of_bounds)
		{
			display_message(INFORMATION_MESSAGE,"    use point five when out of bounds\n");
		}
		if (element_dimension)
		{
			display_message(INFORMATION_MESSAGE,"    only element dimension %d\n");
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"    element dimension all\n");
		}			
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_compose.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_compose */

char *Computed_field_compose::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *group_name, *field_name, temp_string[40];
	int error;

	ENTER(Computed_field_compose::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_compose_type_string, &error);
		append_string(&command_string, " texture_coordinates_field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " find_element_xi_field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " group ", &error);
		if (group_name = duplicate_string(region_path))
		{
			make_valid_token(&group_name);
			append_string(&command_string, group_name, &error);
			DEALLOCATE(group_name);
		}
		append_string(&command_string, " calculate_values_field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[2], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		if (find_nearest)
		{
			append_string(&command_string, " find_nearest", &error);
		}
		if (use_point_five_when_out_of_bounds)
		{
			append_string(&command_string, " use_point_five_when_out_of_bounds", &error);
		}
		sprintf(temp_string, " element_dimension %d", element_dimension);
		append_string(&command_string, temp_string, &error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_compose::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_compose::get_command_string */

} //namespace

int Computed_field_set_type_compose(struct Computed_field *field,
	struct Computed_field *texture_coordinate_field,
	struct Computed_field *find_element_xi_field,
	struct Computed_field *calculate_values_field,
	struct Cmiss_region *search_region, char *region_path,
	int find_nearest, int use_point_five_when_out_of_bounds,
	int element_dimension)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_COMPOSE, this field allows you to
evaluate one field to find "texture coordinates", use a find_element_xi field
to then calculate a corresponding element/xi and finally calculate values using
this element/xi and a third field.  You can then evaluate values on a "host"
mesh for any points "contained" inside.  The <search_element_group> is the group
from which any returned element_xi will belong.
If <use_point_five_when_out_of_bounds> is true then if the texture_coordinate_field
values cannot be found in the find_element_xi_field, then instead of returning
failure, the values will be set to 0.5 and returned as success.
Only elements that have dimension equals <element_dimension> will be searched.
The <region_path> string is supplied so that the commands listed can correctly
name the string used to select the <search_region>.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_compose);
	if (field&&texture_coordinate_field&&find_element_xi_field&&
		calculate_values_field&&search_region&&region_path)
	{
		return_code = 1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields = 3;
		if (texture_coordinate_field->number_of_components ==
			find_element_xi_field->number_of_components)
		{
			if (Computed_field_is_find_element_xi_capable(
				find_element_xi_field, /*dummy*/NULL))
			{
				if (ALLOCATE(source_fields, struct Computed_field *,
					number_of_source_fields))
				{
					/* 2. free current type-specific data */
					Computed_field_clear_type(field);
					/* 3. establish the new type */
					field->number_of_components=
						calculate_values_field->number_of_components;
					source_fields[0]=ACCESS(Computed_field)(texture_coordinate_field);
					source_fields[1]=ACCESS(Computed_field)(find_element_xi_field);
					source_fields[2]=ACCESS(Computed_field)(calculate_values_field);
					field->source_fields=source_fields;
					field->number_of_source_fields=number_of_source_fields;

					field->core = new Computed_field_compose(field,
						region_path, find_nearest, search_region, element_dimension,
						use_point_five_when_out_of_bounds);
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
					"Computed_field_set_type_compose.  "
					"The type of find_element_xi_field supplied has not "
					"been implemented for find_element_xi calculations.");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_compose.  "
				"The texuture_coordinate_field and find_element_xi_field "
				"must have the same number of components");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_compose.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_compose */

int Computed_field_get_type_compose(struct Computed_field *field,
	struct Computed_field **texture_coordinate_field,
	struct Computed_field **find_element_xi_field,
	struct Computed_field **calculate_values_field,
	struct Cmiss_region **search_region, char **region_path,
	int *find_nearest, int *use_point_five_when_out_of_bounds,
	int *element_dimension)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_COMPOSE, the function returns the three
fields which define the field.
Note that the fields are not ACCESSed and the <region_path> points to the
internally used path.
==============================================================================*/
{
	int return_code;
	Computed_field_compose* compose_core;

	ENTER(Computed_field_get_type_compose);
	if (field && (compose_core = dynamic_cast<Computed_field_compose*>(field->core)) &&
		texture_coordinate_field &&
		find_element_xi_field && calculate_values_field && search_region)
	{
		*texture_coordinate_field = field->source_fields[0];
		*find_element_xi_field = field->source_fields[1];
		*calculate_values_field = field->source_fields[2];
		*search_region = compose_core->region;
		*region_path = compose_core->region_path;
		*find_nearest = compose_core->find_nearest;
		*use_point_five_when_out_of_bounds = compose_core->use_point_five_when_out_of_bounds;
		*element_dimension = compose_core->element_dimension;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_compose.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_compose */

int define_Computed_field_type_compose(struct Parse_state *state,
	void *field_modify_void, void *computed_field_compose_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_COMPOSE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char fail_when_out_of_bounds_flag, find_nearest_flag, find_exact_flag,
		*old_region_path, *search_region_path, use_point_five_when_out_of_bounds_flag;
	int element_dimension, find_nearest, return_code,
		use_point_five_when_out_of_bounds;
	struct Computed_field *field, *calculate_values_field,*find_element_xi_field,
		*texture_coordinates_field;
	Computed_field_compose_package *computed_field_compose_package;
	Computed_field_modify_data *field_modify;
	struct Coordinate_system *coordinate_system_ptr;
	struct Cmiss_region *search_region;
	struct Option_table *find_option_table, *option_table,
		*out_of_bounds_option_table;
	struct Set_Computed_field_conditional_data set_calculate_values_field_data,
		set_find_element_xi_field_data, set_texture_coordinates_field_data;

	ENTER(define_Computed_field_type_compose);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void) &&
			(field=field_modify->field) &&
		(computed_field_compose_package =
			(Computed_field_compose_package *)
			computed_field_compose_package_void))
	{
		return_code = 1;
		search_region = (struct Cmiss_region *)NULL;
		search_region_path = (char *)NULL;
		old_region_path = (char *)NULL;
		calculate_values_field = (struct Computed_field *)NULL;
		find_element_xi_field = (struct Computed_field *)NULL;
		texture_coordinates_field = (struct Computed_field *)NULL;
		find_nearest_flag = 0;
		find_exact_flag = 0;
		use_point_five_when_out_of_bounds_flag = 0;
		fail_when_out_of_bounds_flag = 0;
		use_point_five_when_out_of_bounds = 0;
		element_dimension = 0;
		/* Maintain the existing behaviour as the default */
		find_nearest = 0;
		/* get valid parameters for composite field */
		if (computed_field_compose_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_compose(field, 
				&calculate_values_field, &find_element_xi_field,
				&texture_coordinates_field, &search_region, &old_region_path,
				&find_nearest, &use_point_five_when_out_of_bounds,
				&element_dimension);
		}
		if (old_region_path)
		{
			search_region_path = duplicate_string(old_region_path);
		}
		else
		{
			Cmiss_region_get_root_region_path(&search_region_path);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (calculate_values_field)
			{
				ACCESS(Computed_field)(calculate_values_field);
			}
			if (find_element_xi_field)
			{
				ACCESS(Computed_field)(find_element_xi_field);
			}
			if (texture_coordinates_field)
			{
				ACCESS(Computed_field)(texture_coordinates_field);
			}

			option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"The value of a compose field is found by evaluating the <texture_coordinates_field>, then searching for matching values of the <find_element_xi_field> in the elements of the <group> and then finally evaluating the <calculate_values_field> at this found location.  By restricting the <element_dimension> you can speed up the search and you can specify the outcome if the matching values cannot be found in the element <group> with <use_point_five_when_out_of_bounds> or <fail_when_out_of_bounds>.  See a/resample_texture or a/create_slices where the compose field is used to find the equivalent coordinate in another element to evaluate a texture.");
			/* calculate_values_field */
			set_calculate_values_field_data.computed_field_manager =
				Cmiss_region_get_Computed_field_manager(field_modify->region);
			set_calculate_values_field_data.conditional_function = 
				Computed_field_has_numerical_components;
			set_calculate_values_field_data.conditional_function_user_data = 
				(void *)NULL;
			Option_table_add_entry(option_table, "calculate_values_field", 
				&calculate_values_field, &set_calculate_values_field_data, 
				set_Computed_field_conditional);
			Option_table_add_int_non_negative_entry(option_table,"element_dimension",
				&element_dimension);
			/* find_element_xi_field */
			set_find_element_xi_field_data.computed_field_manager =
				Cmiss_region_get_Computed_field_manager(field_modify->region);
			set_find_element_xi_field_data.conditional_function = 
				Computed_field_has_numerical_components;
			set_find_element_xi_field_data.conditional_function_user_data = 
				(void *)NULL;
			Option_table_add_entry(option_table, "find_element_xi_field", 
				&find_element_xi_field, &set_find_element_xi_field_data, 
				set_Computed_field_conditional);
			find_option_table=CREATE(Option_table)();
 			Option_table_add_char_flag_entry(find_option_table,"find_nearest",
				&find_nearest_flag);
			Option_table_add_char_flag_entry(find_option_table,"find_exact",
				&find_exact_flag);
			Option_table_add_suboption_table(option_table, find_option_table);
			/* group */
			Option_table_add_set_Cmiss_region_path(option_table, "group", 
				 computed_field_compose_package->root_region, &search_region_path);
			/* texture_coordinates_field */
			set_texture_coordinates_field_data.computed_field_manager =
				Cmiss_region_get_Computed_field_manager(field_modify->region);
			set_texture_coordinates_field_data.conditional_function = 
				Computed_field_has_numerical_components;
			set_texture_coordinates_field_data.conditional_function_user_data = 
				(void *)NULL;
			Option_table_add_entry(option_table, "texture_coordinates_field", 
				&texture_coordinates_field, &set_texture_coordinates_field_data, 
				set_Computed_field_conditional);
			out_of_bounds_option_table=CREATE(Option_table)();
			/* use_point_five_when_out_of_bounds */
 			Option_table_add_char_flag_entry(out_of_bounds_option_table,
				"use_point_five_when_out_of_bounds",
				&use_point_five_when_out_of_bounds_flag);
 			Option_table_add_char_flag_entry(out_of_bounds_option_table,
				"fail_when_out_of_bounds",
				&fail_when_out_of_bounds_flag);
			Option_table_add_suboption_table(option_table, out_of_bounds_option_table);
			return_code = Option_table_multi_parse(option_table, state);
			/* no errors,not asking for help */
			if (return_code)
			{
				if (search_region_path)
				{
					if (!(Cmiss_region_get_region_from_path(
						computed_field_compose_package->root_region, 
						search_region_path, &search_region)))
					{
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_compose.  Unable to find region %s",
							search_region_path);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "You must specify a region path (group)");
				}
			}
			if (return_code)
			{
				if (find_nearest_flag && find_exact_flag)
				{
					display_message(ERROR_MESSAGE, 
						"Specify only one of find_nearest and find_exact");
					return_code = 0;
				}
				if (find_nearest_flag)
				{
					find_nearest = 1;
				}
				else if (find_exact_flag)
				{
					find_nearest = 0;
				}
				if (use_point_five_when_out_of_bounds_flag &&
					fail_when_out_of_bounds_flag)
				{
					display_message(ERROR_MESSAGE, 
						"Specify only one of use_point_five_when_out_of_bounds "
						"and fail_when_out_of_bounds");
					return_code = 0;
				}
				if (use_point_five_when_out_of_bounds_flag)
				{
					use_point_five_when_out_of_bounds = 1;
				}
				else if (fail_when_out_of_bounds_flag)
				{
					use_point_five_when_out_of_bounds = 0;
				}
			}
			if (return_code)
			{
				if (return_code=Computed_field_set_type_compose(field,
					texture_coordinates_field, find_element_xi_field,
						calculate_values_field, search_region, search_region_path,
						find_nearest, use_point_five_when_out_of_bounds,
						element_dimension))
				{
					/* Set default coordinate system */
					/* Inherit from third source field */
					coordinate_system_ptr = 
						Computed_field_get_coordinate_system(calculate_values_field);
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
						"define_Computed_field_type_compose.  Failed");
				}
			}
			if (calculate_values_field)
			{
				DEACCESS(Computed_field)(&calculate_values_field);
			}
			if (find_element_xi_field)
			{
				DEACCESS(Computed_field)(&find_element_xi_field);
			}
			if (search_region_path)
			{
				DEALLOCATE(search_region_path);
			}
			if (texture_coordinates_field)
			{
				DEACCESS(Computed_field)(&texture_coordinates_field);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_compose.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_compose */

int Computed_field_register_types_compose(
	struct Computed_field_package *computed_field_package, 
	struct Cmiss_region *root_region)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_compose_package
		*computed_field_compose_package = 
		new Computed_field_compose_package;

	ENTER(Computed_field_register_types_compose);
	if (computed_field_package && root_region)
	{
		computed_field_compose_package->root_region = root_region;
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_compose_type_string, 
			define_Computed_field_type_compose,
			computed_field_compose_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_compose.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_compose */
