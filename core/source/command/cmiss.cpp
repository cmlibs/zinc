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

#include "configure/cmiss_zinc_configure.h"

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

extern "C" {
#include "api/cmiss_context.h"
#include "api/cmiss_element.h"
#include "api/cmiss_field_module.h"
#include "api/cmiss_field_subobject_group.h"
#include "api/cmiss_graphics_module.h"
#include "api/cmiss_region.h"
#include "api/cmiss_rendition.h"
#include "api/cmiss_scene.h"
#include "api/cmiss_scene_viewer.h"
#include "api/cmiss_stream.h"
#include "command/console.h"
#include "command/example_path.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_alias.h"
#include "computed_field/computed_field_arithmetic_operators.h"
#include "computed_field/computed_field_compose.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_conditional.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_curve.h"
#include "computed_field/computed_field_deformation.h"
#include "computed_field/computed_field_derivatives.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_find_xi_graphics.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_fibres.h"
#include "computed_field/computed_field_format_output.h"
#include "computed_field/computed_field_function.h"
#include "computed_field/computed_field_group.h"
#include "computed_field/computed_field_image.h"
#include "computed_field/computed_field_integration.h"
#include "computed_field/computed_field_logical_operators.h"
#include "computed_field/computed_field_lookup.h"
}
#include "computed_field/computed_field_matrix_operators.hpp"
#include "computed_field/computed_field_nodeset_operators.hpp"
#include "computed_field/computed_field_subobject_group_internal.hpp"
#include "computed_field/computed_field_vector_operators.hpp"
extern "C" {
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_string_constant.h"
#include "computed_field/computed_field_time.h"
#include "computed_field/computed_field_trigonometry.h"
#include "computed_field/computed_field_update.h"
#include "computed_field/computed_field_scene_viewer_projection.h"
#include "computed_field/computed_field_wrappers.h"
#include "context/context.h"
#include "element/element_operations.h"
#include "emoter/emoter_dialog.h"
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
#include "finite_element/read_fieldml_01.h"
#include "finite_element/snake.h"
#include "finite_element/write_fieldml_01.h"
#include "general/debug.h"
#include "general/error_handler.h"
#include "general/image_utilities.h"
#include "general/io_stream.h"
#include "general/matrix_vector.h"
#include "general/multi_range.h"
#include "general/mystring.h"
#include "general/message.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/defined_graphics_objects.h"
#include "graphics/environment_map.h"
#include "graphics/glyph.h"
#include "graphics/graphics_object.h"
#include "graphics/import_graphics_object.h"
#include "graphics/iso_field_calculation.h"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "graphics/material.h"
#include "graphics/graphic.h"
#include "graphics/graphics_module.h"
#include "graphics/rendition.h"
#include "graphics/render_to_finite_elements.h"
#include "graphics/render_stl.h"
#include "graphics/render_vrml.h"
#include "graphics/render_wavefront.h"
#include "graphics/scene.h"
#include "finite_element/finite_element_helper.h"
}
#include "graphics/triangle_mesh.hpp"
#include "graphics/render_triangularisation.hpp"
#include "graphics/scene.hpp"
#include "graphics/graphics_filter.hpp"
#include "graphics/tessellation.hpp"
extern "C" {
#include "graphics/spectrum.h"
#include "graphics/spectrum_settings.h"
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
#include "three_d_drawing/graphics_buffer.h"
#include "graphics/font.h"
#include "time/time_keeper.h"
#include "curve/curve.h"
#include "command/cmiss.h"
}
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
Module types
------------
*/


struct Cmiss_command_data
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
==============================================================================*/
{
	int access_count;
	char *cm_examples_directory,*cm_parameters_file_name,*example_directory,
		*examples_directory,*example_requirements,*help_directory,*help_url;
	struct Colour background_colour,foreground_colour;
	struct Execute_command *execute_command,*set_command;
	struct Event_dispatcher *event_dispatcher;
#if defined (SELECT_DESCRIPTORS)
	struct LIST(Io_device) *device_list;
#endif /* defined (SELECT_DESCRIPTORS) */
	/* list of glyphs = simple graphics objects with only geometry */
	struct MANAGER(GT_object) *glyph_manager;
	struct Cmiss_region *root_region;
	struct Computed_field_package *computed_field_package;
	struct MANAGER(Environment_map) *environment_map_manager;
	struct MANAGER(FE_basis) *basis_manager;
	struct LIST(FE_element_shape) *element_shape_list;
	/* Always want the entry for graphics_buffer_package even if it will
		not be available on this implementation */
	struct Graphics_buffer_package *graphics_buffer_package;
	struct Cmiss_scene_viewer_package *scene_viewer_package;
	struct Graphics_font_package *graphics_font_package;
	struct IO_stream_package *io_stream_package;
	struct MANAGER(Light) *light_manager;
	struct Light *default_light;
	struct MANAGER(Light_model) *light_model_manager;
	struct Light_model *default_light_model;
	struct Material_package *material_package;
	struct Graphics_font *default_font;
	struct MANAGER(Curve) *curve_manager;
	struct MANAGER(Scene) *scene_manager;
	struct Scene *default_scene;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;
	/* global list of selected objects */
	struct Any_object_selection *any_object_selection;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct Spectrum *default_spectrum;
	struct Streampoint *streampoint_list;
	struct Time_keeper *default_time_keeper;
	struct User_interface *user_interface;
	struct Emoter_dialog *emoter_slider_dialog;
	struct Cmiss_graphics_module *graphics_module;
}; /* struct Cmiss_command_data */

typedef struct
/*******************************************************************************
LAST MODIFIED : 12 December 1996+

DESCRIPTION :
==============================================================================*/
{
	char *examples_directory,*help_directory,*help_url,*startup_comfile;
} User_settings;

/*
Module functions
----------------
*/

#if defined (WX_USER_INTERFACE)
static int Graphics_window_update_Interactive_tool(struct Graphics_window *graphics_window,
	void *interactive_tool_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2007

DESCRIPTION :
WX_USER_INTERFACE_ONLY, get the interactive_tool_manager and pass it
to change the interactive tool settings.
==============================================================================*/
{
	char *tool_name;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct Interactive_tool *global_interactive_tool;
	struct Interactive_tool *wx_interactive_tool;
	global_interactive_tool = (struct Interactive_tool *)interactive_tool_void;
	GET_NAME(Interactive_tool)(global_interactive_tool,&tool_name);
	interactive_tool_manager = Graphics_window_get_interactive_tool_manager(graphics_window);
	if (NULL != (wx_interactive_tool=
		FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(
		(char *)tool_name,interactive_tool_manager)))
	{
		Interactive_tool_copy(wx_interactive_tool,
			global_interactive_tool, (struct MANAGER(Interactive_tool) *)NULL);
	}
    DEALLOCATE(tool_name);
	return 1;
}
#endif /*(WX_USER_INTERFACE)*/

#if defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM)
char *CMISS_set_directory_and_filename_WIN32(char *file_name,
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 27 March 2007

DESCRIPTION :
WX_USER_INTERFACE_ONLY, get the interactive_tool_manager and pass it
to change the interactive tool settings.
==============================================================================*/
{
	 char *drive_name = NULL;
	 char *first = NULL;
	 char *last = NULL;
	 char *temp_directory_name,*directory_name, *temp_name, *temp_string;
	 int lastlength, length;
	 first = strchr(file_name, '\\');
	 last = strrchr(file_name, '\\');
	 lastlength = last - file_name +1;
	 length = first - file_name +1;
	 if ((length>0))
	 {
			if (ALLOCATE(drive_name,char,length))
			{		 
				 strncpy(drive_name,file_name,length);
				 drive_name[length-1]='\0';
				 if (ALLOCATE(temp_string,char,length+8))
				 {
						strcpy(temp_string, "set dir ");
						strcat(temp_string, drive_name);
						temp_string[length+7]='\0';
						Execute_command_execute_string(command_data->execute_command,temp_string);
						DEALLOCATE(temp_string);
				 }
				 DEALLOCATE(drive_name);
			}
	 }
	 if (lastlength>length)
	 {
			if (ALLOCATE(temp_directory_name,char,lastlength+1))
			{
				 strncpy(temp_directory_name,file_name,lastlength);
				 temp_directory_name[lastlength]='\0';
				 if (ALLOCATE(directory_name,char,lastlength-length+2))
				 {
						directory_name = &temp_directory_name[length-1];
						directory_name[lastlength-length+1]='\0';
						if (ALLOCATE(temp_string,char,lastlength-length+10))
						{
							 strcpy(temp_string, "set dir ");
							 strcat(temp_string, directory_name);
							 temp_string[lastlength-length+9]='\0';
							 Execute_command_execute_string(command_data->execute_command,temp_string);
							 DEALLOCATE(temp_string);
						}
						DEALLOCATE(directory_name);
				 }
				 DEALLOCATE(temp_directory_name);
			}
	 }
	 if (lastlength>0)
	 {
			temp_name = &file_name[lastlength];
	 }
	 else
	 {
			temp_name = file_name;
	 }
	 return (temp_name);
}

#endif /*(WX_USER_INTERFACE)*/

static int set_command_prompt(const char *prompt, struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 26 June 2002

DESCRIPTION :
Changes the command prompt provided to the user.
==============================================================================*/
{
	int return_code = 0;

	ENTER(set_command_prompt);
	if (prompt && command_data)
	{
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
		if (command_data->command_window)
		{
			return_code = Command_window_set_command_prompt(command_data->command_window,
				prompt);
		}
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
		if (command_data->command_console)
		{
			return_code = Console_set_command_prompt(command_data->command_console,
				prompt);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_command_prompt.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_command_prompt */

struct Interpreter_command_element_selection_callback_data
{
	char *perl_action;
	struct Interpreter *interpreter;
}; /* struct Interpreter_command_element_selection_callback_data */

/***************************************************************************//**
 * Adds or removes elements to/from group.
 * @param manage_nodes  Set if nodes are added/removed with elements. Nodes are
 * only removed if not in use by any other elements in group.
 * @param manage_faces  Set if faces are added/removed with parent elements.
 * Faces are only removed if not in use by any other elements in group.
 */
static int process_modify_element_group(Cmiss_field_group_id group,
	Cmiss_region_id region, int dimension, char add_flag,
	Cmiss_field_id conditional_field, Cmiss_field_group_id from_group,
	Multi_range *element_ranges, char selected_flag, FE_value time,
	int manage_nodes, int manage_faces)
{
	if (!group || !region)
		return 0;
	int return_code = 1;
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_mesh_id master_mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, dimension);
	char remove_flag = !add_flag;
	Cmiss_mesh_group_id selection_mesh_group = 0;
	if (selected_flag)
	{
		Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(region);
		Cmiss_field_group_id selection_group = Cmiss_rendition_get_selection_group(rendition);
		if (selection_group)
		{
			Cmiss_field_element_group_id selection_element_group =
				Cmiss_field_group_get_element_group(selection_group, master_mesh);
			if (selection_element_group)
			{
				selection_mesh_group = Cmiss_field_element_group_get_mesh(selection_element_group);
				Cmiss_field_element_group_destroy(&selection_element_group);
			}
			Cmiss_field_group_destroy(&selection_group);
		}
		Cmiss_rendition_destroy(&rendition);
	}
	Cmiss_mesh_group_id from_mesh_group = 0;
	if (from_group)
	{
		Cmiss_field_element_group_id from_element_group =
			Cmiss_field_group_get_element_group(from_group, master_mesh);
		if (from_element_group)
		{
			from_mesh_group = Cmiss_field_element_group_get_mesh(from_element_group);
			Cmiss_field_element_group_destroy(&from_element_group);
		}

	}
	int objects_processed = 0;
	int elements_not_processed = 0;
	if (((!selected_flag) || selection_mesh_group) && ((!from_group) || from_mesh_group))
	{
		Cmiss_field_module_begin_change(field_module);
		Cmiss_field_element_group_id modify_element_group = Cmiss_field_group_get_element_group(group, master_mesh);
		if (!modify_element_group)
			modify_element_group = Cmiss_field_group_create_element_group(group, master_mesh);
		Cmiss_mesh_group_id modify_mesh_group = Cmiss_field_element_group_get_mesh(modify_element_group);
		objects_processed += Cmiss_mesh_get_size(Cmiss_mesh_group_base_cast(modify_mesh_group));
		Cmiss_field_element_group_destroy(&modify_element_group);
		Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(field_module);
		Cmiss_field_cache_set_time(cache, time);

		Cmiss_nodeset_group_id modify_nodeset_group = 0;
		Cmiss_field_node_group_id remove_node_group = 0;
		Cmiss_nodeset_group_id remove_nodeset_group = 0;
		if (manage_nodes)
		{
			Cmiss_nodeset_id master_nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
			Cmiss_field_node_group_id modify_node_group = Cmiss_field_group_get_node_group(group, master_nodeset);
			if ((!modify_node_group) && add_flag)
				modify_node_group = Cmiss_field_group_create_node_group(group, master_nodeset);
			if (modify_node_group)
			{
				modify_nodeset_group = Cmiss_field_node_group_get_nodeset(modify_node_group);
				objects_processed += Cmiss_nodeset_get_size(Cmiss_nodeset_group_base_cast(modify_nodeset_group));
				if (remove_flag)
				{
					Cmiss_field_id remove_node_group_field = Cmiss_field_module_create_node_group(field_module, master_nodeset);
					remove_node_group = Cmiss_field_cast_node_group(remove_node_group_field);
					remove_nodeset_group = Cmiss_field_node_group_get_nodeset(remove_node_group);
					Cmiss_field_destroy(&remove_node_group_field);
				}
				Cmiss_field_node_group_destroy(&modify_node_group);
			}
			Cmiss_nodeset_destroy(&master_nodeset);
		}
		Cmiss_nodeset_group_id working_nodeset_group = add_flag ? modify_nodeset_group : remove_nodeset_group;

		Cmiss_mesh_id master_face_mesh = 0;
		Cmiss_mesh_group_id modify_face_mesh_group = 0;
		Cmiss_field_element_group_id working_face_element_group = 0;
		Cmiss_mesh_group_id working_face_mesh_group = 0;
		if (manage_faces && (1 < dimension))
		{
			master_face_mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, dimension - 1);
			Cmiss_field_element_group_id modify_face_element_group = Cmiss_field_group_get_element_group(group, master_face_mesh);
			if ((!modify_face_element_group) && add_flag)
				modify_face_element_group = Cmiss_field_group_create_element_group(group, master_face_mesh);
			if (modify_face_element_group)
			{
				modify_face_mesh_group = Cmiss_field_element_group_get_mesh(modify_face_element_group);
				objects_processed += Cmiss_mesh_get_size(Cmiss_mesh_group_base_cast(modify_face_mesh_group));
				Cmiss_field_id working_face_element_group_field = Cmiss_field_module_create_element_group(field_module, master_face_mesh);
				working_face_element_group = Cmiss_field_cast_element_group(working_face_element_group_field);
				working_face_mesh_group = Cmiss_field_element_group_get_mesh(working_face_element_group);
				Cmiss_field_destroy(&working_face_element_group_field);
				Cmiss_field_element_group_destroy(&modify_face_element_group);
			}
		}

		Cmiss_mesh_id iteration_mesh = master_mesh;
		Cmiss_mesh_id selection_mesh = Cmiss_mesh_group_base_cast(selection_mesh_group);
		Cmiss_mesh_id from_mesh = Cmiss_mesh_group_base_cast(from_mesh_group);
		if (selected_flag && selection_mesh && !Cmiss_mesh_match(selection_mesh, Cmiss_mesh_group_base_cast(modify_mesh_group)))
		{
			iteration_mesh = selection_mesh;
		}
		if (from_mesh && (!Cmiss_mesh_match(from_mesh, Cmiss_mesh_group_base_cast(modify_mesh_group))) &&
			(Cmiss_mesh_get_size(from_mesh) < Cmiss_mesh_get_size(iteration_mesh)))
		{
			iteration_mesh = from_mesh;
		}
		Cmiss_element_iterator_id iter = Cmiss_mesh_create_element_iterator(iteration_mesh);
		Cmiss_element_id element = 0;
		while (NULL != (element = Cmiss_element_iterator_next_non_access(iter)))
		{
			if (element_ranges && !Multi_range_is_value_in_range(element_ranges, Cmiss_element_get_identifier(element)))
				continue;
			if (selection_mesh && (selection_mesh != iteration_mesh) && !Cmiss_mesh_contains_element(selection_mesh, element))
				continue;
			if (from_mesh && (from_mesh != iteration_mesh) && !Cmiss_mesh_contains_element(from_mesh, element))
				continue;
			if (conditional_field)
			{
				Cmiss_field_cache_set_element(cache, element);
				if (!Cmiss_field_evaluate_boolean(conditional_field, cache))
					continue;
			}
			if (add_flag)
			{
				if (!Cmiss_mesh_contains_element(Cmiss_mesh_group_base_cast(modify_mesh_group), element))
				{
					if (!Cmiss_mesh_group_add_element(modify_mesh_group, element))
					{
						display_message(ERROR_MESSAGE,
							"gfx modify egroup:  Could not add element %d", Cmiss_element_get_identifier(element));
						return_code = 0;
						break;
					}
				}
			}
			else
			{
				if (Cmiss_mesh_contains_element(Cmiss_mesh_group_base_cast(modify_mesh_group), element))
				{
					if (!Cmiss_mesh_group_remove_element(modify_mesh_group, element))
					{
						display_message(ERROR_MESSAGE,
							"gfx modify egroup:  Could not remove element %d", Cmiss_element_get_identifier(element));
						return_code = 0;
						break;
					}
				}
			}
			if (working_nodeset_group)
			{
				Cmiss_nodeset_group_add_element_nodes(working_nodeset_group, element);
			}
			if (working_face_mesh_group)
			{
				Cmiss_mesh_group_add_element_faces(working_face_mesh_group, element);
			}
		}
		Cmiss_element_iterator_destroy(&iter);

		if (remove_flag && (remove_nodeset_group || working_face_mesh_group))
		{
			// don't include faces and nodes still used by elements remaining in modify_mesh_group
			// Note: ignores nodes for other dimensions
			iter = Cmiss_mesh_create_element_iterator(Cmiss_mesh_group_base_cast(modify_mesh_group));
				while (NULL != (element = Cmiss_element_iterator_next_non_access(iter)))
				{
				if (remove_nodeset_group)
				{
					Cmiss_nodeset_group_remove_element_nodes(remove_nodeset_group, element);
				}
				if (working_face_mesh_group)
				{
					Cmiss_mesh_group_remove_element_faces(working_face_mesh_group, element);
				}
				}
				Cmiss_element_iterator_destroy(&iter);
				if (remove_nodeset_group)
				{
				Cmiss_nodeset_group_remove_nodes_conditional(modify_nodeset_group,
					Cmiss_field_node_group_base_cast(remove_node_group));
				}
		}
		if (working_face_mesh_group)
		{
			Cmiss_mesh_group_id modify_line_mesh_group = 0;
			Cmiss_field_element_group_id remove_line_element_group = 0;
			Cmiss_mesh_group_id remove_line_mesh_group = 0;
			if (2 < dimension)
			{
				Cmiss_mesh_id master_line_mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, dimension - 2);
				Cmiss_field_element_group_id modify_line_element_group = Cmiss_field_group_get_element_group(group, master_line_mesh);
				if (add_flag && !modify_line_element_group)
					modify_line_element_group = Cmiss_field_group_create_element_group(group, master_line_mesh);
				if (modify_line_element_group)
				{
					modify_line_mesh_group = Cmiss_field_element_group_get_mesh(modify_line_element_group);
					objects_processed += Cmiss_mesh_get_size(Cmiss_mesh_group_base_cast(modify_line_mesh_group));
					if (remove_flag)
					{
						Cmiss_field_id remove_line_element_group_field = Cmiss_field_module_create_element_group(field_module, master_line_mesh);
						remove_line_element_group = Cmiss_field_cast_element_group(remove_line_element_group_field);
						remove_line_mesh_group = Cmiss_field_element_group_get_mesh(remove_line_element_group);
						Cmiss_field_destroy(&remove_line_element_group_field);
					}
					Cmiss_field_element_group_destroy(&modify_line_element_group);
				}
				Cmiss_mesh_destroy(&master_line_mesh);
			}
			Cmiss_mesh_group_id working_line_mesh_group = add_flag ? modify_line_mesh_group : remove_line_mesh_group;
			iter = Cmiss_mesh_create_element_iterator(Cmiss_mesh_group_base_cast(working_face_mesh_group));
			while (NULL != (element = Cmiss_element_iterator_next_non_access(iter)))
			{
				if (add_flag)
				{
					Cmiss_mesh_group_add_element(modify_face_mesh_group, element);
				}
				else
				{
					Cmiss_mesh_group_remove_element(modify_face_mesh_group, element);
				}
				if (working_line_mesh_group)
				{
					Cmiss_mesh_group_add_element_faces(working_line_mesh_group, element);
				}
			}
			Cmiss_element_iterator_destroy(&iter);
			if (remove_line_element_group)
			{
				Cmiss_mesh_group_remove_elements_conditional(modify_line_mesh_group,
					Cmiss_field_element_group_base_cast(remove_line_element_group));
			}
			Cmiss_mesh_group_destroy(&remove_line_mesh_group);
			Cmiss_field_element_group_destroy(&remove_line_element_group);
			if (modify_line_mesh_group)
			{
				objects_processed -= Cmiss_mesh_get_size(Cmiss_mesh_group_base_cast(modify_line_mesh_group));
				Cmiss_mesh_group_destroy(&modify_line_mesh_group);
			}
		}

		Cmiss_mesh_group_destroy(&working_face_mesh_group);
		Cmiss_field_element_group_destroy(&working_face_element_group);
		if (modify_face_mesh_group)
		{
			objects_processed -= Cmiss_mesh_get_size(Cmiss_mesh_group_base_cast(modify_face_mesh_group));
			Cmiss_mesh_group_destroy(&modify_face_mesh_group);
		}
		Cmiss_mesh_destroy(&master_face_mesh);
		Cmiss_nodeset_group_destroy(&remove_nodeset_group);
		Cmiss_field_node_group_destroy(&remove_node_group);
		if (modify_nodeset_group)
		{
			objects_processed -= Cmiss_nodeset_get_size(Cmiss_nodeset_group_base_cast(modify_nodeset_group));
			Cmiss_nodeset_group_destroy(&modify_nodeset_group);
		}
		Cmiss_field_cache_destroy(&cache);
		objects_processed -= Cmiss_mesh_get_size(Cmiss_mesh_group_base_cast(modify_mesh_group));
		Cmiss_mesh_group_destroy(&modify_mesh_group);
		Cmiss_field_module_end_change(field_module);
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
	Cmiss_mesh_group_destroy(&from_mesh_group);
	Cmiss_mesh_group_destroy(&selection_mesh_group);
	Cmiss_mesh_destroy(&master_mesh);
	Cmiss_field_module_destroy(&field_module);

	return return_code;
}

static int set_Texture_image_from_field(struct Texture *texture,
	struct Computed_field *field,
	struct Computed_field *texture_coordinate_field,
	int propagate_field,
	struct Spectrum *spectrum,
	Cmiss_mesh_id search_mesh,
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
	Cmiss_region_id region;
	Cmiss_field_group_id group;
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

void export_object_name_parser(const char *path_name, const char **scene_name,
	const char **rendition_name, const char **graphic_name)
{
	const char *slash_pointer, *dot_pointer;
	int total_length, length;
	char *temp_name;
	if (path_name)
	{
		total_length = strlen(path_name);
		slash_pointer = strchr(path_name, '/');
		dot_pointer = strrchr(path_name, '.');
		if (dot_pointer)
		{
			if ((dot_pointer - path_name) < total_length)
			{
				*graphic_name = duplicate_string(dot_pointer + 1);
			}
			total_length = dot_pointer - path_name;
		}
		if (slash_pointer)
		{
			length = total_length - (slash_pointer + 1 - path_name);
			if (length > 1)
			{
				ALLOCATE(temp_name, char, length+1);
				strncpy(temp_name, slash_pointer + 1, length);
				temp_name[length] = '\0';
				*rendition_name = temp_name;
				total_length = slash_pointer - path_name;
			}
		}
		if (total_length > 1)
		{
			ALLOCATE(temp_name, char, total_length+1);
			strncpy(temp_name, path_name, total_length);
			temp_name[total_length] = '\0';
			*scene_name = temp_name;
		}
	}
}

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

static int apply_transformation_to_node(struct FE_node *node,
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
static struct Spectrum_settings *create_spectrum_component( Spectrum_settings_colour_mapping colour )
{
	int component = 1;
	struct Spectrum_settings *settings = CREATE(Spectrum_settings)();
	Spectrum_settings_set_type(settings, SPECTRUM_LINEAR);
	Spectrum_settings_set_colour_mapping(settings, colour);
	Spectrum_settings_set_extend_above_flag(settings, 1);
	Spectrum_settings_set_extend_below_flag(settings, 1);
	Spectrum_settings_set_reverse_flag(settings, 0);

	if ( colour == SPECTRUM_RED )
		component = 1;
	else if ( colour == SPECTRUM_GREEN )
		component = 2;
	else
		component = 3;

	Spectrum_settings_set_component_number( settings, component );

	return settings;
}

static int create_RGB_spectrum( struct Spectrum **spectrum, void *command_data_void )
{
	int return_code = 0, number_in_list = 0;
	struct LIST(Spectrum_settings) *spectrum_settings_list;
	struct Spectrum_settings *red_settings;
	struct Spectrum_settings *green_settings;
	struct Spectrum_settings *blue_settings;
	struct Cmiss_command_data *command_data = (struct Cmiss_command_data *)command_data_void;

	if ( command_data && ( (*spectrum) = CREATE(Spectrum)("RGB") ) )
	{
		spectrum_settings_list = get_Spectrum_settings_list( (*spectrum) );
		number_in_list = NUMBER_IN_LIST(Spectrum_settings)(spectrum_settings_list);
		if ( number_in_list > 0 )
		{
			REMOVE_ALL_OBJECTS_FROM_LIST(Spectrum_settings)(spectrum_settings_list);
		}
		red_settings = create_spectrum_component( SPECTRUM_RED );
		Spectrum_settings_add( red_settings, /* end of list = 0 */0,
			spectrum_settings_list );

		green_settings = create_spectrum_component( SPECTRUM_GREEN );
		Spectrum_settings_add( green_settings, /* end of list = 0 */0,
			spectrum_settings_list );

		blue_settings = create_spectrum_component( SPECTRUM_BLUE );
		Spectrum_settings_add( blue_settings, /* end of list = 0 */0,
			spectrum_settings_list );

		Spectrum_calculate_range( (*spectrum) );
		Spectrum_calculate_range( (*spectrum) );
		Spectrum_set_minimum_and_maximum( (*spectrum), 0, 1);
		Spectrum_set_opaque_colour_flag( (*spectrum), 0 );
		if (!ADD_OBJECT_TO_MANAGER(Spectrum)( (*spectrum),
				command_data->spectrum_manager))
		{
			DESTROY(Spectrum)(spectrum);
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

void create_triangle_mesh(struct Cmiss_region *region, Triangle_mesh *trimesh)
{
	struct FE_region *fe_region = Cmiss_region_get_FE_region(region);
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
	float coord1, coord2, coord3;
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

int Cmiss_region_list_rendition(Cmiss_region_id region, int commands_flag, int recursive_flag)
{
	if (!region)
		return 0;
	int return_code = 1;
	char *region_path = Cmiss_region_get_path(region);
	Cmiss_rendition_id rendition = Cmiss_region_get_rendition_internal(region);
	if (commands_flag)
	{
		int error = 0;
		char *command_prefix = duplicate_string("gfx modify g_element ");
		make_valid_token(&region_path);
		append_string(&command_prefix, region_path, &error);
		append_string(&command_prefix, " ", &error);
		return_code = Cmiss_rendition_list_commands(rendition, command_prefix, /*command_suffix*/";");
		DEALLOCATE(command_prefix);
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"Contents of region %s rendition:\n", region_path);
		return_code = Cmiss_rendition_list_contents(rendition);
	}
	Cmiss_rendition_destroy(&rendition);
	DEALLOCATE(region_path);
	if (recursive_flag)
	{
		Cmiss_region_id child = Cmiss_region_get_first_child(region);
		while (child)
		{
			if (!Cmiss_region_list_rendition(child, commands_flag, recursive_flag))
			{
				Cmiss_region_destroy(&child);
				return_code = 0;
				break;
			}
			Cmiss_region_reaccess_next_sibling(&child);
		}
	}
	return return_code;
}

int offset_region_identifier(Cmiss_region_id region, char element_flag, int element_offset,
	char face_flag, int face_offset, char line_flag, int line_offset, char node_flag, int node_offset, int use_data)
{
	int return_code = 1;
	struct FE_region *fe_region = NULL;
	if (region)
		fe_region = Cmiss_region_get_FE_region(region);
	/* Offset these nodes and elements before merging */
	if (fe_region)
	{
		FE_region_begin_change(fe_region);
		int highest_dimension = FE_region_get_highest_dimension(fe_region);
		if (element_flag)
		{
			if (!FE_region_change_element_identifiers(fe_region,
				highest_dimension, element_offset,
				(struct Computed_field *)NULL, /*time*/0))
			{
				return_code = 0;
			}
		}
		if (face_flag && (highest_dimension > 2))
		{
			if (!FE_region_change_element_identifiers(fe_region,
				/*dimension*/2, face_offset,
				(struct Computed_field *)NULL, /*time*/0))
			{
				return_code = 0;
			}
		}
		if (line_flag && (highest_dimension > 1))
		{
			if (!FE_region_change_element_identifiers(fe_region,
				/*dimension*/1, line_offset,
				(struct Computed_field *)NULL, /*time*/0))
			{
				return_code = 0;
			}
		}
		if (node_flag)
		{
			if (!use_data)
			{
				if (!FE_region_change_node_identifiers(fe_region,
					node_offset, (struct Computed_field *)NULL, /*time*/0))
				{
					return_code = 0;
				}
			}
			else
			{
				struct FE_region *data_fe_region = FE_region_get_data_FE_region(fe_region);
				FE_region_begin_change(data_fe_region);
				if (!FE_region_change_node_identifiers(data_fe_region,
					node_offset, (struct Computed_field *)NULL, /*time*/0))
				{
					return_code = 0;
				}
				FE_region_end_change(data_fe_region);
			}
		}
		FE_region_end_change(fe_region);
		struct Cmiss_region *child_region = Cmiss_region_get_first_child(region);
		while ((NULL != child_region))
		{
			return_code = offset_region_identifier(child_region, element_flag, element_offset, face_flag,
				face_offset, line_flag, line_offset, node_flag, node_offset, use_data);
			Cmiss_region_reaccess_next_sibling(&child_region);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Unable to get fe_region to offset nodes or elements in file .");
		return_code = 0;
	}
	return return_code;
}

/*
Global functions
----------------
*/
static int cmgui_execute_comfile(const char *comfile_name,const char *example_id,
	const char *examples_directory,const char *example_symbol,char **example_comfile_name,
	struct Execute_command *execute_command)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Executes the comfile specified on the command line.
==============================================================================*/
{
	int return_code;
	char global_temp_string[1000];

	ENTER(cmgui_execute_comfile);
	return_code=0;
	if ((comfile_name||example_id)&&execute_command)
	{
		if (example_id)
		{
			if (examples_directory&&example_symbol)
			{
				/* set the examples directory */
				sprintf(global_temp_string,"set dir ");
				strcat(global_temp_string,example_symbol);
				strcat(global_temp_string,"=");
				strcat(global_temp_string,example_id);
				Execute_command_execute_string(execute_command,global_temp_string);
				sprintf(global_temp_string,"open comfile ");
				if (comfile_name)
				{
					strcat(global_temp_string,comfile_name);
				}
				else
				{
					if (*example_comfile_name)
					{
						strcat(global_temp_string,*example_comfile_name);
					}
					else
					{
						strcat(global_temp_string,"example_");
						strcat(global_temp_string,example_id);
					}
				}
				strcat(global_temp_string,";");
				strcat(global_temp_string,example_symbol);
				strcat(global_temp_string," execute");
				return_code=Execute_command_execute_string(execute_command,global_temp_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"cmgui_execute_comfile.  Missing examples_directory or example_symbol");
			}
		}
		else
		{
			/* open the command line comfile */
			sprintf(global_temp_string,"open comfile ");
			strcat(global_temp_string,comfile_name);
			strcat(global_temp_string," execute");
			return_code=Execute_command_execute_string(execute_command, global_temp_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmgui_execute_comfile.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* cmgui_execute_comfile */

int Cmiss_command_data_process_command_line(int argc, const char *argv[], 
	struct Cmgui_command_line_options *command_line_options)
{
	int return_code = 1;
	struct Option_table *option_table = NULL;

	/* put command line options into structure for parsing & extract below */
	command_line_options->batch_mode_flag = (char)0;
	command_line_options->cm_start_flag = (char)0;
	command_line_options->cm_epath_directory_name = NULL;
	command_line_options->cm_parameters_file_name = NULL;
	command_line_options->command_list_flag = (char)0;
	command_line_options->console_mode_flag = (char)0;
	command_line_options->epath_directory_name = NULL;
	command_line_options->example_file_name = NULL;
	command_line_options->execute_string = NULL;
	command_line_options->write_help_flag = (char)0;
	command_line_options->id_name = NULL;
	command_line_options->mycm_start_flag = (char)0;
	command_line_options->no_display_flag = (char)0;
	command_line_options->random_number_seed = -1;
	command_line_options->server_mode_flag = (char)0;
	command_line_options->visual_id_number = 0;
	command_line_options->command_file_name = NULL;
	
	return return_code;
}

struct Cmiss_command_data *CREATE(Cmiss_command_data)(struct Context *context, 
	struct User_interface_module *UI_module)
/*******************************************************************************
LAST MODIFIED : 3 April 2003

DESCRIPTION :
Initialise all the subcomponents of cmgui and create the Cmiss_command_data
==============================================================================*/
{
	char *cm_examples_directory,*cm_parameters_file_name,*comfile_name,
		*example_id,*examples_directory,*examples_environment,*execute_string,
		*version_command_id;
	char global_temp_string[1000];
	int return_code;
	int batch_mode, console_mode, command_list, no_display, non_random,
		server_mode, start_cm, start_mycm, visual_id, write_help;
#if defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER)
	int status;
#endif /* defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER) */
	struct Cmgui_command_line_options command_line_options;
	struct Cmiss_command_data *command_data;
#if defined(USE_CMGUI_COMMAND_WINDOW)
	struct Command_window *command_window;
#endif /* defined(USE_CMGUI_COMMAND_WINDOW) */
	struct Graphical_material *material;
	struct Option_table *option_table;
	User_settings user_settings;
#if defined (WIN32_USER_INTERFACE)
	ENTER(WinMain);
#endif /* defined (WIN32_USER_INTERFACE) */
	return_code = 1;

	if (ALLOCATE(command_data, struct Cmiss_command_data, 1))
	{
		command_data->access_count = 1;
		// duplicate argument list so it can be modified by User Interface
		/* initialize application specific global variables */
		command_data->execute_command = CREATE(Execute_command)();;
		command_data->set_command = CREATE(Execute_command)();
		command_data->event_dispatcher = (struct Event_dispatcher *)NULL;
		command_data->user_interface= (struct User_interface *)NULL;
		command_data->emoter_slider_dialog=(struct Emoter_dialog *)NULL;
#if defined (WX_USER_INTERFACE)
		command_data->data_viewer=(struct Node_viewer *)NULL;
		command_data->node_viewer=(struct Node_viewer *)NULL;
		command_data->element_point_viewer=(struct Element_point_viewer *)NULL;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
		command_data->material_editor = (struct Material_editor *)NULL;
		command_data->region_tree_viewer = (struct Region_tree_viewer *)NULL;
		command_data->spectrum_editor_dialog = (struct Spectrum_editor_dialog *)NULL;
#endif /*defined (WX_USER_INTERFACE) */
		command_data->example_directory=(char *)NULL;

#if defined (WX_USER_INTERFACE)
		command_data->comfile_window_manager=(struct MANAGER(Comfile_window) *)NULL;
#endif /* defined WX_USER_INTERFACE*/
		command_data->default_light=(struct Light *)NULL;
		command_data->light_manager=(struct MANAGER(Light) *)NULL;
		command_data->default_light_model=(struct Light_model *)NULL;
		command_data->light_model_manager=(struct MANAGER(Light_model) *)NULL;
		command_data->environment_map_manager=(struct MANAGER(Environment_map) *)NULL;
		command_data->volume_texture_manager=(struct MANAGER(VT_volume_texture) *)NULL;
		command_data->default_spectrum=(struct Spectrum *)NULL;
		command_data->spectrum_manager=(struct MANAGER(Spectrum) *)NULL;
		command_data->graphics_buffer_package=(struct Graphics_buffer_package *)NULL;
		command_data->scene_viewer_package=(struct Cmiss_scene_viewer_package *)NULL;
		command_data->graphics_module = (struct Cmiss_graphics_module *)NULL;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		command_data->graphics_window_manager=(struct MANAGER(Graphics_window) *)NULL;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
		command_data->root_region = (struct Cmiss_region *)NULL;
		command_data->curve_manager=(struct MANAGER(Curve) *)NULL;
		command_data->basis_manager=(struct MANAGER(FE_basis) *)NULL;
		command_data->streampoint_list=(struct Streampoint *)NULL;
#if defined (SELECT_DESCRIPTORS)
		command_data->device_list=(struct LIST(Io_device) *)NULL;
#endif /* defined (SELECT_DESCRIPTORS) */
		command_data->glyph_manager=(struct MANAGER(GT_object) *)NULL;
		command_data->any_object_selection=(struct Any_object_selection *)NULL;
		command_data->element_point_ranges_selection=(struct Element_point_ranges_selection *)NULL;
		command_data->io_stream_package = (struct IO_stream_package *)NULL;
		command_data->computed_field_package=(struct Computed_field_package *)NULL;
		command_data->default_scene=(struct Scene *)NULL;
		command_data->scene_manager=(struct MANAGER(Scene) *)NULL;
#if defined (USE_OPENCASCADE)
		command_data->cad_tool = (struct Cad_tool *)NULL;
#endif /* defined (USE_OPENCASCADE) */
		command_data->examples_directory=(char *)NULL;
		command_data->example_requirements=(char *)NULL;
		command_data->cm_examples_directory=(char *)NULL;
		command_data->cm_parameters_file_name=(char *)NULL;
		command_data->default_time_keeper = (struct Time_keeper *)NULL;
		command_data->background_colour.red=(float)0;
		command_data->background_colour.green=(float)0;
		command_data->background_colour.blue=(float)0;
		command_data->foreground_colour.red=(float)1;
		command_data->foreground_colour.green=(float)1;
		command_data->foreground_colour.blue=(float)1;
		command_data->help_directory=(char *)NULL;
		command_data->help_url=(char *)NULL;

		/* set default values for command-line modifiable options */
		/* Note User_interface will not be created if command_list selected */
		batch_mode = 0;
		command_list = 0;
		console_mode = 0;
		no_display = 0;
		server_mode = 0;
		visual_id = 0;
		write_help = 0;
		/* flag to say randomise */
		non_random = -1;
		/* flag for starting cm */
		start_cm = 0;
		/* flag for starting mycm */
		start_mycm = 0;
		/* to over-ride all other example directory settings */
		examples_directory = (char *)NULL;
		/* back-end examples directory */
		cm_examples_directory = (char *)NULL;
		/* back-end parameters file */
		cm_parameters_file_name = (char *)NULL;
		/* the comfile is in the examples directory */
		example_id = (char *)NULL;
		/* a string executed by the interpreter before loading any comfiles */
		execute_string = (char *)NULL;
		/* set no command id supplied */
		version_command_id = (char *)NULL;
		/* the name of the comfile to be run on startup */
		comfile_name = (char *)NULL;

		user_settings.examples_directory = (char *)NULL;
		user_settings.help_directory = (char *)NULL;
		user_settings.help_url = (char *)NULL;
		user_settings.startup_comfile = (char *)NULL;

		/* parse commmand line options */

		/* put command line options into structure for parsing & extract below */
		command_line_options.batch_mode_flag = (char)batch_mode;
		command_line_options.cm_start_flag = (char)start_cm;
		command_line_options.cm_epath_directory_name = cm_examples_directory;
		command_line_options.cm_parameters_file_name = cm_parameters_file_name;
		command_line_options.command_list_flag = (char)command_list;
		command_line_options.console_mode_flag = (char)console_mode;
		command_line_options.epath_directory_name = examples_directory;
		command_line_options.example_file_name = example_id;
		command_line_options.execute_string = execute_string;
		command_line_options.write_help_flag = (char)write_help;
		command_line_options.id_name = version_command_id;
		command_line_options.mycm_start_flag = (char)start_mycm;
		command_line_options.no_display_flag = (char)no_display;
		command_line_options.random_number_seed = non_random;
		command_line_options.server_mode_flag = (char)server_mode;
		command_line_options.visual_id_number = visual_id;
		command_line_options.command_file_name = comfile_name;
		/* copy command line options to local vars for use and easy clean-up */
		batch_mode = (int)command_line_options.batch_mode_flag;
		start_cm = command_line_options.cm_start_flag;
		cm_examples_directory = command_line_options.cm_epath_directory_name;
		cm_parameters_file_name = command_line_options.cm_parameters_file_name;
		command_list = command_line_options.command_list_flag;
		console_mode = command_line_options.console_mode_flag;
		examples_directory = command_line_options.epath_directory_name;
		example_id = command_line_options.example_file_name;
		execute_string = command_line_options.execute_string;
		write_help = command_line_options.write_help_flag;
		version_command_id = command_line_options.id_name;
		start_mycm = command_line_options.mycm_start_flag;
		no_display = command_line_options.no_display_flag;
		non_random = command_line_options.random_number_seed;
		server_mode = (int)command_line_options.server_mode_flag;
		visual_id = command_line_options.visual_id_number;
		comfile_name = command_line_options.command_file_name;

		command_data->io_stream_package = Cmiss_context_get_default_IO_stream_package(context);

		if ((!command_list) && (!write_help))
		{
			if (NULL != (command_data->event_dispatcher = 
					Cmiss_context_get_default_event_dispatcher(context)))
			{
				if (!no_display)
				{
				}
			}
			else
			{
				return_code = 0;
			}
		}

		/* use command line options in preference to defaults read from XResources */

		if (examples_directory)
		{
			command_data->examples_directory = examples_directory;
		}
		else if (NULL != (examples_environment = getenv("CMISS_EXAMPLES")))
		{
			command_data->examples_directory = duplicate_string(examples_environment);
		}
		else
		{
			command_data->examples_directory = (char *)NULL;
		}
#if defined (WIN32_SYSTEM)
		/* We don't know about cygdrive as we are using the win32 api,
		   but try and interpret the variable anyway.  We can't handle
			other cygwin paths unless we call out to cygpath. */
		if (command_data->examples_directory
		    && (strlen(command_data->examples_directory) > 11)
		    && (!strncmp(command_data->examples_directory, "/cygdrive", 9)))
		{
			char *new_examples_string;
			ALLOCATE(new_examples_string, char,
				strlen(command_data->examples_directory) + 10);
			new_examples_string[0] = command_data->examples_directory[10];
		   new_examples_string[1] = ':';
		   new_examples_string[2] = '\\';
		   strcpy(new_examples_string + 3, command_data->examples_directory + 12);
		   DEALLOCATE(command_data->examples_directory);
		   command_data->examples_directory = new_examples_string;
		}
#endif /* defined (WIN32_SYSTEM) */
		command_data->cm_examples_directory = cm_examples_directory;
		command_data->cm_parameters_file_name = cm_parameters_file_name;
		command_data->help_directory = user_settings.help_directory;
		command_data->help_url = user_settings.help_url;

		/* create the managers */

#if defined (WX_USER_INTERFACE)
		/* comfile window manager */
		command_data->comfile_window_manager = UI_module->comfile_window_manager;
#endif /* defined (WX_USER_INTERFACE) */
		command_data->graphics_module = 
			Cmiss_context_get_default_graphics_module(context);
		/* light manager */
		if (NULL != (command_data->light_manager=Cmiss_graphics_module_get_light_manager(
				        command_data->graphics_module)))
		{
			command_data->default_light=Cmiss_graphics_module_get_default_light(
				command_data->graphics_module);
		}
		command_data->light_model_manager=
			Cmiss_graphics_module_get_light_model_manager(command_data->graphics_module);
		
		command_data->default_light_model=
			Cmiss_graphics_module_get_default_light_model(command_data->graphics_module);
		// ensure we have a default tessellation
		Cmiss_tessellation_id default_tessellation = Cmiss_graphics_module_get_default_tessellation(command_data->graphics_module);
		Cmiss_tessellation_destroy(&default_tessellation);

		/* environment map manager */
		command_data->environment_map_manager=CREATE(MANAGER(Environment_map))();
		/* volume texture manager */
		command_data->volume_texture_manager=CREATE(MANAGER(VT_volume_texture))();
		/* spectrum manager */
		if (NULL != (command_data->spectrum_manager=
				Cmiss_graphics_module_get_spectrum_manager(
					command_data->graphics_module)))
		{
			command_data->default_spectrum=
				Cmiss_graphics_module_get_default_spectrum(command_data->graphics_module);
		}
		/* create Material package and CMGUI default materials */
		if (NULL != (command_data->material_package = 
				Cmiss_graphics_module_get_material_package(command_data->graphics_module)))
		{
			if (NULL != (material = Material_package_get_default_material(command_data->material_package)))
			{
				Graphical_material_set_alpha(material, 1.0);
			}
		}
		command_data->graphics_font_package = Cmiss_graphics_module_get_font_package(
			command_data->graphics_module);
		command_data->default_font = Cmiss_graphics_module_get_default_font(
			command_data->graphics_module);

#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		command_data->graphics_buffer_package = UI_module->graphics_buffer_package;
		/* graphics window manager.  Note there is no default window. */
		command_data->graphics_window_manager = UI_module->graphics_window_manager;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
		/* FE_element_shape manager */
		/*???DB.  To be done */
		command_data->element_shape_list=CREATE(LIST(FE_element_shape))();

		command_data->curve_manager=Cmiss_context_get_default_curve_manager(context);

		command_data->basis_manager=CREATE(MANAGER(FE_basis))();

		command_data->root_region = Cmiss_context_get_default_region(context);

#if defined (SELECT_DESCRIPTORS)
		/* create device list */
		/*SAB.  Eventually want device manager */
		command_data->device_list=CREATE(LIST(Io_device))();
#endif /* defined (SELECT_DESCRIPTORS) */

		command_data->glyph_manager = Cmiss_graphics_module_get_default_glyph_manager(
			command_data->graphics_module);

		/* global list of selected objects */
		command_data->any_object_selection = Cmiss_context_get_any_object_selection(context);
		command_data->element_point_ranges_selection = 
			Cmiss_context_get_element_point_ranges_selection(context);

		/* computed field manager and default computed fields zero, xi,
			default_coordinate, etc. */
		/*???RC should the default computed fields be established in
		  CREATE(Computed_field_package)? */

		/*???GRC will eventually remove manager from field package so it is
		  purely type-specific data. Field manager is now owned by region.
		  Temporarily passing it to package to keep existing code running */
		struct MANAGER(Computed_field) *computed_field_manager=
			Cmiss_region_get_Computed_field_manager(command_data->root_region);
		command_data->computed_field_package = 
			CREATE(Computed_field_package)(computed_field_manager);
		/* Add Computed_fields to the Computed_field_package */
		if (command_data->computed_field_package)
		{
			Computed_field_register_types_coordinate(
				command_data->computed_field_package);
			if (command_data->root_region)
			{
				Computed_field_register_type_alias(
					command_data->computed_field_package, 
					command_data->root_region);
			}
			Computed_field_register_types_arithmetic_operators(
				command_data->computed_field_package);
			Computed_field_register_types_trigonometry(
				command_data->computed_field_package);
			Computed_field_register_types_format_output(
				command_data->computed_field_package);
			if (command_data->root_region)
			{
				Computed_field_register_types_compose(
					command_data->computed_field_package, 
					command_data->root_region);
			}
			Computed_field_register_types_composite(
				command_data->computed_field_package);		
			Computed_field_register_types_conditional(
				command_data->computed_field_package);		
			if (command_data->curve_manager)
			{
				Computed_field_register_types_curve(
					command_data->computed_field_package, 
					command_data->curve_manager);
			}
#if defined (USE_ITK)
			Computed_field_register_types_derivatives(
				command_data->computed_field_package);
#endif /* defined (USE_ITK) */
			Computed_field_register_types_fibres(
				command_data->computed_field_package);
			Computed_field_register_types_function(
					command_data->computed_field_package);
			Computed_field_register_types_logical_operators(
				command_data->computed_field_package);
			if (command_data->root_region)
			{
				Computed_field_register_types_lookup(
					command_data->computed_field_package, 
					command_data->root_region);
			}
			Computed_field_register_types_matrix_operators(
				command_data->computed_field_package);
			Computed_field_register_types_nodeset_operators(
				command_data->computed_field_package);
			Computed_field_register_types_vector_operators(
				command_data->computed_field_package);
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
			if (command_data->graphics_window_manager)
			{
				Computed_field_register_type_scene_viewer_projection(
					command_data->computed_field_package,
					command_data->graphics_window_manager);
			}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
			Computed_field_register_type_image(
				command_data->computed_field_package);
			if (command_data->root_region)
			{
				Computed_field_register_type_integration(
					command_data->computed_field_package, 
					command_data->root_region);
			}
			Computed_field_register_types_finite_element(
				command_data->computed_field_package);
			Computed_field_register_types_deformation(
				command_data->computed_field_package);
			Computed_field_register_types_string_constant(
				command_data->computed_field_package);

			Computed_field_register_types_image_resample(
				command_data->computed_field_package);
#if defined (USE_ITK)
			Computed_field_register_types_threshold_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_binary_threshold_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_canny_edge_detection_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_mean_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_sigmoid_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_discrete_gaussian_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_histogram_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_curvature_anisotropic_diffusion_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_derivative_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_rescale_intensity_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_connected_threshold_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_gradient_magnitude_recursive_gaussian_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_fast_marching_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_binary_dilate_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_binary_erode_image_filter(
				command_data->computed_field_package);
#endif /* defined (USE_ITK) */
		}
		/* graphics_module */
		command_data->default_time_keeper=ACCESS(Time_keeper)(UI_module->default_time_keeper);

		/* scene manager */
		/*???RC & SAB.   LOTS of managers need to be created before this 
		  and the User_interface too */
		if (NULL != (command_data->scene_manager=Cmiss_graphics_module_get_scene_manager(
									 command_data->graphics_module)))
		Cmiss_graphics_module_enable_renditions(
			command_data->graphics_module,command_data->root_region);
		{
			command_data->default_scene = Cmiss_graphics_module_get_default_scene(
				command_data->graphics_module);
			if (command_data->default_scene)
			{
//			display_message(INFORMATION_MESSAGE,"Cmiss_command_data *CREATE\n");
				Cmiss_scene_set_region(command_data->default_scene, command_data->root_region);
			}
		}

#if defined (OLD_CODE)
		if(command_data->default_scene)
		{
			Scene_enable_time_behaviour(command_data->default_scene,
				command_data->default_time_keeper);
		}
#endif /* defined (OLD_CODE) */
		if (command_data->computed_field_package && command_data->default_time_keeper)
		{
			Computed_field_register_types_time(command_data->computed_field_package,
				command_data->default_time_keeper);
		}

		/* properly set up the Execute_command objects */
		Execute_command_set_command_function(command_data->execute_command,
			cmiss_execute_command, (void *)command_data);
		Execute_command_set_command_function(command_data->set_command,
			cmiss_set_command, (void *)command_data);
		/* initialize random number generator */
		if (-1 == non_random)
		{
			/* randomise */
			srand(time(NULL));
			/*???DB.  time is not ANSI */
		}
		else
		{
			/* randomise using given seed */
			srand(non_random);
		}

		if (return_code && (!command_list) && (!write_help))
		{
			if (start_cm||start_mycm)
			{
				sprintf(global_temp_string,"create cm");
				if (start_mycm)
				{
					strcat(global_temp_string," mycm");
				}
				if (cm_parameters_file_name)
				{
					strcat(global_temp_string," parameters ");
					strcat(global_temp_string,cm_parameters_file_name);
				}
				if (cm_examples_directory)
				{
					strcat(global_temp_string," examples_directory ");
					strcat(global_temp_string,cm_examples_directory);
				}
				/* start the back-end */
				cmiss_execute_command(global_temp_string,
					(void *)command_data);
			}
			if (user_settings.startup_comfile)
			{
				/* Can't get the startupComfile name without X at the moment */
				cmgui_execute_comfile(user_settings.startup_comfile, NULL,
					NULL, NULL, (char **)NULL, command_data->execute_command);
			}
			if (execute_string)
			{
				cmiss_execute_command(execute_string,(void *)command_data);
			}
			if (example_id||comfile_name)
			{
				/* open the command line comfile */
				cmgui_execute_comfile(comfile_name,example_id,
					command_data->examples_directory,
					CMGUI_EXAMPLE_DIRECTORY_SYMBOL, &command_data->example_comfile,
					command_data->execute_command);
			}
		}

		if ((!command_list) && (!write_help))
		{
			/* START_ERROR_HANDLING;*/
			switch (signal_code)
			{
				case SIGFPE:
				{
					printf("Floating point exception occurred\n");
					display_message(ERROR_MESSAGE,
						"Floating point exception occurred");
				} break;
				case SIGILL:
				{
					printf("Illegal instruction occurred\n");
					display_message(ERROR_MESSAGE,
						"Illegal instruction occurred");
				} break;
				case SIGSEGV:
				{
					printf("Invalid memory reference occurred\n");
					display_message(ERROR_MESSAGE,
						"Invalid memory reference occurred");
				} break;
			}
		}
		if (command_list)
		{
			cmiss_execute_command("??", (void *)command_data);
		}
		if (example_id)
		{
			DEALLOCATE(example_id);
		}
		if (execute_string)
		{
			DEALLOCATE(execute_string);
		}
		if (version_command_id)
		{
			DEALLOCATE(version_command_id);
		}
		if (comfile_name)
		{
			DEALLOCATE(comfile_name);
		}

		if (command_list || write_help || batch_mode || !return_code)
		{
			Cmiss_command_data_destroy(&command_data);
		}
	}
	else
	{
		command_data = (struct Cmiss_command_data *)NULL;
	}
	LEAVE;

	return (command_data);
} /* CREATE(Cmiss_command_data) */

int DESTROY(Cmiss_command_data)(struct Cmiss_command_data **command_data_address)
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Clean up the command_data, deallocating all the associated memory and resources.
NOTE: Do not call this directly: call Cmiss_command_data_destroy() to deaccess.
==============================================================================*/
{
	int return_code = 0;
#if defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER)
	int status;
#endif /* defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER) */
	struct Cmiss_command_data *command_data;
	ENTER(DESTROY(Cmiss_command_data));

	if (command_data_address && (command_data = *command_data_address))
	{
		if (command_data->access_count != 0)
		{
			display_message(ERROR_MESSAGE,
				"Call to DESTROY(Cmiss_command_data) while still in use");
			return 0;
		}
		if (command_data->emoter_slider_dialog)
		{
			DESTROY(Emoter_dialog)(&command_data->emoter_slider_dialog);
		}
#if defined (WX_USER_INTERFACE)
		/* viewers */
		if (command_data->data_viewer)
		{
			DESTROY(Node_viewer)(&(command_data->data_viewer));
		}
		if (command_data->node_viewer)
		{
			DESTROY(Node_viewer)(&(command_data->node_viewer));
		}
		if (command_data->element_point_viewer)
		{
			DESTROY(Element_point_viewer)(&(command_data->element_point_viewer));
		}
#endif /* defined (WX_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
		if (command_data->material_editor)
		{
			DESTROY(Material_editor)(&(command_data->material_editor));
		}
		if (command_data->region_tree_viewer)
		{
			DESTROY(Region_tree_viewer)(&(command_data->region_tree_viewer));
		}
		if (command_data->spectrum_editor_dialog)
		{
			DESTROY(Spectrum_editor_dialog)(&(command_data->spectrum_editor_dialog));
		}
#endif /* defined (WX_USER_INTERFACE) */

		DEACCESS(Scene)(&command_data->default_scene);
		if (command_data->graphics_module)
		{
			Cmiss_graphics_module_destroy(&command_data->graphics_module);
		}
		DEACCESS(Time_keeper)(&command_data->default_time_keeper);
		if (command_data->computed_field_package)
		{
			Computed_field_package_remove_types(command_data->computed_field_package);
			DESTROY(Computed_field_package)(&command_data->computed_field_package);
		}
#if defined (SELECT_DESCRIPTORS)
		DESTROY(LIST(Io_device))(&command_data->device_list);
#endif /* defined (SELECT_DESCRIPTORS) */

		DEACCESS(Cmiss_region)(&(command_data->root_region));
		DESTROY(MANAGER(FE_basis))(&command_data->basis_manager);
		DESTROY(LIST(FE_element_shape))(&command_data->element_shape_list);

		/* some fields register for changes with the following managers,
			 hence must destroy after regions and their fields */
		command_data->curve_manager = NULL;
		DEACCESS(Spectrum)(&(command_data->default_spectrum));
		command_data->spectrum_manager=NULL;
		DEACCESS(Material_package)(&command_data->material_package);
		DEACCESS(Graphics_font)(&command_data->default_font);
		DESTROY(MANAGER(VT_volume_texture))(&command_data->volume_texture_manager);
		DESTROY(MANAGER(Environment_map))(&command_data->environment_map_manager);				
		DEACCESS(Light_model)(&(command_data->default_light_model));
		DEACCESS(Light)(&(command_data->default_light));
		command_data->light_manager = NULL;
		if (command_data->example_directory)
		{
			DEALLOCATE(command_data->example_directory);
		}
		if (command_data->example_comfile)
		{
			DEALLOCATE(command_data->example_comfile);
		}
		if (command_data->example_requirements)
		{
			DEALLOCATE(command_data->example_requirements);
		}

		Close_image_environment();

		DESTROY(Execute_command)(&command_data->execute_command);
		DESTROY(Execute_command)(&command_data->set_command);

#if defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER)
		destroy_interpreter(command_data->interpreter, &status);
#endif /* defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER) */

		if (command_data->command_console)
		{
			DESTROY(Console)(&command_data->command_console);
		}
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
		if (command_data->command_window)
		{
			DESTROY(Command_window)(&command_data->command_window);
		}
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */

		if (command_data->user_interface)
			command_data->user_interface = NULL;
		if (command_data->event_dispatcher)
			command_data->event_dispatcher = NULL;

		/* clean up command-line options */

		if (command_data->examples_directory)
		{
			DEALLOCATE(command_data->examples_directory);
		}
		if (command_data->cm_examples_directory)
		{
			DEALLOCATE(command_data->cm_examples_directory);
		}
		if (command_data->cm_parameters_file_name)
		{
			DEALLOCATE(command_data->cm_parameters_file_name);
		}

		DEALLOCATE(*command_data_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Cmiss_command_data).  "
			"Invalid arguments");
	}
	
	
	LEAVE;
	
	return (return_code);
} /* DESTROY(Cmiss_command_data) */

struct Cmiss_command_data *Cmiss_command_data_access(struct Cmiss_command_data *command_data)
{
	if (command_data)
	{
		command_data->access_count++;
	}
	return command_data;
}

int Cmiss_command_data_destroy(
	struct Cmiss_command_data **command_data_address)
{
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(Cmiss_command_data_destroy);
	if (command_data_address && (NULL != (command_data = *command_data_address)))
	{
		command_data->access_count--;
		if (0 == command_data->access_count)
		{
			DESTROY(Cmiss_command_data)(command_data_address);
		}
		*command_data_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_command_data_destroy.  Missing command data");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_command_data_main_loop(struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Process events until some events request the program to finish.
==============================================================================*/
{
	int return_code = 0;

	ENTER(Cmiss_command_data_main_loop);
	/* main processing / loop */
	if (command_data && command_data->event_dispatcher)
	{
		/* user interface loop */						
		return_code=Event_dispatcher_main_loop(command_data->event_dispatcher);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_command_data_main_loop */

struct Cmiss_region *Cmiss_command_data_get_root_region(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 18 April 2003

DESCRIPTION :
Returns the root region from the <command_data>.
==============================================================================*/
{
	struct Cmiss_region *root_region;

	ENTER(Cmiss_command_data_get_root_region);
	root_region=(struct Cmiss_region *)NULL;
	if (command_data)
	{
		/* API functions return accessed values */
		root_region=ACCESS(Cmiss_region)(command_data->root_region);
	}
	LEAVE;

	return (root_region);
} /* Cmiss_command_data_get_root_region */

struct Cmiss_time_keeper *Cmiss_command_data_get_default_time_keeper(
	struct Cmiss_command_data *command_data)
{
	struct Cmiss_time_keeper *default_time_keeper;

	ENTER(Cmiss_command_data_get_default_time_keeper);
	default_time_keeper=(struct Cmiss_time_keeper *)NULL;
	if (command_data)
	{
		default_time_keeper=command_data->default_time_keeper;
	}
	LEAVE;

	return (default_time_keeper);
} /* Cmiss_command_data_get_default_time_keeper */

struct Execute_command *Cmiss_command_data_get_execute_command(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 28 May 2003

DESCRIPTION :
Returns the execute command structure from the <command_data>, useful for 
executing cmiss commands from C.
==============================================================================*/
{
	struct Execute_command *execute_command;

	ENTER(Cmiss_command_data_get_execute_command);
	execute_command=(struct Execute_command *)NULL;
	if (command_data)
	{
		execute_command=command_data->execute_command;
	}
	LEAVE;

	return (execute_command);
} /* Cmiss_command_data_get_execute_command */

struct IO_stream_package *Cmiss_command_data_get_IO_stream_package(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 16 September 2004

DESCRIPTION :
Returns the io_stream_package structure from the <command_data>
==============================================================================*/
{
	struct IO_stream_package *io_stream_package;

	ENTER(Cmiss_command_data_get_io_stream_package);
	io_stream_package=(struct IO_stream_package *)NULL;
	if (command_data)
	{
		io_stream_package = command_data->io_stream_package;
	}
	LEAVE;

	return (io_stream_package);
} /* Cmiss_command_data_get_io_stream_package */

Idle_package_id Cmiss_command_data_get_idle_package(
	struct Cmiss_command_data *command_data
)
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
Gets an Idle_package for this <command_data>
==============================================================================*/
{
	struct Idle_package *idle_package;

	ENTER(Cmiss_command_data_get_idle_package);
	idle_package = (struct Idle_package *)NULL;
	if (command_data)
	{
		idle_package = CREATE(Idle_package)(command_data->event_dispatcher);
	}
	LEAVE;

	return (idle_package);
}

struct MANAGER(Computed_field) *Cmiss_command_data_get_computed_field_manager(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 16 September 2004

DESCRIPTION :
Returns the root region from the <command_data>.
==============================================================================*/
{
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(Cmiss_command_data_get_computed_field_manager);
	computed_field_manager=(struct MANAGER(Computed_field) *)NULL;
	if (command_data)
	{
		computed_field_manager = Computed_field_package_get_computed_field_manager(
			command_data->computed_field_package);
	}
	LEAVE;

	return (computed_field_manager);
} /* Cmiss_command_data_get_computed_field_manager */

struct User_interface *Cmiss_command_data_get_user_interface(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 25 January 2006

DESCRIPTION :
Gets the user_interface for this <command_data>
==============================================================================*/
{
	struct User_interface *user_interface;

	ENTER(Cmiss_command_data_get_user_interface);
	user_interface = (struct User_interface *)NULL;
	if (command_data)
	{
		user_interface = command_data->user_interface;
	}
	LEAVE;

	return (user_interface);
} /* Cmiss_command_data_get_user_interface */

struct Cmiss_scene_viewer_package *Cmiss_command_data_get_scene_viewer_package(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION :
Returns the scene viewer data from the <command_data>.
==============================================================================*/
{
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package;

	ENTER(Cmiss_command_package_get_scene_viewer_package);
	cmiss_scene_viewer_package=(struct Cmiss_scene_viewer_package *)NULL;
	if (command_data)
	{
		cmiss_scene_viewer_package = command_data->scene_viewer_package;
	}
	LEAVE;

	return (cmiss_scene_viewer_package);
}

struct Cmiss_graphics_module *Cmiss_command_data_get_graphics_module(
	struct Cmiss_command_data *command_data)
{
	struct Cmiss_graphics_module *graphics_module;

	ENTER(Cmiss_command_package_get_graphics_module);
	graphics_module=(struct Cmiss_graphics_module *)NULL;
	if (command_data)
	{
		graphics_module = Cmiss_graphics_module_access(command_data->graphics_module);
	}
	LEAVE;

	return (graphics_module);
}

struct MANAGER(Graphics_window) *Cmiss_command_data_get_graphics_window_manager(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 26 January 2007

DESCRIPTION :
Returns the graphics_window manager from the <command_data>.
==============================================================================*/
{
	struct MANAGER(Graphics_window) *graphics_window_manager;

	ENTER(Cmiss_command_data_get_graphics_window_manager);
	graphics_window_manager=(struct MANAGER(Graphics_window) *)NULL;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
	if (command_data)
	{
		graphics_window_manager = command_data->graphics_window_manager;
	}
#else /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
	USE_PARAMETER(command_data);
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
	LEAVE;

	return (graphics_window_manager);
} /* Cmiss_command_data_get_graphics_window_manager */

int Cmiss_command_data_set_cmgui_string(Cmiss_command_data *command_data, const char *name_string, 
	const char *version_string,const char *date_string, const char *copyright_string, 
	const char *build_string, const char *revision_string)
{
	int return_code = 1;
	if (command_data)
	{
#if defined(USE_CMGUI_COMMAND_WINDOW)
		if (command_data->command_window)
		{
			return_code =Command_window_set_cmgui_string(command_data->command_window,
				name_string, version_string, date_string, copyright_string, build_string, revision_string);
		}
		else
		{
			return_code = 0;
		}
#endif
	}

	return return_code;
}
