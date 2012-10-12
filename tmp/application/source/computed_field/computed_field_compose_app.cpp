
#include "api/cmiss_field_subobject_group.h"
#include "api/cmiss_field_constant.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_compose.h"
// insert app headers here
#include "mesh/cmiss_element_private_app.hpp"

Computed_field *Computed_field_create_compose(Cmiss_field_module *field_module,
	struct Computed_field *texture_coordinate_field,
	struct Computed_field *find_element_xi_field,
	struct Computed_field *calculate_values_field,
	Cmiss_mesh_id search_mesh,
	int find_nearest, int use_point_five_when_out_of_bounds);

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
