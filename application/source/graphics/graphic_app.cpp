
#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */

#include "zinc/graphicsmodule.h"
#include "zinc/graphicsfont.h"
#include "general/enumerator.h"
#include "general/enumerator_private.hpp"
#include "general/message.h"
#include "command/parser.h"
#include "graphics/graphic.h"
#include "graphics/render_gl.h"
#include "graphics/rendition.h"
#include "computed_field/computed_field_finite_element.h"
#include "graphics/auxiliary_graphics_types_app.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "graphics/graphics_object_app.h"
#include "graphics/spectrum_app.h"
#include "graphics/font.h"
#include "graphics/graphics_coordinate_system.hpp"
#include "graphics/material_app.h"
#include "finite_element/finite_element_region_app.h"
#include "graphics/tessellation_app.hpp"

namespace {

/***************************************************************************//**
 * Makes a new graphic of the supplied graphic_type, optionally a copy of an
 * existing_graphic.
 *
 * @param rendition  Source of graphics defaults if creating a new graphic.
 * @param graphic_type  The type of the new graphic.
 * @param existing_graphic  An existing graphic to copy settings from if of
 * same graphic_type.
 * @return  1 on success, 0 on failure.
 */
Cmiss_graphic* get_graphic_for_gfx_modify(Cmiss_rendition *rendition,
	Cmiss_graphic_type graphic_type, Cmiss_graphic *existing_graphic)
{
	Cmiss_graphic *graphic = CREATE(Cmiss_graphic)(graphic_type);
	if (existing_graphic && (graphic_type == existing_graphic->graphic_type))
	{
		Cmiss_graphic_copy_without_graphics_object(graphic, existing_graphic);
		// GRC not sure why this was done:
		// Cmiss_graphic_set_rendition_private(graphic, existing_graphic->rendition);
	}
	else
	{
		Cmiss_rendition_set_graphic_defaults(rendition, graphic);
		/* Set up the coordinate_field */
		// GRC move following to Cmiss_rendition_set_graphic_defaults ?
		// GRC can improve as logic is probably already in rendition
		if (!graphic->coordinate_field)
		{
			struct Cmiss_region *region = Cmiss_rendition_get_region(rendition);
			struct FE_region *fe_region =	Cmiss_region_get_FE_region(region);
			struct FE_region *data_region = FE_region_get_data_FE_region(fe_region);
			struct FE_field *fe_field;
			if (FE_region_get_default_coordinate_FE_field(fe_region, &fe_field) ||
				FE_region_get_default_coordinate_FE_field(data_region, &fe_field))
			{
				struct Computed_field *coordinate_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_wraps_fe_field,
					(void *)fe_field, Cmiss_region_get_Computed_field_manager(region));
				if (coordinate_field)
				{
					Cmiss_graphic_set_coordinate_field(graphic, coordinate_field);
				}
			}
		}
	}
	return (graphic);
}

} // namespace anonymous

int gfx_modify_rendition_graphic(struct Parse_state *state,
	enum Cmiss_graphic_type graphic_type, const char *help_text,
	struct Modify_rendition_data *modify_rendition_data,
	struct Rendition_command_data *rendition_command_data)
{
	if (!(state && rendition_command_data && modify_rendition_data))
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_rendition_graphic.  Invalid argument(s)");
		return 0;
	}
	Cmiss_graphic *graphic = 0;
	if (modify_rendition_data->modify_this_graphic)
	{
		graphic = Cmiss_graphic_access(modify_rendition_data->graphic);
		graphic_type = Cmiss_graphic_get_graphic_type(graphic);
	}
	else
	{
		graphic = get_graphic_for_gfx_modify(rendition_command_data->rendition,
			graphic_type, modify_rendition_data->graphic);
		if (modify_rendition_data->group && (!graphic->subgroup_field))
		{
			graphic->subgroup_field =
				Cmiss_field_access(Cmiss_field_group_base_cast(modify_rendition_data->group));
		}
	}
	if (!graphic)
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_rendition_graphic.  Could not create graphic");
		return 0;
	}
	REACCESS(Cmiss_graphic)(&(modify_rendition_data->graphic), graphic);

	int return_code = 1;
	Cmiss_graphics_coordinate_system coordinate_system = Cmiss_graphic_get_coordinate_system(graphic);
	char *font_name = (char *)NULL;
	Cmiss_field_id orientation_scale_field = 0;
	Cmiss_field_id variable_scale_field = 0;
	Cmiss_field_id xi_point_density_field = 0;
	GT_object *glyph = 0;
	Triple glyph_offset, glyph_scale_factors, glyph_size;
	enum Graphic_glyph_scaling_mode glyph_scaling_mode = GRAPHIC_GLYPH_SCALING_GENERAL;
	const char *glyph_scaling_mode_string = 0;
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_GLYPH))
	{
		Cmiss_graphic_get_glyph_parameters(graphic,
			&glyph, &glyph_scaling_mode, glyph_offset, glyph_size,
			&orientation_scale_field, glyph_scale_factors,
			&variable_scale_field);
		if (glyph)
			ACCESS(GT_object)(glyph);
		if (orientation_scale_field)
		{
			ACCESS(Computed_field)(orientation_scale_field);
		}
		if (variable_scale_field)
		{
			ACCESS(Computed_field)(variable_scale_field);
		}
		glyph_scaling_mode_string = ENUMERATOR_STRING(Graphic_glyph_scaling_mode)(glyph_scaling_mode);
	}
	enum Use_element_type use_element_type;
	const char *use_element_type_string = 0;
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_USE_ELEMENT_TYPE))
	{
		use_element_type = graphic->use_element_type;
		use_element_type_string = ENUMERATOR_STRING(Use_element_type)(use_element_type);
	}
	enum Xi_discretization_mode xi_discretization_mode;
	const char *xi_discretization_mode_string = 0;
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_XI_DISCRETIZATION_MODE))
	{
		Cmiss_graphic_get_xi_discretization(graphic,
			&xi_discretization_mode, &xi_point_density_field);
		if (xi_point_density_field)
		{
			ACCESS(Computed_field)(xi_point_density_field);
		}
		xi_discretization_mode_string = ENUMERATOR_STRING(Xi_discretization_mode)(xi_discretization_mode);
	}
	int number_of_components = 3;
	int visibility = graphic->visibility_flag;
	int number_of_valid_strings;
	const char **valid_strings;
	enum Cmiss_graphics_render_type render_type = graphic->render_type;
	const char *render_type_string = ENUMERATOR_STRING(Cmiss_graphics_render_type)(render_type);
	/* The value stored in the graphic is an integer rather than a char */
	char reverse_track = (graphic->reverse_track) ? 1 : 0;
	char *seed_nodeset_name = 0;
	if (graphic->seed_nodeset)
	{
		seed_nodeset_name = Cmiss_nodeset_get_name(graphic->seed_nodeset);
	}

	Option_table *option_table = CREATE(Option_table)();
	if (help_text)
	{
		Option_table_add_help(option_table, help_text);
	}

	/* as */
	Option_table_add_entry(option_table,"as",&(graphic->name),
		(void *)1,set_name);

	/* cell_centres/cell_corners/cell_density/exact_xi */
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_XI_DISCRETIZATION_MODE))
	{
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Xi_discretization_mode)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Xi_discretization_mode) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table,number_of_valid_strings,
			valid_strings,&xi_discretization_mode_string);
		DEALLOCATE(valid_strings);
	}

	/* centre */
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_GLYPH))
	{
		Option_table_add_entry(option_table,"centre",glyph_offset,
			&(number_of_components),set_reversed_float_vector);
	}

	/* circle_discretization */
	if (graphic_type == CMISS_GRAPHIC_CYLINDERS)
	{
		Option_table_add_entry(option_table, "circle_discretization",
			(void *)&(graphic->circle_discretization), (void *)NULL,
			set_Circle_discretization);
	}

	/* constant_radius */
	if (graphic_type == CMISS_GRAPHIC_CYLINDERS)
	{
		Option_table_add_entry(option_table,"constant_radius",
			&(graphic->constant_radius),NULL,set_float);
	}

	/* coordinate */
	Set_Computed_field_conditional_data set_coordinate_field_data;
	set_coordinate_field_data.computed_field_manager = rendition_command_data->computed_field_manager;
	set_coordinate_field_data.conditional_function = Computed_field_has_up_to_3_numerical_components;
	set_coordinate_field_data.conditional_function_user_data = (void *)NULL;
	Option_table_add_Computed_field_conditional_entry(option_table, "coordinate",
		&(graphic->coordinate_field), &set_coordinate_field_data);

	/* coordinate system */
	const char *coordinate_system_string =
		ENUMERATOR_STRING(Cmiss_graphics_coordinate_system)(coordinate_system);
	valid_strings = ENUMERATOR_GET_VALID_STRINGS(Cmiss_graphics_coordinate_system)(
		&number_of_valid_strings,
		(ENUMERATOR_CONDITIONAL_FUNCTION(Cmiss_graphics_coordinate_system) *)NULL,
		(void *)NULL);
	Option_table_add_enumerator(option_table, number_of_valid_strings,
		valid_strings, &coordinate_system_string);
	DEALLOCATE(valid_strings);

	/* data */
	Set_Computed_field_conditional_data set_data_field_data;
	set_data_field_data.computed_field_manager = rendition_command_data->computed_field_manager;
	set_data_field_data.conditional_function = Computed_field_has_numerical_components;
	set_data_field_data.conditional_function_user_data = (void *)NULL;
	Option_table_add_Computed_field_conditional_entry(option_table, "data",
		&(graphic->data_field), &set_data_field_data);

	/* decimation_threshold */
	if (graphic_type == CMISS_GRAPHIC_ISO_SURFACES)
	{
		Option_table_add_double_entry(option_table, "decimation_threshold",
			&(graphic->decimation_threshold));
	}

	/* delete */
	Option_table_add_entry(option_table,"delete",
		&(modify_rendition_data->delete_flag),NULL,set_char_flag);

	/* density */
	Set_Computed_field_conditional_data set_xi_point_density_field_data;
	set_xi_point_density_field_data.computed_field_manager = rendition_command_data->computed_field_manager;
	set_xi_point_density_field_data.conditional_function = Computed_field_is_scalar;
	set_xi_point_density_field_data.conditional_function_user_data = (void *)NULL;
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_XI_DISCRETIZATION_MODE))
	{
		Option_table_add_Computed_field_conditional_entry(option_table, "density",
			&xi_point_density_field, &set_xi_point_density_field_data);
	}

	/* discretization */
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_DISCRETIZATION))
	{
		Option_table_add_entry(option_table,"discretization",
			&(graphic->discretization),NULL, set_Element_discretization);
	}

	/* ellipse/line/rectangle/ribbon */
	Streamline_type streamline_type = STREAM_LINE;
	const char *streamline_type_string = ENUMERATOR_STRING(Streamline_type)(streamline_type);
	if (graphic_type == CMISS_GRAPHIC_STREAMLINES)
	{
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Streamline_type)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Streamline_type) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table,number_of_valid_strings,
			valid_strings,&streamline_type_string);
		DEALLOCATE(valid_strings);
	}

	/* exterior */
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_EXTERIOR_FLAG))
	{
		Option_table_add_entry(option_table,"exterior",&(graphic->exterior),
			NULL,set_char_flag);
	}

	/* face */
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_FACE))
	{
		Option_table_add_entry(option_table,"face",&(graphic->face),
			NULL,set_exterior);
	}

	/* first_iso_value */
	if (graphic_type == CMISS_GRAPHIC_ISO_SURFACES)
	{
		Option_table_add_double_entry(option_table,"first_iso_value",
			&(graphic->first_iso_value));
	}

	/* font */
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_LABEL_FIELD))
	{
		Option_table_add_name_entry(option_table, "font",
			&font_name);
	}

	/* glyph */
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_GLYPH))
	{
		Option_table_add_entry(option_table,"glyph",&glyph,
			rendition_command_data->glyph_manager,set_Graphics_object);
	}

	/* glyph scaling mode constant/scalar/vector/axes/general */
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_GLYPH))
	{
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Graphic_glyph_scaling_mode)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Graphic_glyph_scaling_mode) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
			valid_strings, &glyph_scaling_mode_string);
		DEALLOCATE(valid_strings);
	}

	/* iso_scalar */
	Set_Computed_field_conditional_data set_iso_scalar_field_data;
	set_iso_scalar_field_data.computed_field_manager = rendition_command_data->computed_field_manager;
	set_iso_scalar_field_data.conditional_function = Computed_field_is_scalar;
	set_iso_scalar_field_data.conditional_function_user_data = (void *)NULL;
	if (graphic_type == CMISS_GRAPHIC_ISO_SURFACES)
	{
		Option_table_add_Computed_field_conditional_entry(option_table, "iso_scalar",
			&(graphic->iso_scalar_field), &set_iso_scalar_field_data);
	}

	/* iso_values */
	if (graphic_type == CMISS_GRAPHIC_ISO_SURFACES)
	{
		Option_table_add_variable_length_double_vector_entry(option_table,
			"iso_values", &(graphic->number_of_iso_values), &(graphic->iso_values));
	}

	/* last_iso_value */
	if (graphic_type == CMISS_GRAPHIC_ISO_SURFACES)
	{
		Option_table_add_double_entry(option_table,"last_iso_value",
			&(graphic->last_iso_value));
	}

	/* label */
	Set_Computed_field_conditional_data set_label_field_data;
	set_label_field_data.computed_field_manager = rendition_command_data->computed_field_manager;
	set_label_field_data.conditional_function = (MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
	set_label_field_data.conditional_function_user_data = (void *)NULL;
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_LABEL_FIELD))
	{
		Option_table_add_Computed_field_conditional_entry(option_table, "label",
			&(graphic->label_field), &set_label_field_data);
	}

	/* ldensity */
	Set_Computed_field_conditional_data set_label_density_field_data;
	set_label_density_field_data.computed_field_manager = rendition_command_data->computed_field_manager;
	set_label_density_field_data.conditional_function = Computed_field_has_up_to_3_numerical_components;
	set_label_density_field_data.conditional_function_user_data = (void *)NULL;
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_LABEL_FIELD))
	{
		Option_table_add_Computed_field_conditional_entry(option_table, "ldensity",
			&(graphic->label_density_field), &set_label_density_field_data);
	}

	/* length */
	if (graphic_type == CMISS_GRAPHIC_STREAMLINES)
	{
		Option_table_add_entry(option_table,"length",
			&(graphic->streamline_length),NULL,set_float);
	}

	/* line_width */
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_LINE_WIDTH))
	{
		Option_table_add_int_non_negative_entry(option_table,"line_width",
			&(graphic->line_width));
	}

	/* material */
	Option_table_add_entry(option_table,"material",&(graphic->material),
		rendition_command_data->graphical_material_manager,
		set_Graphical_material);

	/* native_discretization */
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_NATIVE_DISCRETIZATION_FIELD))
	{
		Option_table_add_set_FE_field_from_FE_region(option_table,
			"native_discretization", &(graphic->native_discretization_field),
			Cmiss_region_get_FE_region(rendition_command_data->region));
	}

	/* no_data/field_scalar/magnitude_scalar/travel_scalar */
	Streamline_data_type streamline_data_type = STREAM_NO_DATA;
	const char *streamline_data_type_string =
		ENUMERATOR_STRING(Streamline_data_type)(streamline_data_type);
	if (graphic_type == CMISS_GRAPHIC_STREAMLINES)
	{
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Streamline_data_type)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Streamline_data_type) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
			valid_strings, &streamline_data_type_string);
		DEALLOCATE(valid_strings);
	}

	/* orientation */
	Set_Computed_field_conditional_data set_orientation_scale_field_data;
	set_orientation_scale_field_data.computed_field_manager = rendition_command_data->computed_field_manager;
	set_orientation_scale_field_data.conditional_function = Computed_field_is_orientation_scale_capable;
	set_orientation_scale_field_data.conditional_function_user_data = (void *)NULL;
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_GLYPH))
	{
		Option_table_add_Computed_field_conditional_entry(option_table, "orientation",
			&orientation_scale_field, &set_orientation_scale_field_data);
	}

	/* position */
	Option_table_add_entry(option_table,"position",
		&(modify_rendition_data->position),NULL,set_int_non_negative);

	/* radius_scalar */
	Set_Computed_field_conditional_data set_radius_scalar_field_data;
	set_radius_scalar_field_data.computed_field_manager = rendition_command_data->computed_field_manager;
	set_radius_scalar_field_data.conditional_function = Computed_field_is_scalar;
	set_radius_scalar_field_data.conditional_function_user_data = (void *)NULL;
	if (graphic_type == CMISS_GRAPHIC_CYLINDERS)
	{
		Option_table_add_Computed_field_conditional_entry(option_table, "radius_scalar",
			&(graphic->radius_scalar_field), &set_radius_scalar_field_data);
	}

	/* range_number_of_iso_values */
	/* Use a different temporary storage for this value so we
		can tell it from the original variable_length array
		"iso_values" above. */
	int range_number_of_iso_values = 0;
	if (graphic_type == CMISS_GRAPHIC_ISO_SURFACES)
	{
		Option_table_add_int_positive_entry(option_table,
			"range_number_of_iso_values", &range_number_of_iso_values);
	}

	/* render_type */
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_RENDER_TYPE))
	{
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Cmiss_graphics_render_type)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Cmiss_graphics_render_type) *)NULL, (void *)NULL);
		Option_table_add_enumerator(option_table,number_of_valid_strings,
			valid_strings,&render_type_string);
		DEALLOCATE(valid_strings);
	}

	/* reverse */
	if (graphic_type == CMISS_GRAPHIC_STREAMLINES)
	{
		Option_table_add_entry(option_table,"reverse_track",
			&reverse_track,NULL,set_char_flag);
	}

	/* scale_factor */
	if (graphic_type == CMISS_GRAPHIC_CYLINDERS)
	{
		Option_table_add_entry(option_table, "scale_factor",
			&(graphic->radius_scale_factor), NULL, set_float);
	}

	/* scale_factors */
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_GLYPH))
	{
		Option_table_add_special_float3_entry(option_table,"scale_factors",
			glyph_scale_factors,"*");
	}

	/* secondary_material (was: multipass_pass1_material) */
	if (graphic_type == CMISS_GRAPHIC_LINES)
	{
		Option_table_add_entry(option_table, "secondary_material",
			&(graphic->secondary_material), rendition_command_data->graphical_material_manager,
			set_Graphical_material);
	}

	/* seed_element */
	if (graphic_type == CMISS_GRAPHIC_STREAMLINES)
	{
		Option_table_add_entry(option_table, "seed_element",
			&(graphic->seed_element), Cmiss_region_get_FE_region(rendition_command_data->region),
			set_FE_element_top_level_FE_region);
	}

	// seed_node_mesh_location_field
	Set_Computed_field_conditional_data set_seed_mesh_location_field_data;
	set_seed_mesh_location_field_data.conditional_function = Computed_field_has_value_type_mesh_location;
	set_seed_mesh_location_field_data.conditional_function_user_data = (void *)NULL;
	set_seed_mesh_location_field_data.computed_field_manager = rendition_command_data->computed_field_manager;
	if (graphic_type == CMISS_GRAPHIC_STREAMLINES)
	{
		Option_table_add_Computed_field_conditional_entry(option_table, "seed_node_mesh_location_field",
			&(graphic->seed_node_mesh_location_field), &set_seed_mesh_location_field_data);
	}

	// seed_nodeset
	if (graphic_type == CMISS_GRAPHIC_STREAMLINES)
	{
		Option_table_add_string_entry(option_table, "seed_nodeset", &seed_nodeset_name,
			" NODE_GROUP_FIELD_NAME|[GROUP_NAME.]cmiss_nodes|cmiss_data|none");
	}

	/* select_mode */
	enum Graphics_select_mode select_mode = Cmiss_graphic_get_select_mode(graphic);
	const char *select_mode_string = ENUMERATOR_STRING(Graphics_select_mode)(select_mode);
	valid_strings = ENUMERATOR_GET_VALID_STRINGS(Graphics_select_mode)(
		&number_of_valid_strings,
		(ENUMERATOR_CONDITIONAL_FUNCTION(Graphics_select_mode) *)NULL,
		(void *)NULL);
	Option_table_add_enumerator(option_table,number_of_valid_strings,
		valid_strings,&select_mode_string);
	DEALLOCATE(valid_strings);

	/* selected_material */
	Option_table_add_entry(option_table,"selected_material",
		&(graphic->selected_material),
		rendition_command_data->graphical_material_manager,
		set_Graphical_material);

	/* size */
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_GLYPH))
	{
		Option_table_add_special_float3_entry(option_table,"size",
			glyph_size,"*");
	}

	/* spectrum */
	Option_table_add_entry(option_table,"spectrum",
		&(graphic->spectrum),rendition_command_data->spectrum_manager,
		set_Spectrum);

	/* subgroup field */
	Set_Computed_field_conditional_data set_subgroup_field_data;
	set_subgroup_field_data.computed_field_manager = rendition_command_data->computed_field_manager;
	set_subgroup_field_data.conditional_function = Computed_field_is_scalar;
	set_subgroup_field_data.conditional_function_user_data = (void *)NULL;
	Option_table_add_Computed_field_conditional_entry(option_table, "subgroup",
		&(graphic->subgroup_field), &set_subgroup_field_data);

	/* tessellation */
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_TESSELLATION))
	{
		Option_table_add_Cmiss_tessellation_entry(option_table, "tessellation",
			rendition_command_data->graphics_module, &(graphic->tessellation));
	}

	/* texture_coordinates */
	Set_Computed_field_conditional_data set_texture_coordinate_field_data;
	set_texture_coordinate_field_data.computed_field_manager = rendition_command_data->computed_field_manager;
	set_texture_coordinate_field_data.conditional_function = Computed_field_has_up_to_3_numerical_components;
	set_texture_coordinate_field_data.conditional_function_user_data = (void *)NULL;
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_TEXTURE_COORDINATE_FIELD))
	{
		Option_table_add_Computed_field_conditional_entry(option_table, "texture_coordinates",
			&(graphic->texture_coordinate_field), &set_texture_coordinate_field_data);
	}

	/* use_elements/use_faces/use_lines */
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_USE_ELEMENT_TYPE))
	{
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Use_element_type)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Use_element_type) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table,number_of_valid_strings,
			valid_strings,&use_element_type_string);
		DEALLOCATE(valid_strings);
	}

	/* variable_scale */
	Set_Computed_field_conditional_data set_variable_scale_field_data;
	set_variable_scale_field_data.computed_field_manager = rendition_command_data->computed_field_manager;
	set_variable_scale_field_data.conditional_function = Computed_field_has_up_to_3_numerical_components;
	set_variable_scale_field_data.conditional_function_user_data = (void *)NULL;
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_GLYPH))
	{
		Option_table_add_Computed_field_conditional_entry(option_table, "variable_scale",
			&variable_scale_field, &set_variable_scale_field_data);
	}

	/* vector */
	Set_Computed_field_conditional_data set_stream_vector_field_data;
	set_stream_vector_field_data.computed_field_manager = rendition_command_data->computed_field_manager;
	set_stream_vector_field_data.conditional_function = Computed_field_is_stream_vector_capable;
	set_stream_vector_field_data.conditional_function_user_data = (void *)NULL;
	if (graphic_type == CMISS_GRAPHIC_STREAMLINES)
	{
		Option_table_add_Computed_field_conditional_entry(option_table, "vector",
			&(graphic->stream_vector_field), &set_stream_vector_field_data);
	}

	/* visible/invisible */
	Option_table_add_switch(option_table, "visible", "invisible", &visibility);

	/* width */
	if (graphic_type == CMISS_GRAPHIC_STREAMLINES)
	{
		Option_table_add_entry(option_table,"width",
			&(graphic->streamline_width),NULL,set_float);
	}

	/* xi */
	if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_XI_DISCRETIZATION_MODE))
	{
		Option_table_add_entry(option_table,"xi",
			graphic->seed_xi,&number_of_components,set_float_vector);
	}

	if ((return_code=Option_table_multi_parse(option_table,state)))
	{
		if (graphic_type == CMISS_GRAPHIC_ISO_SURFACES)
		{
			if (graphic->iso_values)
			{
				if (range_number_of_iso_values || graphic->first_iso_value || graphic->last_iso_value)
				{
					display_message(ERROR_MESSAGE,
						"When specifying a list of <iso_values> do not specify <range_number_of_iso_values>, <first_iso_value> or <last_iso_value>.");
					return_code=0;
				}
			}
			else
			{
				if (range_number_of_iso_values)
				{
					graphic->number_of_iso_values = range_number_of_iso_values;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"You must either specify a list of <iso_values> or a range with <range_number_of_iso_values>, <first_iso_value> or <last_iso_value>.");
					return_code = 0;
				}
			}
			if ((struct Computed_field *)NULL == graphic->iso_scalar_field)
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_rendition_iso_surfaces.  Missing iso_scalar field");
				return_code=0;
			}
		}

		if (graphic->data_field&&!graphic->spectrum)
		{
			graphic->spectrum=ACCESS(Spectrum)(
				rendition_command_data->default_spectrum);
		}
		Cmiss_graphic_set_visibility_flag(graphic, visibility);

		if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_XI_DISCRETIZATION_MODE))
		{
			STRING_TO_ENUMERATOR(Xi_discretization_mode)(
				xi_discretization_mode_string, &xi_discretization_mode);
			if (((XI_DISCRETIZATION_CELL_DENSITY != xi_discretization_mode) &&
				(XI_DISCRETIZATION_CELL_POISSON != xi_discretization_mode)) ||
				xi_point_density_field)
			{
				Cmiss_graphic_set_xi_discretization(graphic,
					xi_discretization_mode, xi_point_density_field);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"No density field specified for cell_density|cell_poisson");
				return_code = 0;
			}
		}

		if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_USE_ELEMENT_TYPE))
		{
			STRING_TO_ENUMERATOR(Use_element_type)(use_element_type_string, &use_element_type);
			Cmiss_graphic_set_use_element_type(graphic, use_element_type);
		}

		if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_LABEL_FIELD))
		{
			Cmiss_graphics_font *new_font = 0;
			if (font_name && (0 != (new_font = Cmiss_graphics_module_find_font_by_name(
				rendition_command_data->graphics_module, font_name))))
			{
				REACCESS(Cmiss_graphics_font)(&graphic->font, new_font);
				Cmiss_graphics_font_destroy(&new_font);
			}
		}

		if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_GLYPH))
		{
			STRING_TO_ENUMERATOR(Graphic_glyph_scaling_mode)(
				glyph_scaling_mode_string, &glyph_scaling_mode);
			Cmiss_graphic_set_glyph_parameters(graphic,
				glyph, glyph_scaling_mode, glyph_offset, glyph_size,
				orientation_scale_field,glyph_scale_factors,
				variable_scale_field);
		}

		STRING_TO_ENUMERATOR(Cmiss_graphics_coordinate_system)(
			coordinate_system_string, &coordinate_system);
		Cmiss_graphic_set_coordinate_system(graphic, coordinate_system);

		if (Cmiss_graphic_type_uses_attribute(graphic_type, CMISS_GRAPHIC_ATTRIBUTE_RENDER_TYPE))
		{
			STRING_TO_ENUMERATOR(Cmiss_graphics_render_type)(render_type_string, &render_type);
			Cmiss_graphic_set_render_type(graphic, render_type);
		}

		STRING_TO_ENUMERATOR(Graphics_select_mode)(select_mode_string, &select_mode);
		Cmiss_graphic_set_select_mode(graphic, select_mode);

		if (graphic_type == CMISS_GRAPHIC_STREAMLINES)
		{
			if (!(graphic->stream_vector_field))
			{
				display_message(INFORMATION_MESSAGE,"Must specify a vector before any streamlines can be created");
			}
			if (return_code && seed_nodeset_name)
			{
				Cmiss_field_module_id field_module = Cmiss_region_get_field_module(rendition_command_data->region);
				Cmiss_nodeset_id seed_nodeset =
					Cmiss_field_module_find_nodeset_by_name(field_module, seed_nodeset_name);
				if (seed_nodeset || (fuzzy_string_compare(seed_nodeset_name, "none")))
				{
					if (graphic->seed_nodeset)
					{
						Cmiss_nodeset_destroy(&graphic->seed_nodeset);
					}
					// take over reference:
					graphic->seed_nodeset = seed_nodeset;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Unknown seed_nodeset %s", seed_nodeset_name);
					return_code = 0;
				}
				Cmiss_field_module_destroy(&field_module);
			}
			if ((graphic->seed_node_mesh_location_field && (!graphic->seed_nodeset)) ||
				((!graphic->seed_node_mesh_location_field) && graphic->seed_nodeset))
			{
				display_message(ERROR_MESSAGE,
					"Must specify both seed_nodeset and seed_node_mesh_location_field, or neither");
				return_code = 0;
			}
			if (return_code)
			{
				Cmiss_field *stream_vector_field = 0;
				float length = 0.0, width = 0.0;
				int reverse_track_int;
				Cmiss_graphic_get_streamline_parameters(graphic,
					&streamline_type,&stream_vector_field,&reverse_track_int,
					&length,&width);
				STRING_TO_ENUMERATOR(Streamline_type)(streamline_type_string,
					&streamline_type);
				STRING_TO_ENUMERATOR(Streamline_data_type)(
					streamline_data_type_string, &streamline_data_type);
				if (graphic->data_field)
				{
					if (STREAM_FIELD_SCALAR != streamline_data_type)
					{
						display_message(WARNING_MESSAGE,
							"Must use field_scalar option with data; ensuring this");
						streamline_data_type=STREAM_FIELD_SCALAR;
					}
				}
				else
				{
					if (STREAM_FIELD_SCALAR == streamline_data_type)
					{
						display_message(WARNING_MESSAGE,
							"Must specify data field with field_scalar option");
						streamline_data_type=STREAM_NO_DATA;
					}
				}
				if ((STREAM_NO_DATA!=streamline_data_type)&&!graphic->spectrum)
				{
					graphic->spectrum=ACCESS(Spectrum)(
						rendition_command_data->default_spectrum);
				}
				Cmiss_graphic_set_streamline_parameters(
					graphic,streamline_type,stream_vector_field,(int)reverse_track,
					length,width);
				Cmiss_graphic_set_data_spectrum_parameters_streamlines(
					graphic,streamline_data_type,graphic->data_field,
					graphic->spectrum);
			}
		}
	}
	DESTROY(Option_table)(&option_table);
	if (!return_code)
	{
		/* parse error, help */
		DEACCESS(Cmiss_graphic)(&(modify_rendition_data->graphic));
	}
	if (glyph)
	{
		DEACCESS(GT_object)(&glyph);
	}
	if (orientation_scale_field)
	{
		DEACCESS(Computed_field)(&orientation_scale_field);
	}
	if (variable_scale_field)
	{
		DEACCESS(Computed_field)(&variable_scale_field);
	}
	if (xi_point_density_field)
	{
		DEACCESS(Computed_field)(&xi_point_density_field);
	}
	if (font_name)
	{
		DEALLOCATE(font_name);
	}
	if (xi_point_density_field)
	{
		DEACCESS(Computed_field)(&xi_point_density_field);
	}
	if (seed_nodeset_name)
	{
		DEALLOCATE(seed_nodeset_name);
	}
	DEACCESS(Cmiss_graphic)(&graphic);
	return return_code;
}

int gfx_modify_rendition_cylinders(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void)
{
	return gfx_modify_rendition_graphic(state, CMISS_GRAPHIC_CYLINDERS, /*help_text*/(const char *)0,
		reinterpret_cast<Modify_rendition_data *>(modify_rendition_data_void),
		reinterpret_cast<Rendition_command_data *>(rendition_command_data_void));
}

int gfx_modify_rendition_data_points(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void)
{
	return gfx_modify_rendition_graphic(state, CMISS_GRAPHIC_DATA_POINTS, /*help_text*/(const char *)0,
		reinterpret_cast<Modify_rendition_data *>(modify_rendition_data_void),
		reinterpret_cast<Rendition_command_data *>(rendition_command_data_void));
}

int gfx_modify_rendition_element_points(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void)
{
	return gfx_modify_rendition_graphic(state, CMISS_GRAPHIC_ELEMENT_POINTS, /*help_text*/(const char *)0,
		reinterpret_cast<Modify_rendition_data *>(modify_rendition_data_void),
		reinterpret_cast<Rendition_command_data *>(rendition_command_data_void));
}

int gfx_modify_rendition_iso_surfaces(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void)
{
	const char *help_text =
		"Create isosurfaces in volume elements <use_elements> or isolines in face or surface elements <use_faces>.  "
		"The isosurface will be generated at the values in the elements where <iso_scalar> equals the iso values specified.  "
		"The iso values can be specified either as a list with the <iso_values> option "
		"or by specifying <range_number_of_iso_values>, <first_iso_value> and <last_iso_value>.  "
		"The <as> parameter allows a name to be specified for this setting.  "
		"The <coordinate> parameter optionally overrides the groups default coordinate field.  "
		"If a <data> field is specified then the <spectrum> is used to render the data values as colour on the generated isosurface.  "
		"If a <decimation_threshold> is specified then the resulting iso_surface will be decimated according to the threshold.  "
		"If <delete> is specified then if the graphic matches an existing setting (either by parameters or name) then it will be removed.  "
		"If <exterior> is specified then only faces with one parent will be selected when <use_faces> is specified.  "
		"If <face> is specified then only that face will be selected when <use_faces> is specified.  "
		"The <material> is used to render the surface.  "
		"You can specify the <position> the graphic has in the graphic list.  "
		"You can specify the <line_width>, this option only applies when <use_faces> is specified.  "
		"You can render a mesh as solid <render_shaded> or as a wireframe <render_wireframe>.  "
		"If <select_on> is active then the element tool will select the elements the iso_surface was generated from.  "
		"If <no_select> is active then the iso_surface cannot be selected.  "
		"If <draw_selected> is active then iso_surfaces will only be generated in elements that are selected.  "
		"Conversely, if <draw_unselected> is active then iso_surfaces will only be generated in elements that are not selected.  "
		"The <texture_coordinates> are used to lay out a texture if the <material> contains a texture.  "
		"A graphic can be made <visible> or <invisible>.  ";

	return gfx_modify_rendition_graphic(state, CMISS_GRAPHIC_ISO_SURFACES, help_text,
		reinterpret_cast<Modify_rendition_data *>(modify_rendition_data_void),
		reinterpret_cast<Rendition_command_data *>(rendition_command_data_void));
}

int gfx_modify_rendition_lines(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void)
{
	return gfx_modify_rendition_graphic(state, CMISS_GRAPHIC_LINES, /*help_text*/(const char *)0,
		reinterpret_cast<Modify_rendition_data *>(modify_rendition_data_void),
		reinterpret_cast<Rendition_command_data *>(rendition_command_data_void));
}

int gfx_modify_rendition_node_points(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void)
{
	return gfx_modify_rendition_graphic(state, CMISS_GRAPHIC_NODE_POINTS, /*help_text*/(const char *)0,
		reinterpret_cast<Modify_rendition_data *>(modify_rendition_data_void),
		reinterpret_cast<Rendition_command_data *>(rendition_command_data_void));
}

int gfx_modify_rendition_point(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void)
{
	return gfx_modify_rendition_graphic(state, CMISS_GRAPHIC_POINT, /*help_text*/(const char *)0,
		reinterpret_cast<Modify_rendition_data *>(modify_rendition_data_void),
		reinterpret_cast<Rendition_command_data *>(rendition_command_data_void));
}

int gfx_modify_rendition_streamlines(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void)
{
	return gfx_modify_rendition_graphic(state, CMISS_GRAPHIC_STREAMLINES, /*help_text*/(const char *)0,
		reinterpret_cast<Modify_rendition_data *>(modify_rendition_data_void),
		reinterpret_cast<Rendition_command_data *>(rendition_command_data_void));
}

int gfx_modify_rendition_surfaces(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void)
{
	return gfx_modify_rendition_graphic(state, CMISS_GRAPHIC_SURFACES, /*help_text*/(const char *)0,
		reinterpret_cast<Modify_rendition_data *>(modify_rendition_data_void),
		reinterpret_cast<Rendition_command_data *>(rendition_command_data_void));
}

