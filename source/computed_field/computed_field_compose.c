/*******************************************************************************
FILE : computed_field_compose.c

LAST MODIFIED : 7 January 2003

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
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "region/cmiss_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_compose.h"

struct Computed_field_compose_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Cmiss_region *root_region;
};

struct Computed_field_compose_type_specific_data
{
	char *region_path;
	struct Cmiss_region *region;
};

static char computed_field_compose_type_string[] = "compose";

int Computed_field_is_type_compose(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_compose);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_compose_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_compose.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_type_compose */

static int Computed_field_compose_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_compose_type_specific_data *data;

	ENTER(Computed_field_compose_clear_type_specific);
	if (field && (data = 
		(struct Computed_field_compose_type_specific_data *)
		field->type_specific_data))
	{
		if (data->region)
		{
			DEACCESS(Cmiss_region)(&(data->region));
		}
		if (data->region_path)
		{
			DEALLOCATE(data->region_path);
		}
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_compose_clear_type_specific.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_compose_clear_type_specific */

static void *Computed_field_compose_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_compose_type_specific_data *destination, *source;

	ENTER(Computed_field_compose_copy_type_specific);
	if (source_field && destination_field && (source = 
		(struct Computed_field_compose_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_compose_type_specific_data, 1) &&
			(destination->region_path = duplicate_string(source->region_path)))
		{
			destination->region = ACCESS(Cmiss_region)(source->region);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_compose_copy_type_specific.  "
				"Unable to allocate memory");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_compose_copy_type_specific.  Invalid argument(s)");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_compose_copy_type_specific */

#define Computed_field_compose_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_compose_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Compare the type specific data.
==============================================================================*/
{
	int return_code;
	struct Computed_field_compose_type_specific_data *data, *other_data;

	ENTER(Computed_field_compose_type_specific_contents_match);
	if (field && other_computed_field && (data = 
		(struct Computed_field_compose_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_compose_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		if ((data->region == other_data->region) &&
			(!strcmp(data->region_path, other_data->region_path)))
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
} /* Computed_field_compose_type_specific_contents_match */

#define Computed_field_compose_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

int Computed_field_compose_is_defined_at_node(struct Computed_field *field,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
==============================================================================*/
{
	FE_value xi[3];
	int return_code;
	struct Computed_field_compose_type_specific_data *data;
	struct FE_element *element;

	ENTER(Computed_field_compose_is_defined_at_node);
	if (field && node && (data = 
		(struct Computed_field_compose_type_specific_data *)
		field->type_specific_data))
	{
		return_code=1;
		if (return_code = Computed_field_is_defined_at_node(
			field->source_fields[0],node))
		{
			if (return_code=
				Computed_field_evaluate_cache_at_node(
				field->source_fields[0],node,/*time*/0.0))
			{
				if (Computed_field_find_element_xi(
					field->source_fields[1], field->source_fields[0]->values,
					field->source_fields[0]->number_of_components, &element, xi,
					/*element_dimension=all*/0,data->region, /*propagate_field*/0,
					/*find_nearest_location*/0) && element)
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
} /* Computed_field_compose_is_defined_at_node */

#define Computed_field_compose_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_compose_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_compose_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value compose_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int i, return_code;
	struct Computed_field_compose_type_specific_data *data;
	struct FE_element *compose_element;

	ENTER(Computed_field_compose_evaluate_cache_at_node);
	if (field && node && (data = 
		(struct Computed_field_compose_type_specific_data *)
		field->type_specific_data))
	{
		/* 1. Precalculate any source fields that this field depends on */
		/* only calculate the first source_field at this location */
		if (return_code=Computed_field_evaluate_cache_at_node(
			field->source_fields[0],node,time))
		{
			/* 2. Calculate the field */
			/* The values from the first source field are inverted in the
				second source field to get element_xi which is evaluated with
				the third source field */
			if ((return_code = Computed_field_find_element_xi(field->source_fields[1],
				field->source_fields[0]->values,
				field->source_fields[0]->number_of_components,
					  &compose_element, compose_xi, /*element_dimension=all*/0,
						data->region, /*propagate_field*/0, /*find_nearest_location*/0))
				&& compose_element)
			{
				/* calculate the third source_field at this new location */
				return_code=Computed_field_evaluate_cache_in_element(
					field->source_fields[2],compose_element,compose_xi,
					time,/*top_level*/(struct FE_element *)NULL,
					/*calculate_derivatives*/0);
				for (i=0;i<field->number_of_components;i++)
				{
					field->values[i]=field->source_fields[2]->values[i];
				}
			}
			else
			{
				/* Actually don't fail here, just make the values constant so that
					people can compose outside the valid range */
				return_code = 1;
				for (i=0;i<field->number_of_components;i++)
				{
					field->values[i]=0.5;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_compose_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_compose_evaluate_cache_at_node */

static int Computed_field_compose_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value compose_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int i, return_code;
	struct Computed_field_compose_type_specific_data *data;
	struct FE_element *compose_element;

	ENTER(Computed_field_compose_evaluate_cache_in_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi && (data = 
		(struct Computed_field_compose_type_specific_data *)
		field->type_specific_data))
	{
		/* 1. Precalculate any source fields that this field depends on */
		/* only calculate the first source_field at this location */
		if (return_code=Computed_field_evaluate_cache_in_element(
			field->source_fields[0],element,xi,time,top_level_element,0))
		{
			/* 2. Calculate the field */
			/* The values from the first source field are inverted in the
				second source field to get element_xi which is evaluated with
				the third source field */
			if ((return_code = Computed_field_find_element_xi(field->source_fields[1],
				field->source_fields[0]->values,
				field->source_fields[0]->number_of_components,
					  &compose_element, compose_xi, /*element_dimension=all*/0,
						data->region, /*propagate_field*/0, /*find_nearest_location*/0))
				&& compose_element)
			{
				/* calculate the third source_field at this new location */
				return_code=Computed_field_evaluate_cache_in_element(
					field->source_fields[2],compose_element,compose_xi,
					time,/*top_level*/(struct FE_element *)NULL,
					/*calculate_derivatives*/0);
				for (i=0;i<field->number_of_components;i++)
				{
					field->values[i]=field->source_fields[2]->values[i];
				}
				field->derivatives_valid = 0;
			}
			else
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
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_compose_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_compose_evaluate_cache_in_element */

#define Computed_field_compose_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_compose_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_compose_set_values_at_node \
	(Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Unavailable for this field type.
==============================================================================*/

#define Computed_field_compose_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Unavailable for this field type.
==============================================================================*/

#define Computed_field_compose_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_compose_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_compose_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

static int list_Computed_field_compose(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_compose_type_specific_data *data;

	ENTER(List_Computed_field_compose);
	if (field && (data = 
		(struct Computed_field_compose_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE,"    texture coordinates field :");
		display_message(INFORMATION_MESSAGE," %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    find element xi field :");
		display_message(INFORMATION_MESSAGE," %s\n",
			field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,"    search element group :");
		display_message(INFORMATION_MESSAGE," %s\n", data->region_path);
		display_message(INFORMATION_MESSAGE,"    calculate values field :");
		display_message(INFORMATION_MESSAGE," %s\n",
			field->source_fields[2]->name);
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

static char *Computed_field_compose_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *group_name, *field_name;
	int error;
	struct Computed_field_compose_type_specific_data *data;

	ENTER(Computed_field_compose_get_command_string);
	command_string = (char *)NULL;
	if (field && (data = 
		(struct Computed_field_compose_type_specific_data *)
		field->type_specific_data))
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
		if (group_name = duplicate_string(data->region_path))
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_compose_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_compose_get_command_string */

#define Computed_field_compose_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_compose(struct Computed_field *field,
	struct Computed_field *texture_coordinate_field,
	struct Computed_field *find_element_xi_field,
	struct Computed_field *calculate_values_field,
	struct Cmiss_region *search_region, char *region_path)
/*******************************************************************************
LAST MODIFIED : 7 January 2003

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_COMPOSE, this field allows you to
evaluate one field to find "texture coordinates", use a find_element_xi field
to then calculate a corresponding element/xi and finally calculate values using
this element/xi and a third field.  You can then evaluate values on a "host"
mesh for any points "contained" inside.  The <search_element_group> is the group
from which any returned element_xi will belong.
The <region_path> string is supplied so that the commands listed can correctly
name the string used to select the <search_region>.
==============================================================================*/
{
	char *region_path_copy;
	int number_of_source_fields, return_code;
	struct Computed_field **source_fields;
	struct Computed_field_compose_type_specific_data *data;

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
					number_of_source_fields) &&
					ALLOCATE(data, struct Computed_field_compose_type_specific_data, 1)
					&& (region_path_copy = duplicate_string(region_path)))
				{
					/* 2. free current type-specific data */
					Computed_field_clear_type(field);
					/* 3. establish the new type */
					field->type_string = computed_field_compose_type_string;
					field->number_of_components=
						calculate_values_field->number_of_components;
					source_fields[0]=ACCESS(Computed_field)(texture_coordinate_field);
					source_fields[1]=ACCESS(Computed_field)(find_element_xi_field);
					source_fields[2]=ACCESS(Computed_field)(calculate_values_field);
					field->source_fields=source_fields;
					field->number_of_source_fields=number_of_source_fields;
					field->type_specific_data = (void *)data;
					data->region = ACCESS(Cmiss_region)(search_region);
					data->region_path = region_path_copy;

					/* Set all the methods */
					COMPUTED_FIELD_ESTABLISH_METHODS(compose);
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
	struct Cmiss_region **search_region, char **region_path)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
If the field is of type COMPUTED_FIELD_COMPOSE, the function returns the three
fields which define the field.
Note that the fields are not ACCESSed and the <region_path> points to the
internally used path.
==============================================================================*/
{
	int return_code;
	struct Computed_field_compose_type_specific_data *data;

	ENTER(Computed_field_get_type_compose);
	if (field && (field->type_string == computed_field_compose_type_string) &&
		(data = (struct Computed_field_compose_type_specific_data *)
		field->type_specific_data) && texture_coordinate_field &&
		find_element_xi_field && calculate_values_field && search_region)
	{
		*texture_coordinate_field = field->source_fields[0];
		*find_element_xi_field = field->source_fields[1];
		*calculate_values_field = field->source_fields[2];
		*search_region = data->region;
		*region_path = data->region_path;
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

static int define_Computed_field_type_compose(struct Parse_state *state,
	void *field_void, void *computed_field_compose_package_void)
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_COMPOSE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *old_region_path, *search_region_path;
	int return_code;
	struct Computed_field *field, *calculate_values_field,*find_element_xi_field,
		*texture_coordinates_field;
	struct Computed_field_compose_package *computed_field_compose_package;
	struct Coordinate_system *coordinate_system_ptr;
	struct Cmiss_region *search_region;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_calculate_values_field_data,
		set_find_element_xi_field_data, set_texture_coordinates_field_data;

	ENTER(define_Computed_field_type_compose);
	if (state && (field = (struct Computed_field *)field_void) &&
		(computed_field_compose_package =
			(struct Computed_field_compose_package *)
			computed_field_compose_package_void))
	{
		return_code = 1;
		search_region = (struct Cmiss_region *)NULL;
		search_region_path = (char *)NULL;
		old_region_path = (char *)NULL;
		calculate_values_field = (struct Computed_field *)NULL;
		find_element_xi_field = (struct Computed_field *)NULL;
		texture_coordinates_field = (struct Computed_field *)NULL;
		/* get valid parameters for composite field */
		if (computed_field_compose_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_compose(field, 
				&calculate_values_field, &find_element_xi_field,
				&texture_coordinates_field, &search_region, &old_region_path);
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
			/* calculate_values_field */
			set_calculate_values_field_data.computed_field_manager =
				computed_field_compose_package->computed_field_manager;
			set_calculate_values_field_data.conditional_function = 
				Computed_field_has_numerical_components;
			set_calculate_values_field_data.conditional_function_user_data = 
				(void *)NULL;
			Option_table_add_entry(option_table, "calculate_values_field", 
				&calculate_values_field, &set_calculate_values_field_data, 
				set_Computed_field_conditional);
			/* find_element_xi_field */
			set_find_element_xi_field_data.computed_field_manager =
				computed_field_compose_package->computed_field_manager;
			set_find_element_xi_field_data.conditional_function = 
				Computed_field_has_numerical_components;
			set_find_element_xi_field_data.conditional_function_user_data = 
				(void *)NULL;
			Option_table_add_entry(option_table, "find_element_xi_field", 
				&find_element_xi_field, &set_find_element_xi_field_data, 
				set_Computed_field_conditional);
			/* group */
			Option_table_add_set_Cmiss_region_path(option_table, "group", 
				 computed_field_compose_package->root_region, &search_region_path);
			/* texture_coordinates_field */
			set_texture_coordinates_field_data.computed_field_manager =
				computed_field_compose_package->computed_field_manager;
			set_texture_coordinates_field_data.conditional_function = 
				Computed_field_has_numerical_components;
			set_texture_coordinates_field_data.conditional_function_user_data = 
				(void *)NULL;
			Option_table_add_entry(option_table, "texture_coordinates_field", 
				&texture_coordinates_field, &set_texture_coordinates_field_data, 
				set_Computed_field_conditional);
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
				if (return_code=Computed_field_set_type_compose(field,
					texture_coordinates_field, find_element_xi_field,
					calculate_values_field, search_region, search_region_path))
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
LAST MODIFIED : 7 January 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_compose_package 
		computed_field_compose_package;

	ENTER(Computed_field_register_types_compose);
	if (computed_field_package && root_region)
	{
		computed_field_compose_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
			computed_field_package);
		computed_field_compose_package.root_region = root_region;
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_compose_type_string, 
			define_Computed_field_type_compose,
			&computed_field_compose_package);
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
