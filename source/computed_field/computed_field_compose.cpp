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
#include "api/cmiss_field_module.h"
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
#include "computed_field/field_module.hpp"
#include "mesh/cmiss_element_private.hpp"

class Computed_field_compose_package : public Computed_field_type_package
{
public:
	struct Cmiss_region *root_region;
};

namespace {

char computed_field_compose_type_string[] = "compose";

class Computed_field_compose : public Computed_field_core
{
private:
	Cmiss_mesh_id mesh;
	int find_nearest;
	int use_point_five_when_out_of_bounds;

public:
	Computed_field_compose(Cmiss_mesh_id search_mesh,
			int find_nearest, int use_point_five_when_out_of_bounds = 0) :
		Computed_field_core(),
		mesh(Cmiss_mesh_access(search_mesh)),
		find_nearest(find_nearest),
		use_point_five_when_out_of_bounds(use_point_five_when_out_of_bounds)
	{		
	};
		
	virtual ~Computed_field_compose();

	virtual void inherit_source_field_attributes()
	{
		if (field)
		{
			/* inherit coordinate system from third source field */
			Computed_field *calculate_values_field = field->source_fields[2];
			Computed_field_set_coordinate_system(field,
				Computed_field_get_coordinate_system(calculate_values_field));
		}
	}

	int get_type(
		struct Computed_field **texture_coordinate_field_address,
		struct Computed_field **find_element_xi_field_address,
		struct Computed_field **calculate_values_field_address,
		Cmiss_mesh_id *mesh_address, int *find_nearest_address,
		int *use_point_five_when_out_of_bounds_address);

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_compose_type_string);
	}

	int compare(Computed_field_core* other_field);

	virtual bool is_defined_at_location(Cmiss_field_cache& cache);

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

Computed_field_compose::~Computed_field_compose()
{
	Cmiss_mesh_destroy(&mesh);
}

Computed_field_core *Computed_field_compose::copy()
{
	return new Computed_field_compose(mesh, find_nearest, use_point_five_when_out_of_bounds);
}

int Computed_field_compose::compare(Computed_field_core *other_core)
{
	int return_code;
	struct Computed_field_compose *other;

	ENTER(Computed_field_compose::type_specific_contents_match);
	if (field && (0 != (other = dynamic_cast<Computed_field_compose*>(other_core))))
	{
		if (Cmiss_mesh_match(mesh, other->mesh) &&
			(find_nearest == other->find_nearest) &&
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

bool Computed_field_compose::is_defined_at_location(Cmiss_field_cache& cache)
{
	return (0 != field->evaluate(cache));
}

int Computed_field_compose::evaluate(Cmiss_field_cache& cache, FieldValueCache& inValueCache)
{
	int return_code = 0;
	RealFieldValueCache *coordinateValueCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
	if (coordinateValueCache)
	{
		RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
		Cmiss_field_cache& extraCache = *valueCache.getExtraCache();
		extraCache.setTime(cache.getTime());
		/* The values from the first source field are inverted in the
			second source field to get element_xi which is evaluated with
			the third source field */
		Cmiss_element_id compose_element =0;
		FE_value compose_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
		if (Computed_field_find_element_xi(getSourceField(1), &extraCache,
			coordinateValueCache->values,
			coordinateValueCache->componentCount,
			&compose_element, compose_xi,
			mesh, /*propagate_field*/0, find_nearest)
			&& compose_element)
		{
			/* calculate the third source_field at this new location */
			extraCache.setMeshLocation(compose_element, compose_xi);
			RealFieldValueCache *calculateValueCache = RealFieldValueCache::cast(getSourceField(2)->evaluate(extraCache));
			if (calculateValueCache)
			{
				return_code = 1;
				for (int i=0;i<field->number_of_components;i++)
				{
					valueCache.values[i] = calculateValueCache->values[i];
				}
			}
		}
		else
		{
			if (use_point_five_when_out_of_bounds)
			{
				/* Actually don't fail here, just make the values constant so that
					people can compose outside the valid range */
				return_code = 1;
				for (int i=0;i<field->number_of_components;i++)
				{
					valueCache.values[i]=0.5;
				}
			}
			else
			{
				return_code = 0;
			}
		}
		valueCache.derivatives_valid = 0;
	}
	return return_code;
}

int Computed_field_compose::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code = 0;

	ENTER(Computed_field_compose::list);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    texture coordinates field :");
		display_message(INFORMATION_MESSAGE," %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    find element xi field :");
		display_message(INFORMATION_MESSAGE," %s\n",
			field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE, "    mesh : ");
		char *mesh_name = Cmiss_mesh_get_name(mesh);
		display_message(INFORMATION_MESSAGE, "%s\n", mesh_name);
		DEALLOCATE(mesh_name);
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
		display_message(INFORMATION_MESSAGE,"    element dimension %d\n", Cmiss_mesh_get_dimension(mesh));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_compose::list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

char *Computed_field_compose::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
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
		append_string(&command_string, " mesh ", &error);
		char *mesh_name = Cmiss_mesh_get_name(mesh);
		append_string(&command_string, mesh_name, &error);
		DEALLOCATE(mesh_name);
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
		sprintf(temp_string, " element_dimension %d", Cmiss_mesh_get_dimension(mesh));
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

/***************************************************************************//**
 * Creates a field of type COMPUTED_FIELD_COMPOSE. This field allows you to
 * evaluate one field to find "texture coordinates", use a find_element_xi field
 * to then calculate a corresponding element/xi and finally calculate values
 * using this element/xi and a third field.  You can then evaluate values on a
 * "host" mesh for any points "contained" inside.  The <search_mesh> is the
 * domain from which any returned element_xi will belong.
 * If <use_point_five_when_out_of_bounds> is true then if the
 * texture_coordinate_field values cannot be found in the find_element_xi_field,
 * then instead of returning failure, the values will be set to 0.5 and returned
 * as success.
 * NOTE: this field type has been superceded by find_mesh_location combined with
 * embedded field. DO NOT add to external API.
 */
Computed_field *Computed_field_create_compose(Cmiss_field_module *field_module,
	struct Computed_field *texture_coordinate_field,
	struct Computed_field *find_element_xi_field,
	struct Computed_field *calculate_values_field,
	Cmiss_mesh_id search_mesh,
	int find_nearest, int use_point_five_when_out_of_bounds)
{
	Computed_field *field = NULL;

	ENTER(Computed_field_create_compose);
	if (texture_coordinate_field && texture_coordinate_field->isNumerical() &&
		find_element_xi_field && find_element_xi_field->isNumerical() &&
		calculate_values_field && calculate_values_field->isNumerical() &&
		search_mesh &&
		(Cmiss_mesh_get_master_region_internal(search_mesh) ==
			Cmiss_field_module_get_master_region_internal(field_module)))
	{
		if (texture_coordinate_field->number_of_components ==
			find_element_xi_field->number_of_components)
		{
			if (Computed_field_is_find_element_xi_capable(
				find_element_xi_field, /*dummy*/NULL))
			{
				Computed_field *source_fields[3];
				source_fields[0] = texture_coordinate_field;
				source_fields[1] = find_element_xi_field;
				source_fields[2] = calculate_values_field;
				field = Computed_field_create_generic(field_module,
					/*check_source_field_regions*/true,
					calculate_values_field->number_of_components,
					/*number_of_source_fields*/3, source_fields,
					/*number_of_source_values*/0, NULL,
					new Computed_field_compose(search_mesh, find_nearest,
						use_point_five_when_out_of_bounds));
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_create_compose.  "
					"The type of find_element_xi_field supplied has not "
					"been implemented for find_element_xi calculations.");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_create_compose.  "
				"The texture_coordinate_field and find_element_xi_field "
				"must have the same number of components");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_compose.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* Computed_field_create_compose */

/***************************************************************************//**
 * If the field is of type COMPUTED_FIELD_COMPOSE, the function returns the
 * three fields which define the field.
 * Note that the fields and mesh are not ACCESSed.
 */
int Computed_field_compose::get_type(
	struct Computed_field **texture_coordinate_field_address,
	struct Computed_field **find_element_xi_field_address,
	struct Computed_field **calculate_values_field_address,
	Cmiss_mesh_id *mesh_address, int *find_nearest_address,
	int *use_point_five_when_out_of_bounds_address)
{
	int return_code;
	if (field && texture_coordinate_field_address &&
		find_element_xi_field_address && calculate_values_field_address &&
		mesh_address && find_nearest_address &&
		use_point_five_when_out_of_bounds_address)
	{
		*texture_coordinate_field_address = field->source_fields[0];
		*find_element_xi_field_address = field->source_fields[1];
		*calculate_values_field_address = field->source_fields[2];
		*mesh_address = mesh;
		*find_nearest_address = find_nearest;
		*use_point_five_when_out_of_bounds_address = use_point_five_when_out_of_bounds;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_compose::get_type.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int define_Computed_field_type_compose(struct Parse_state *state,
	void *field_modify_void, void *computed_field_compose_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_COMPOSE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	Computed_field_compose_package *computed_field_compose_package;
	Computed_field_modify_data *field_modify;
	struct Option_table *find_option_table, *option_table,
		*out_of_bounds_option_table;
	struct Set_Computed_field_conditional_data set_calculate_values_field_data,
		set_find_element_xi_field_data, set_texture_coordinates_field_data;

	ENTER(define_Computed_field_type_compose);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void) &&
		(computed_field_compose_package =
			(Computed_field_compose_package *)
			computed_field_compose_package_void))
	{
		return_code = 1;
		Cmiss_mesh_id mesh = 0;
		char *search_group_name = 0;
		Cmiss_field_id calculate_values_field = 0;
		Cmiss_field_id find_element_xi_field = 0;
		Cmiss_field_id texture_coordinates_field = 0;
		char find_nearest_flag = 0;
		char find_exact_flag = 0;
		char use_point_five_when_out_of_bounds_flag = 0;
		char fail_when_out_of_bounds_flag = 0;
		int use_point_five_when_out_of_bounds = 0;
		int element_dimension = 0;
		/* Maintain the existing behaviour as the default */
		int find_nearest = 0;
		/* get valid parameters for composite field */
		if (field_modify->get_field())
		{
			Computed_field_compose* compose_core =
				dynamic_cast<Computed_field_compose*>(field_modify->get_field()->core);
			if (compose_core)
			{
				return_code = compose_core->get_type(
					&calculate_values_field, &find_element_xi_field,
					&texture_coordinates_field, &mesh,
					&find_nearest, &use_point_five_when_out_of_bounds);
				if (mesh)
				{
					Cmiss_mesh_access(mesh);
					element_dimension = Cmiss_mesh_get_dimension(mesh);
				}
			}
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
				"The value of a compose field is found by evaluating the <texture_coordinates_field>, "
				"then searching for matching values of the <find_element_xi_field> in the elements of "
				"the <mesh> (alternatively specified by <group> and <element_dimension>) and then "
				"finally evaluating the <calculate_values_field> at this found location.  You can "
				"specify the outcome if the matching values cannot be found in the mesh with "
				"<use_point_five_when_out_of_bounds> or <fail_when_out_of_bounds>.  See examples "
				"a/resample_texture or a/create_slices where the compose field is used to find the "
				"equivalent coordinate in another element to evaluate a texture.");
			/* calculate_values_field */
			set_calculate_values_field_data.computed_field_manager =
				field_modify->get_field_manager();
			set_calculate_values_field_data.conditional_function = 
				Computed_field_has_numerical_components;
			set_calculate_values_field_data.conditional_function_user_data = 
				(void *)NULL;
			Option_table_add_entry(option_table, "calculate_values_field", 
				&calculate_values_field, &set_calculate_values_field_data, 
				set_Computed_field_conditional);
			Option_table_add_int_positive_entry(option_table,"element_dimension",
				&element_dimension);
			/* find_element_xi_field */
			set_find_element_xi_field_data.computed_field_manager =
				field_modify->get_field_manager();
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
			Option_table_add_string_entry(option_table, "group",
				 &search_group_name, " GROUP_NAME");
			// mesh
			Option_table_add_mesh_entry(option_table, "mesh", field_modify->get_region(), &mesh);
			/* texture_coordinates_field */
			set_texture_coordinates_field_data.computed_field_manager =
				field_modify->get_field_manager();
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

			if (return_code && !mesh)
			{
				mesh = Cmiss_field_module_find_mesh_by_dimension(
					field_modify->get_field_module(), element_dimension);
				if (search_group_name)
				{
					Cmiss_field_id group_field = Cmiss_field_module_find_field_by_name(field_modify->get_field_module(), search_group_name);
					Cmiss_field_group_id group = Cmiss_field_cast_group(group_field);
					Cmiss_field_element_group_id element_group = Cmiss_field_group_get_element_group(group, mesh);
					Cmiss_mesh_destroy(&mesh);
					mesh = Cmiss_mesh_group_base_cast(Cmiss_field_element_group_get_mesh(element_group));
					Cmiss_field_element_group_destroy(&element_group);
					Cmiss_field_group_destroy(&group);
					Cmiss_field_destroy(&group_field);
				}
			}
			if (return_code && !mesh)
			{
				display_message(ERROR_MESSAGE, "You must specify a mesh (or element_dimension and optional group).");
				return_code = 0;
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
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_compose(field_modify->get_field_module(),
						texture_coordinates_field, find_element_xi_field,
						calculate_values_field, mesh,
						find_nearest, use_point_five_when_out_of_bounds));
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
			Cmiss_mesh_destroy(&mesh);
			if (calculate_values_field)
			{
				DEACCESS(Computed_field)(&calculate_values_field);
			}
			if (find_element_xi_field)
			{
				DEACCESS(Computed_field)(&find_element_xi_field);
			}
			if (search_group_name)
			{
				DEALLOCATE(search_group_name);
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
