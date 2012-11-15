
#include "zinc/fieldsubobjectgroup.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_integration.h"
#include "computed_field/computed_field_composite.h"
#include "mesh/cmiss_element_private_app.hpp"
#include "finite_element/finite_element_region.h"
#include "computed_field/computed_field_finite_element.h"

class Computed_field_integration_package : public Computed_field_type_package
{
public:
	Cmiss_region *root_region;
};

const char computed_field_xi_texture_coordinates_type_string[] = "xi_texture_coordinates";
const char computed_field_integration_type_string[] = "integration";

int Computed_field_get_type_integration(Computed_field *field,
	Cmiss_mesh_id *mesh_address, FE_element **seed_element,
	Computed_field **integrand, int *magnitude_coordinates,
	Computed_field **coordinate_field);

int define_Computed_field_type_integration(Parse_state *state,
	void *field_modify_void,void *computed_field_integration_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_INTEGRATION (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code = 1;
	ENTER(define_Computed_field_type_integration);
	Computed_field_modify_data *field_modify = reinterpret_cast<Computed_field_modify_data *>(field_modify_void);
	USE_PARAMETER(computed_field_integration_package_void);
	if (state && field_modify)
	{
		Cmiss_region_id region = field_modify->get_region();
		Cmiss_mesh_id mesh = 0;
		Cmiss_field_id coordinate_field = 0;
		Cmiss_field_id integrand = 0;
		int magnitude_coordinates_flag = 0;
		int seed_element_identifier = 0;
		float time_update = 0;
		if ((NULL != field_modify->get_field()) &&
			Computed_field_is_type_integration(field_modify->get_field()))
		{
			Cmiss_element_id seed_element;
			return_code = Computed_field_get_type_integration(field_modify->get_field(),
				&mesh, &seed_element, &integrand, &magnitude_coordinates_flag, &coordinate_field);
			if (seed_element)
			{
				seed_element_identifier = Cmiss_element_get_identifier(seed_element);
				Cmiss_element_destroy(&seed_element);
			}
		}
		if (coordinate_field)
		{
			ACCESS(Computed_field)(coordinate_field);
		}
		if (integrand)
		{
			ACCESS(Computed_field)(integrand);
		}
		else
		{
			/* Make a default integrand of one */
			double value = 1.0;
			// use temporary field module to supply different defaults
			Cmiss_field_module *temp_field_module = Cmiss_field_module_create(region);
			Cmiss_field_module_set_field_name(temp_field_module, "constant_1.0");
			integrand = Computed_field_create_constant(temp_field_module,
				/*number_of_components*/1, &value);
			if (NULL == integrand)
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_integration.  Unable to create constant integrand");
				return_code = 0;
			}
			Cmiss_field_module_destroy(&temp_field_module);
		}
		char *group_name = 0;

		Option_table *option_table = CREATE(Option_table)();
		/* coordinate */
		Set_Computed_field_conditional_data set_coordinate_field_data;
		set_coordinate_field_data.computed_field_manager = field_modify->get_field_manager();
		set_coordinate_field_data.conditional_function = Computed_field_has_up_to_3_numerical_components;
		set_coordinate_field_data.conditional_function_user_data = 0;
		Option_table_add_Computed_field_conditional_entry(option_table, "coordinate",
			&coordinate_field, &set_coordinate_field_data);
		/* integrand */
		Set_Computed_field_conditional_data set_integrand_field_data;
		set_integrand_field_data.computed_field_manager = field_modify->get_field_manager();
		set_integrand_field_data.conditional_function = Computed_field_is_scalar;
		set_integrand_field_data.conditional_function_user_data = 0;
		Option_table_add_Computed_field_conditional_entry(option_table, "integrand",
			&integrand, &set_integrand_field_data);
		/* magnitude_coordinates|no_magnitude_coordinates */
		Option_table_add_switch(option_table, "magnitude_coordinates", "no_magnitude_coordinates",
			&magnitude_coordinates_flag);
		// mesh
		Option_table_add_mesh_entry(option_table, "mesh", region, &mesh);
		/* region - legacy group name */
		Option_table_add_string_entry(option_table, "region", &group_name, " GROUP_NAME(DEPRECATED)");
		/* seed_element */
		Option_table_add_int_non_negative_entry(option_table, "seed_element",
			&seed_element_identifier);
		/* update_time_integration */
		Option_table_add_entry(option_table,"update_time_integration",
			&time_update, NULL, set_float);
		return_code = Option_table_multi_parse(option_table,state);
		DESTROY(Option_table)(&option_table);

		if (return_code && !mesh)
		{
			int dimension = FE_region_get_highest_dimension(Cmiss_region_get_FE_region(region));
			mesh = Cmiss_field_module_find_mesh_by_dimension(field_modify->get_field_module(), dimension);
			if (group_name)
			{
				Cmiss_field_id group_field = Cmiss_field_module_find_field_by_name(field_modify->get_field_module(), group_name);
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
			display_message(ERROR_MESSAGE, "You must specify a mesh.");
			return_code = 0;
		}
		if (return_code && !coordinate_field)
		{
			display_message(ERROR_MESSAGE, "You must specify a coordinate field.");
			return_code = 0;
		}
		if (return_code && !integrand)
		{
			display_message(ERROR_MESSAGE, "You must specify an integrand field.");
			return_code = 0;
		}
		Cmiss_element_id seed_element = 0;
		if (return_code)
		{
			seed_element = Cmiss_mesh_find_element_by_identifier(mesh, seed_element_identifier);
			if (!seed_element)
			{
				display_message(ERROR_MESSAGE, "Could not find seed_element %d in mesh.", seed_element_identifier);
				return_code = 0;
			}
		}
		if (return_code)
		{
			if (time_update && (NULL != field_modify->get_field()) &&
				Computed_field_is_type_integration(field_modify->get_field()))
			{
				display_message(ERROR_MESSAGE,
					"The update_time_integration code has not been updated"
					"with the latest changes.");
				return_code=0;
			}
			else
			{
				return_code = field_modify->update_field_and_deaccess(
					Computed_field_create_integration(field_modify->get_field_module(),
						mesh, seed_element, integrand, magnitude_coordinates_flag, coordinate_field));
			}
		}
		if (group_name)
		{
			DEALLOCATE(group_name);
		}
		Cmiss_element_destroy(&seed_element);
		Cmiss_mesh_destroy(&mesh);
		Cmiss_field_destroy(&coordinate_field);
		Cmiss_field_destroy(&integrand);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_integration.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int define_Computed_field_type_xi_texture_coordinates(Parse_state *state,
	void *field_modify_void,void *computed_field_integration_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_XI_TEXTURE_COORDINATES (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code = 1;
	ENTER(define_Computed_field_type_xi_texture_coordinates);
	Computed_field_modify_data *field_modify = reinterpret_cast<Computed_field_modify_data *>(field_modify_void);
	USE_PARAMETER(computed_field_integration_package_void);
	if (state && field_modify)
	{
		Cmiss_region_id region = field_modify->get_region();
		Cmiss_mesh_id mesh = 0;
		int seed_element_identifier = 0;
		char *group_name = 0;

		Option_table *option_table = CREATE(Option_table)();
		// mesh
		Option_table_add_mesh_entry(option_table, "mesh", region, &mesh);
		/* region - legacy group name */
		Option_table_add_string_entry(option_table, "region", &group_name, " GROUP_NAME(OBSOLETE)");
		/* seed_element */
		Option_table_add_int_non_negative_entry(option_table, "seed_element",
			&seed_element_identifier);
		return_code = Option_table_multi_parse(option_table,state);
		DESTROY(Option_table)(&option_table);

		if (return_code && !mesh)
		{
			int dimension = FE_region_get_highest_dimension(Cmiss_region_get_FE_region(region));
			mesh = Cmiss_field_module_find_mesh_by_dimension(field_modify->get_field_module(), dimension);
			if (group_name)
			{
				Cmiss_field_id group_field = Cmiss_field_module_find_field_by_name(field_modify->get_field_module(), group_name);
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
			display_message(ERROR_MESSAGE, "Must specify mesh.");
			return_code = 0;
		}
		Cmiss_element_id seed_element = 0;
		if (return_code)
		{
			seed_element = Cmiss_mesh_find_element_by_identifier(mesh, seed_element_identifier);
			if (!seed_element)
			{
				display_message(ERROR_MESSAGE, "Could not find seed_element %d in mesh.", seed_element_identifier);
				return_code = 0;
			}
		}
		// use temporary field module to supply different defaults
		Cmiss_field_id coordinate_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
			Computed_field_is_type_xi_coordinates, (void *)NULL,
			field_modify->get_field_manager());
		if (coordinate_field)
		{
			ACCESS(Computed_field)(coordinate_field);
		}
		else
		{
			if ((!state->current_token) ||
				(strcmp(PARSER_HELP_STRING, state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_xi_texture_coordinates.  xi field not found");
			}
			return_code = 0;
		}
		double value = 1.0;
		Cmiss_field_module *temp_field_module = Cmiss_field_module_create(region);
		Cmiss_field_module_set_field_name(temp_field_module, "constant_1.0");
		Computed_field *integrand = Computed_field_create_constant(temp_field_module,
			/*number_of_components*/1, &value);
		Cmiss_field_module_destroy(&temp_field_module);
		if (NULL == integrand)
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_xi_texture_coordinates.  Unable to create constant field");
			return_code = 0;
		}
		if (return_code)
		{
			return_code = field_modify->update_field_and_deaccess(
				Computed_field_create_integration(field_modify->get_field_module(),
					mesh, seed_element, integrand, /*magnitude_coordinates*/0, coordinate_field));
		}
		if (group_name)
		{
			DEALLOCATE(group_name);
		}
		Cmiss_element_destroy(&seed_element);
		Cmiss_mesh_destroy(&mesh);
		Cmiss_field_destroy(&coordinate_field);
		Cmiss_field_destroy(&integrand);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_xi_texture_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Computed_field_register_types_integration(
	Computed_field_package *computed_field_package,
	Cmiss_region *root_region)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_integration_package
		*computed_field_integration_package =
		new Computed_field_integration_package;

	ENTER(Computed_field_register_type_integration);
	if (computed_field_package)
	{
		computed_field_integration_package->root_region = root_region;
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_integration_type_string,
			define_Computed_field_type_integration,
			computed_field_integration_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_xi_texture_coordinates_type_string,
			define_Computed_field_type_xi_texture_coordinates,
			computed_field_integration_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_type_integration.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_type_integration */
