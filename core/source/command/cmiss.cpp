/***************************************************************************//**
 * cmiss.cpp
 *
 * Functions for executing cmiss commands.
 */
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

#include "zinc/zincconfigure.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined (WIN32_SYSTEM)
#  include <direct.h>
#else /* !defined (WIN32_SYSTEM) */
#  include <unistd.h>
#endif /* !defined (WIN32_SYSTEM) */
#include <math.h>
#include <time.h>

#include "zinc/context.h"
#include "zinc/element.h"
#include "zinc/fieldmodule.h"
#include "zinc/fieldsubobjectgroup.h"
#include "zinc/graphicsmodule.h"
#include "zinc/region.h"
#include "zinc/scene.h"
#include "zinc/sceneviewer.h"
#include "zinc/stream.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_alias.h"
#include "computed_field/computed_field_arithmetic_operators.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_conditional.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_curve.h"
#include "computed_field/computed_field_deformation.h"
#include "computed_field/computed_field_derivatives.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_find_xi_graphics.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_format_output.h"
#include "computed_field/computed_field_function.h"
#include "computed_field/computed_field_group.h"
#include "computed_field/computed_field_image.h"
#include "computed_field/computed_field_integration.h"
#include "computed_field/computed_field_logical_operators.h"
#include "computed_field/computed_field_lookup.h"
#include "computed_field/computed_field_matrix_operators.hpp"
#include "computed_field/computed_field_nodeset_operators.hpp"
#include "computed_field/computed_field_subobject_group_internal.hpp"
#include "computed_field/computed_field_vector_operators.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_string_constant.h"
#include "computed_field/computed_field_time.h"
#include "computed_field/computed_field_trigonometry.h"
#include "computed_field/computed_field_update.h"
#include "computed_field/computed_field_scene_viewer_projection.h"
#include "computed_field/computed_field_wrappers.h"
#include "context/context.h"
#include "element/element_operations.h"
#include "field_io/read_fieldml.h"
#include "finite_element/export_cm_files.h"
#if defined (USE_NETGEN)
#include "finite_element/generate_mesh_netgen.h"
#endif /* defined (USE_NETGEN) */
#include "finite_element/export_finite_element.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_conversion.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_to_graphics_object.h"
#include "finite_element/finite_element_to_iges.h"
#include "finite_element/finite_element_to_iso_lines.h"
#include "finite_element/finite_element_to_streamlines.h"
#include "finite_element/import_finite_element.h"
#include "finite_element/snake.h"
#include "general/debug.h"
#include "general/error_handler.h"
#include "general/image_utilities.h"
#include "general/io_stream.h"
#include "general/matrix_vector.h"
#include "general/multi_range.h"
#include "general/mystring.h"
#include "general/message.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/environment_map.h"
#include "graphics/glyph.hpp"
#include "graphics/glyph_colour_bar.hpp"
#include "graphics/graphics_object.h"
#include "graphics/import_graphics_object.h"
#include "graphics/iso_field_calculation.h"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "graphics/material.h"
#include "graphics/graphic.h"
#include "graphics/scene.h"
#include "graphics/render_to_finite_elements.h"
#include "graphics/render_stl.h"
#include "graphics/render_vrml.h"
#include "graphics/render_wavefront.h"
#include "graphics/graphics_module.h"
#include "finite_element/finite_element_helper.h"
#include "graphics/triangle_mesh.hpp"
#include "graphics/render_triangularisation.hpp"
#include "graphics/graphics_filter.hpp"
#include "graphics/tessellation.hpp"
#include "graphics/spectrum.h"
#include "graphics/spectrum_component.h"
#include "graphics/texture.h"
#include "graphics/userdef_objects.h"
#include "graphics/volume_texture.h"
#include "image_processing/computed_field_image_resample.h"
#if defined (USE_ITK)
#include "image_processing/computed_field_threshold_image_filter.h"
#include "image_processing/computed_field_binary_threshold_image_filter.h"
#include "image_processing/computed_field_canny_edge_detection_filter.h"
#include "image_processing/computed_field_mean_image_filter.h"
#include "image_processing/computed_field_sigmoid_image_filter.h"
#include "image_processing/computed_field_discrete_gaussian_image_filter.h"
#include "image_processing/computed_field_curvature_anisotropic_diffusion_image_filter.h"
#include "image_processing/computed_field_derivative_image_filter.h"
#include "image_processing/computed_field_rescale_intensity_image_filter.h"
#include "image_processing/computed_field_connected_threshold_image_filter.h"
#include "image_processing/computed_field_gradient_magnitude_recursive_gaussian_image_filter.h"
#include "image_processing/computed_field_histogram_image_filter.h"
#include "image_processing/computed_field_fast_marching_image_filter.h"
#include "image_processing/computed_field_binary_dilate_image_filter.h"
#include "image_processing/computed_field_binary_erode_image_filter.h"
#endif /* defined (USE_ITK) */
#if defined (SELECT_DESCRIPTORS)
#include "io_devices/io_device.h"
#endif /* !defined (SELECT_DESCRIPTORS) */
#include "minimise/minimise.h"
#include "node/node_operations.h"
#include "region/cmiss_region.h"
#include "selection/any_object_selection.h"
#include "graphics/font.h"
#include "time/time_keeper.hpp"
#include "curve/curve.h"
#include "mesh/cmiss_element_private.hpp"
#include "mesh/cmiss_node_private.hpp"
#if defined (USE_OPENCASCADE)
#include "cad/graphicimporter.h"
#include "cad/point.h"
#include "cad/curve.h"
#include "cad/surface.h"
#include "cad/geometric_shape.h"
#include "graphics/graphics_object.hpp"
#include "cad/opencascade_importer.h"
#include "cad/computed_field_cad_topology.h"
#endif /* defined (USE_OPENCASCADE) */

/*
Module functions
----------------
*/

/***************************************************************************//**
 * Adds or removes elements to/from group.
 * @param manage_nodes  Set if nodes are added/removed with elements. Nodes are
 * only removed if not in use by any other elements in group.
 * @param manage_faces  Set if faces are added/removed with parent elements.
 * Faces are only removed if not in use by any other elements in group.
 */
int process_modify_element_group(cmzn_field_group_id group,
	cmzn_region_id region, int dimension, char add_flag,
	cmzn_field_id conditional_field, cmzn_field_group_id from_group,
	Multi_range *element_ranges, char selected_flag, FE_value time,
	int manage_nodes, int manage_faces)
{
	if (!group || !region)
		return 0;
	int return_code = 1;
	cmzn_field_module_id field_module = cmzn_region_get_field_module(region);
	cmzn_mesh_id master_mesh = cmzn_field_module_find_mesh_by_dimension(field_module, dimension);
	char remove_flag = !add_flag;
	cmzn_mesh_group_id selection_mesh_group = 0;
	if (selected_flag)
	{
		cmzn_scene *scene = cmzn_region_get_scene_private(region);
		cmzn_field_group_id selection_group = cmzn_scene_get_selection_group(scene);
		if (selection_group)
		{
			cmzn_field_element_group_id selection_element_group =
				cmzn_field_group_get_element_group(selection_group, master_mesh);
			if (selection_element_group)
			{
				selection_mesh_group = cmzn_field_element_group_get_mesh(selection_element_group);
				cmzn_field_element_group_destroy(&selection_element_group);
			}
			cmzn_field_group_destroy(&selection_group);
		}
	}
	cmzn_mesh_group_id from_mesh_group = 0;
	if (from_group)
	{
		cmzn_field_element_group_id from_element_group =
			cmzn_field_group_get_element_group(from_group, master_mesh);
		if (from_element_group)
		{
			from_mesh_group = cmzn_field_element_group_get_mesh(from_element_group);
			cmzn_field_element_group_destroy(&from_element_group);
		}

	}
	int objects_processed = 0;
	int elements_not_processed = 0;
	if (((!selected_flag) || selection_mesh_group) && ((!from_group) || from_mesh_group))
	{
		cmzn_field_module_begin_change(field_module);
		cmzn_field_element_group_id modify_element_group = cmzn_field_group_get_element_group(group, master_mesh);
		if (!modify_element_group)
			modify_element_group = cmzn_field_group_create_element_group(group, master_mesh);
		cmzn_mesh_group_id modify_mesh_group = cmzn_field_element_group_get_mesh(modify_element_group);
		objects_processed += cmzn_mesh_get_size(cmzn_mesh_group_base_cast(modify_mesh_group));
		cmzn_field_element_group_destroy(&modify_element_group);
		cmzn_field_cache_id cache = cmzn_field_module_create_cache(field_module);
		cmzn_field_cache_set_time(cache, time);

		cmzn_nodeset_group_id modify_nodeset_group = 0;
		cmzn_field_node_group_id remove_node_group = 0;
		cmzn_nodeset_group_id remove_nodeset_group = 0;
		if (manage_nodes)
		{
			cmzn_nodeset_id master_nodeset = cmzn_field_module_find_nodeset_by_domain_type(field_module, CMISS_FIELD_DOMAIN_NODES);
			cmzn_field_node_group_id modify_node_group = cmzn_field_group_get_node_group(group, master_nodeset);
			if ((!modify_node_group) && add_flag)
				modify_node_group = cmzn_field_group_create_node_group(group, master_nodeset);
			if (modify_node_group)
			{
				modify_nodeset_group = cmzn_field_node_group_get_nodeset(modify_node_group);
				objects_processed += cmzn_nodeset_get_size(cmzn_nodeset_group_base_cast(modify_nodeset_group));
				if (remove_flag)
				{
					cmzn_field_id remove_node_group_field = cmzn_field_module_create_node_group(field_module, master_nodeset);
					remove_node_group = cmzn_field_cast_node_group(remove_node_group_field);
					remove_nodeset_group = cmzn_field_node_group_get_nodeset(remove_node_group);
					cmzn_field_destroy(&remove_node_group_field);
				}
				cmzn_field_node_group_destroy(&modify_node_group);
			}
			cmzn_nodeset_destroy(&master_nodeset);
		}
		cmzn_nodeset_group_id working_nodeset_group = add_flag ? modify_nodeset_group : remove_nodeset_group;

		cmzn_mesh_id master_face_mesh = 0;
		cmzn_mesh_group_id modify_face_mesh_group = 0;
		cmzn_field_element_group_id working_face_element_group = 0;
		cmzn_mesh_group_id working_face_mesh_group = 0;
		if (manage_faces && (1 < dimension))
		{
			master_face_mesh = cmzn_field_module_find_mesh_by_dimension(field_module, dimension - 1);
			cmzn_field_element_group_id modify_face_element_group = cmzn_field_group_get_element_group(group, master_face_mesh);
			if ((!modify_face_element_group) && add_flag)
				modify_face_element_group = cmzn_field_group_create_element_group(group, master_face_mesh);
			if (modify_face_element_group)
			{
				modify_face_mesh_group = cmzn_field_element_group_get_mesh(modify_face_element_group);
				objects_processed += cmzn_mesh_get_size(cmzn_mesh_group_base_cast(modify_face_mesh_group));
				cmzn_field_id working_face_element_group_field = cmzn_field_module_create_element_group(field_module, master_face_mesh);
				working_face_element_group = cmzn_field_cast_element_group(working_face_element_group_field);
				working_face_mesh_group = cmzn_field_element_group_get_mesh(working_face_element_group);
				cmzn_field_destroy(&working_face_element_group_field);
				cmzn_field_element_group_destroy(&modify_face_element_group);
			}
		}

		cmzn_mesh_id iteration_mesh = master_mesh;
		cmzn_mesh_id selection_mesh = cmzn_mesh_group_base_cast(selection_mesh_group);
		cmzn_mesh_id from_mesh = cmzn_mesh_group_base_cast(from_mesh_group);
		if (selected_flag && selection_mesh && !cmzn_mesh_match(selection_mesh, cmzn_mesh_group_base_cast(modify_mesh_group)))
		{
			iteration_mesh = selection_mesh;
		}
		if (from_mesh && (!cmzn_mesh_match(from_mesh, cmzn_mesh_group_base_cast(modify_mesh_group))) &&
			(cmzn_mesh_get_size(from_mesh) < cmzn_mesh_get_size(iteration_mesh)))
		{
			iteration_mesh = from_mesh;
		}
		cmzn_element_iterator_id iter = cmzn_mesh_create_element_iterator(iteration_mesh);
		cmzn_element_id element = 0;
		while (NULL != (element = cmzn_element_iterator_next_non_access(iter)))
		{
			if (element_ranges && !Multi_range_is_value_in_range(element_ranges, cmzn_element_get_identifier(element)))
				continue;
			if (selection_mesh && (selection_mesh != iteration_mesh) && !cmzn_mesh_contains_element(selection_mesh, element))
				continue;
			if (from_mesh && (from_mesh != iteration_mesh) && !cmzn_mesh_contains_element(from_mesh, element))
				continue;
			if (conditional_field)
			{
				cmzn_field_cache_set_element(cache, element);
				if (!cmzn_field_evaluate_boolean(conditional_field, cache))
					continue;
			}
			if (add_flag)
			{
				if (!cmzn_mesh_contains_element(cmzn_mesh_group_base_cast(modify_mesh_group), element))
				{
					if (!cmzn_mesh_group_add_element(modify_mesh_group, element))
					{
						display_message(ERROR_MESSAGE,
							"gfx modify egroup:  Could not add element %d", cmzn_element_get_identifier(element));
						return_code = 0;
						break;
					}
				}
			}
			else
			{
				if (cmzn_mesh_contains_element(cmzn_mesh_group_base_cast(modify_mesh_group), element))
				{
					if (!cmzn_mesh_group_remove_element(modify_mesh_group, element))
					{
						display_message(ERROR_MESSAGE,
							"gfx modify egroup:  Could not remove element %d", cmzn_element_get_identifier(element));
						return_code = 0;
						break;
					}
				}
			}
			if (working_nodeset_group)
			{
				cmzn_nodeset_group_add_element_nodes(working_nodeset_group, element);
			}
			if (working_face_mesh_group)
			{
				cmzn_mesh_group_add_element_faces(working_face_mesh_group, element);
			}
		}
		cmzn_element_iterator_destroy(&iter);

		if (remove_flag && (remove_nodeset_group || working_face_mesh_group))
		{
			// don't include faces and nodes still used by elements remaining in modify_mesh_group
			// Note: ignores nodes for other dimensions
			iter = cmzn_mesh_create_element_iterator(cmzn_mesh_group_base_cast(modify_mesh_group));
				while (NULL != (element = cmzn_element_iterator_next_non_access(iter)))
				{
				if (remove_nodeset_group)
				{
					cmzn_nodeset_group_remove_element_nodes(remove_nodeset_group, element);
				}
				if (working_face_mesh_group)
				{
					cmzn_mesh_group_remove_element_faces(working_face_mesh_group, element);
				}
				}
				cmzn_element_iterator_destroy(&iter);
				if (remove_nodeset_group)
				{
				cmzn_nodeset_group_remove_nodes_conditional(modify_nodeset_group,
					cmzn_field_node_group_base_cast(remove_node_group));
				}
		}
		if (working_face_mesh_group)
		{
			cmzn_mesh_group_id modify_line_mesh_group = 0;
			cmzn_field_element_group_id remove_line_element_group = 0;
			cmzn_mesh_group_id remove_line_mesh_group = 0;
			if (2 < dimension)
			{
				cmzn_mesh_id master_line_mesh = cmzn_field_module_find_mesh_by_dimension(field_module, dimension - 2);
				cmzn_field_element_group_id modify_line_element_group = cmzn_field_group_get_element_group(group, master_line_mesh);
				if (add_flag && !modify_line_element_group)
					modify_line_element_group = cmzn_field_group_create_element_group(group, master_line_mesh);
				if (modify_line_element_group)
				{
					modify_line_mesh_group = cmzn_field_element_group_get_mesh(modify_line_element_group);
					objects_processed += cmzn_mesh_get_size(cmzn_mesh_group_base_cast(modify_line_mesh_group));
					if (remove_flag)
					{
						cmzn_field_id remove_line_element_group_field = cmzn_field_module_create_element_group(field_module, master_line_mesh);
						remove_line_element_group = cmzn_field_cast_element_group(remove_line_element_group_field);
						remove_line_mesh_group = cmzn_field_element_group_get_mesh(remove_line_element_group);
						cmzn_field_destroy(&remove_line_element_group_field);
					}
					cmzn_field_element_group_destroy(&modify_line_element_group);
				}
				cmzn_mesh_destroy(&master_line_mesh);
			}
			cmzn_mesh_group_id working_line_mesh_group = add_flag ? modify_line_mesh_group : remove_line_mesh_group;
			iter = cmzn_mesh_create_element_iterator(cmzn_mesh_group_base_cast(working_face_mesh_group));
			while (NULL != (element = cmzn_element_iterator_next_non_access(iter)))
			{
				if (add_flag)
				{
					cmzn_mesh_group_add_element(modify_face_mesh_group, element);
				}
				else
				{
					cmzn_mesh_group_remove_element(modify_face_mesh_group, element);
				}
				if (working_line_mesh_group)
				{
					cmzn_mesh_group_add_element_faces(working_line_mesh_group, element);
				}
			}
			cmzn_element_iterator_destroy(&iter);
			if (remove_line_element_group)
			{
				cmzn_mesh_group_remove_elements_conditional(modify_line_mesh_group,
					cmzn_field_element_group_base_cast(remove_line_element_group));
			}
			cmzn_mesh_group_destroy(&remove_line_mesh_group);
			cmzn_field_element_group_destroy(&remove_line_element_group);
			if (modify_line_mesh_group)
			{
				objects_processed -= cmzn_mesh_get_size(cmzn_mesh_group_base_cast(modify_line_mesh_group));
				cmzn_mesh_group_destroy(&modify_line_mesh_group);
			}
		}

		cmzn_mesh_group_destroy(&working_face_mesh_group);
		cmzn_field_element_group_destroy(&working_face_element_group);
		if (modify_face_mesh_group)
		{
			objects_processed -= cmzn_mesh_get_size(cmzn_mesh_group_base_cast(modify_face_mesh_group));
			cmzn_mesh_group_destroy(&modify_face_mesh_group);
		}
		cmzn_mesh_destroy(&master_face_mesh);
		cmzn_nodeset_group_destroy(&remove_nodeset_group);
		cmzn_field_node_group_destroy(&remove_node_group);
		if (modify_nodeset_group)
		{
			objects_processed -= cmzn_nodeset_get_size(cmzn_nodeset_group_base_cast(modify_nodeset_group));
			cmzn_nodeset_group_destroy(&modify_nodeset_group);
		}
		cmzn_field_cache_destroy(&cache);
		objects_processed -= cmzn_mesh_get_size(cmzn_mesh_group_base_cast(modify_mesh_group));
		cmzn_mesh_group_destroy(&modify_mesh_group);
		cmzn_field_module_end_change(field_module);
	}
	if (0 < elements_not_processed)
	{
		display_message(WARNING_MESSAGE,
			"gfx modify egroup:  %d elements could not be removed", elements_not_processed);
	}
	else if (0 == objects_processed)
	{
		display_message(WARNING_MESSAGE, "gfx modify egroup:  group unchanged");
	}
	cmzn_mesh_group_destroy(&from_mesh_group);
	cmzn_mesh_group_destroy(&selection_mesh_group);
	cmzn_mesh_destroy(&master_mesh);
	cmzn_field_module_destroy(&field_module);

	return return_code;
}

int set_Texture_image_from_field(struct Texture *texture,
	struct Computed_field *field,
	struct Computed_field *texture_coordinate_field,
	int propagate_field,
	struct Spectrum *spectrum,
	cmzn_mesh_id search_mesh,
	enum Texture_storage_type storage,
	int image_width, int image_height, int image_depth,
	int number_of_bytes_per_component,
	struct Graphics_buffer_package *graphics_buffer_package,
	struct Graphical_material *fail_material)
/*******************************************************************************
LAST MODIFIED : 30 June 2006

DESCRIPTION :
Creates the image in the format given by sampling the <field> according to the
reverse mapping of the <texture_coordinate_field>.  The values returned by
field are converted to "colours" by applying the <spectrum>.
Currently limited to 1 byte per component.
@param search_mesh  The mesh to find locations with matching texture coordinates.
==============================================================================*/
{
	char *field_name;
	int bytes_per_pixel, number_of_components, return_code,
		source_dimension, *source_sizes, tex_number_of_components, use_pixel_location = 1;
	struct Computed_field *source_texture_coordinate_field = NULL;

	ENTER(set_Texture_image_from_field);
	if (texture && field && spectrum &&
		(4 >= (number_of_components =
			Texture_storage_type_get_number_of_components(storage))))
	{
		/* Setup sizes */
		if (Computed_field_get_native_resolution(
			field, &source_dimension, &source_sizes,
			&source_texture_coordinate_field))
		{
			if (!texture_coordinate_field)
			{
				texture_coordinate_field =
					source_texture_coordinate_field;
			}
			if (image_width == 0)
			{
				if (source_dimension > 0)
				{
					image_width = source_sizes[0];
				}
				else
				{
					image_width = 1;
				}
			}
			if (image_height == 0)
			{
				if (source_dimension > 1)
				{
					image_height = source_sizes[1];
				}
				else
				{
					image_height = 1;
				}
			}
			if (image_depth == 0)
			{
				if (source_dimension > 2)
				{
					image_depth = source_sizes[2];
				}
				else
				{
					image_depth = 1;
				}
			}
			DEALLOCATE(source_sizes);
		}

		if (texture_coordinate_field &&
			(3 >= (tex_number_of_components =
			Computed_field_get_number_of_components(texture_coordinate_field))))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Texture_image_from_field.  Invalid texture_coordinate field.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Texture_image_from_field.  Invalid argument(s)");
		return_code = 0;
	}
	if (return_code)
	{
		if (number_of_bytes_per_component <= 0)
		{
			 number_of_bytes_per_component = 1;
		}
		/* allocate the texture image */
		use_pixel_location = (texture_coordinate_field == source_texture_coordinate_field);
		field_name = (char *)NULL;
		GET_NAME(Computed_field)(field, &field_name);
		if (Texture_allocate_image(texture, image_width, image_height,
			image_depth, storage, number_of_bytes_per_component, field_name))
		{
			bytes_per_pixel = number_of_components*number_of_bytes_per_component;
			Set_cmiss_field_value_to_texture(field, texture_coordinate_field,
				texture, spectrum,	fail_material, image_height, image_width, image_depth,
				bytes_per_pixel, number_of_bytes_per_component, use_pixel_location,
				storage,propagate_field,	graphics_buffer_package, search_mesh);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Texture_image_from_field.  Could not allocate image in texture");
			return_code = 0;
		}
		DEALLOCATE(field_name);
	}
	LEAVE;

	return (return_code);
} /* set_Texture_image_from_field */

struct Texture_evaluate_image_data
{
	cmzn_region_id region;
	cmzn_field_group_id group;
	char *field_name, *texture_coordinates_field_name;
	int element_dimension; /* where 0 is any dimension */
	int propagate_field;
	struct Computed_field *field, *texture_coordinates_field;
	struct Graphical_material *fail_material;
	struct Spectrum *spectrum;
};

struct Texture_image_data
{
	char *image_file_name;
	int crop_bottom_margin,crop_left_margin,crop_height,crop_width;
};

struct Texture_file_number_series_data
{
	int start, stop, increment;
};

struct Apply_transformation_data
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Data for applying transformation.  Required because the data list of nodes is
not in the same region as the region returned from FE_node_get_FE_region which
is the region where the fields are defined (the parent region in this case).
==============================================================================*/
{
	gtMatrix transformation;
	struct FE_region *fe_region;
}; /* struct Apply_transformation_data */

int apply_transformation_to_node(struct FE_node *node,
	void *data_void)
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Iterator that modifies the position of each node according to the
transformation in the transformation data.
Should enclose multiple calls in FE_region_begin_change/end_change wrappers.
==============================================================================*/
{
	FE_value x, x2, y, y2, z, z2, h2;
	int return_code;
	struct Apply_transformation_data *data;

	ENTER(apply_transformation_to_node);
	return_code = 0;
	if (node && (data = (struct Apply_transformation_data  *)data_void))
	{
		FE_field *coordinate_field = get_FE_node_default_coordinate_field(node);
		if (FE_node_get_position_cartesian(node, coordinate_field,
			&x, &y, &z, (FE_value *)NULL))
		{
			/* Get the new position */
			h2 = (data->transformation)[0][3] * x
				+ (data->transformation)[1][3] * y
				+ (data->transformation)[2][3] * z
				+ (data->transformation)[3][3];
			x2 = ((data->transformation)[0][0] * x
				+ (data->transformation)[1][0] * y
				+ (data->transformation)[2][0] * z
				+ (data->transformation)[3][0]) / h2;
			y2 = ((data->transformation)[0][1] * x
				+ (data->transformation)[1][1] * y
				+ (data->transformation)[2][1] * z
				+ (data->transformation)[3][1]) / h2;
			z2 = ((data->transformation)[0][2] * x
				+ (data->transformation)[1][2] * y
				+ (data->transformation)[2][2] * z
				+ (data->transformation)[3][2]) / h2;

			if (FE_node_set_position_cartesian(node,coordinate_field,x2,y2,z2))
			{
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"apply_transformation_to_node.  Could not move node");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"apply_transformation_to_node.  Could not calculate coordinate field");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"apply_transformation_to_node.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* apply_transformation_to_node */

#if defined (USE_OPENCASCADE)
static struct cmzn_spectrum_component *create_spectrum_component( cmzn_spectrum_component_colour_mapping colour )
{
	int component = 1;
	struct cmzn_spectrum_component *settings = CREATE(cmzn_spectrum_component)();
	cmzn_spectrum_component_set_scale_type(settings, CMISS_SPECTRUM_COMPONENT_SCALE_LINEAR);
	cmzn_spectrum_component_set_colour_mapping(settings, colour);
	cmzn_spectrum_component_set_extend_above(settings, true);
	cmzn_spectrum_component_set_extend_below_flag(settings, true);
	cmzn_spectrum_component_set_colour_reverse(settings, 0);

	if ( colour == CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_RED )
		component = 1;
	else if ( colour == CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_GREEN )
		component = 2;
	else
		component = 3;

	cmzn_spectrum_component_set_field_component( settings, component );

	return settings;
}

static int create_RGB_spectrum( struct Spectrum **spectrum, void *command_data_void )
{
	int return_code = 0, number_in_list = 0;
	struct LIST(cmzn_spectrum_component) *spectrum_settings_list;
	struct cmzn_spectrum_component *red_settings;
	struct cmzn_spectrum_component *green_settings;
	struct cmzn_spectrum_component *blue_settings;
	struct cmzn_command_data *command_data = (struct cmzn_command_data *)command_data_void;

	if ( command_data && (NULL != ((*spectrum) = cmzn_spectrum_create_private())) &&
		cmzn_spectrum_set_name(spectrum, "RGB"))
		spectrum_settings_list = get_cmzn_spectrum_component_list( (*spectrum) );
		number_in_list = NUMBER_IN_LIST(cmzn_spectrum_component)(spectrum_settings_list);
		if ( number_in_list > 0 )
		{
			REMOVE_ALL_OBJECTS_FROM_LIST(cmzn_spectrum_component)(spectrum_settings_list);
		}
		red_settings = create_spectrum_component( CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_RED );
		cmzn_spectrum_component_add( red_settings, /* end of list = 0 */0,
			spectrum_settings_list );

		green_settings = create_spectrum_component( CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_GREEN );
		cmzn_spectrum_component_add( green_settings, /* end of list = 0 */0,
			spectrum_settings_list );

		blue_settings = create_spectrum_component( CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_BLUE );
		cmzn_spectrum_component_add( blue_settings, /* end of list = 0 */0,
			spectrum_settings_list );

		Spectrum_calculate_range( (*spectrum) );
		Spectrum_calculate_range( (*spectrum) );
		Spectrum_set_minimum_and_maximum( (*spectrum), 0, 1);
		cmzn_spectrum_set_material_overwrite( (*spectrum), 0 );
		if (!ADD_OBJECT_TO_MANAGER(Spectrum)( (*spectrum),
				command_data->spectrum_manager))
		{
			DEACCESS(Spectrum)(spectrum);
			//DEACCESS(Spectrum)(&(command_data->default_spectrum));
		}
		else
		{
			return_code = 1;
		}
	}
	return return_code;
}

#endif /* USE_OPENCASCADE */

void create_triangle_mesh(struct cmzn_region *region, Triangle_mesh *trimesh)
{
	struct FE_region *fe_region = cmzn_region_get_FE_region(region);
	// for efficiency cache changes until all finished
	FE_region_begin_change(fe_region);

	// create a 3-D coordinate field
	FE_field *coordinate_field = FE_field_create_coordinate_3d(fe_region, "coordinates");

	const Mesh_triangle_list  triangle_list = trimesh->get_triangle_list();
	Mesh_triangle_list_const_iterator triangle_iter;
	const Triangle_vertex_set vertex_set = trimesh->get_vertex_set();
	Triangle_vertex_set_const_iterator vertex_iter;
	// create template node with 3-D coordinate field parameters
	struct FE_node *template_node;
	int identifier = 1;
	struct FE_node *node;
	int number_of_values_confirmed;
	FE_value coordinates[3];
	ZnReal coord1, coord2, coord3;
	int initial_identifier = FE_region_get_last_FE_node_identifier(fe_region);
	int i = 0;
	for (vertex_iter = vertex_set.begin(); vertex_iter!=vertex_set.end(); ++vertex_iter)
	{
		identifier = initial_identifier+(*vertex_iter)->get_identifier();
		(*vertex_iter)->get_coordinates(&coord1, &coord2, &coord3);
		template_node = CREATE(FE_node)(/*cm_node_identifier*/identifier, fe_region, /*template_node*/NULL);
		define_FE_field_at_node_simple(template_node, coordinate_field, /*number_of_derivatives*/0, /*derivative_value_types*/NULL);

		// create a node from the template
		coordinates[0] = coord1;
		coordinates[1] = coord2;
		coordinates[2] = coord3;
		node = CREATE(FE_node)(identifier, /*fe_region*/NULL, template_node);
		ACCESS(FE_node)(node);
		set_FE_nodal_field_FE_value_values(coordinate_field, node, coordinates, &number_of_values_confirmed);
		FE_region_merge_FE_node(fe_region, node);
		DEACCESS(FE_node)(&node);

		DESTROY(FE_node)(&template_node);
		i++;
	}
	// establish mode which automates creation of shared faces
	FE_region_begin_define_faces(fe_region, /*all dimensions*/-1);

	struct CM_element_information element_identifier;
	FE_element *element;
	FE_element *template_element;
	const Triangle_vertex *vertex1, *vertex2, *vertex3;

	// create a triangle template element with linear simplex field
	for (triangle_iter = triangle_list.begin(); triangle_iter!=triangle_list.end(); ++triangle_iter)
	{
		(*triangle_iter)->get_vertexes(&vertex1, &vertex2, &vertex3);
		template_element = FE_element_create_with_simplex_shape(fe_region, /*dimension*/2);
		set_FE_element_number_of_nodes(template_element, 3);
		FE_element_define_field_simple(template_element, coordinate_field, LINEAR_SIMPLEX);

		// make an element based on the template & fill node list
		element_identifier.type = CM_ELEMENT;
		element_identifier.number = FE_region_get_next_FE_element_identifier(fe_region, /*dimension*/2, 1);
		element = CREATE(FE_element)(&element_identifier, (struct FE_element_shape *)NULL,
			(struct FE_region *)NULL, template_element);
		ACCESS(FE_element)(element);
		set_FE_element_node(element, 0, FE_region_get_FE_node_from_identifier(fe_region,initial_identifier+vertex1->get_identifier()));
		set_FE_element_node(element, 1, FE_region_get_FE_node_from_identifier(fe_region,initial_identifier+vertex2->get_identifier()));
		set_FE_element_node(element, 2, FE_region_get_FE_node_from_identifier(fe_region,initial_identifier+vertex3->get_identifier()));
		FE_region_merge_FE_element_and_faces_and_nodes(fe_region, element);

		DEACCESS(FE_element)(&element);

	DEACCESS(FE_element)(&template_element);
	}
	// must remember to end define faces mode
	FE_region_end_define_faces(fe_region);

	DEACCESS(FE_field)(&coordinate_field);

	FE_region_end_change(fe_region);
}
