/*******************************************************************************
FILE : cmiss.c

LAST MODIFIED : 12 April 2006

DESCRIPTION :
Functions for executing cmiss commands.
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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#if defined (MOTIF)
#include <Xm/List.h>
#endif /* defined (MOTIF) */
#include "api/cmiss_scene_viewer.h"
#include "api/cmiss_scene_viewer_private.h"
#if defined (CELL)
#include "cell/cell_interface.h"
#include "cell/cell_window.h"
#endif /* defined (CELL) */
#include "comfile/comfile.h"
#if defined (MOTIF)
#include "comfile/comfile_window.h"
#endif /* defined (MOTIF) */
#if defined (WX_USER_INTERFACE)
#include "comfile/comfile_window_wx.h"
#endif /* defined (WX_USER_INTERFACE) */
#include "command/console.h"
#include "command/command_window.h"
#include "command/example_path.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_component_operations.h"
#include "computed_field/computed_field_compose.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_conditional.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_curve.h"
#include "computed_field/computed_field_deformation.h"
#include "computed_field/computed_field_derivatives.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_fibres.h"
#include "computed_field/computed_field_integration.h"
#include "computed_field/computed_field_logical_operators.h"
#include "computed_field/computed_field_lookup.h"
#include "computed_field/computed_field_matrix_operations.h"
#include "computed_field/computed_field_region_operations.h"
#include "computed_field/computed_field_sample_texture.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_string_constant.h"
#include "computed_field/computed_field_time.h"
#include "computed_field/computed_field_trigonometry.h"
#include "computed_field/computed_field_update.h"
#include "computed_field/computed_field_vector_operations.h"
#include "computed_field/computed_field_window_projection.h"
#include "computed_field/computed_field_wrappers.h"
#if defined (MOTIF)
#include "element/element_creator.h"
#endif /* defined (MOTIF) */
#include "element/element_operations.h"
#include "element/element_point_tool.h"
#if defined (MOTIF)
#include "element/element_point_viewer.h"
#endif /* defined (MOTIF) */
#if defined (WX_USER_INTERFACE)
#include "element/element_point_viewer_wx.h"
#endif /* defined (WX_USER_INTERFACE) */
#include "element/element_tool.h"
#include "emoter/emoter_dialog.h"
#include "finite_element/export_cm_files.h"
#include "finite_element/export_finite_element.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_conversion.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_to_graphics_object.h"
#include "finite_element/finite_element_to_iges.h"
#include "finite_element/finite_element_to_iso_lines.h"
#include "finite_element/finite_element_to_streamlines.h"
#if defined (MOTIF)
#include "finite_element/grid_field_calculator.h"
#endif /* defined (MOTIF) */
#include "finite_element/import_finite_element.h"
#include "finite_element/read_fieldml.h"
#include "finite_element/snake.h"
#include "finite_element/write_fieldml.h"
#include "general/debug.h"
#include "general/error_handler.h"
#include "general/image_utilities.h"
#include "general/io_stream.h"
#include "general/matrix_vector.h"
#include "general/multi_range.h"
#include "general/mystring.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/defined_graphics_objects.h"
#include "graphics/environment_map.h"
#include "graphics/glyph.h"
#include "graphics/graphical_element.h"
#include "graphics/graphics_object.h"
#include "graphics/graphics_window.h"
#include "graphics/import_graphics_object.h"
#include "graphics/iso_field_calculation.h"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "graphics/material.h"
#if defined (MOTIF)
#include "graphics/movie_graphics.h"
#if defined (NEW_ALIAS)
#include "graphics/renderalias.h"
#endif /* defined (NEW_ALIAS) */
#endif /* defined (MOTIF) */
#include "graphics/render_to_finite_elements.h"
#include "graphics/rendervrml.h"
#include "graphics/renderwavefront.h"
#include "graphics/scene.h"
#if defined (MOTIF) 
#include "graphics/scene_editor.h"
#elif defined (WX_USER_INTERFACE)
#include "graphics/scene_editor_wx.h"
#endif /* switch(USER_INTERFACE)*/
#include "graphics/spectrum.h"
#if defined (MOTIF)
#include "graphics/spectrum_editor.h"
#include "graphics/spectrum_editor_dialog.h"
#endif /* defined (MOTIF) */
#if defined (WX_USER_INTERFACE)
#include "graphics/spectrum_editor_wx.h"
#include "graphics/spectrum_editor_dialog_wx.h"
#endif /* defined (WX_USER_INTERFACE) */
#include "graphics/spectrum_settings.h"
#include "graphics/texture.h"
#include "graphics/transform_tool.h"
#include "graphics/userdef_objects.h"
#include "graphics/volume_texture.h"
#if defined (GTK_USER_INTERFACE)
#include "gtk/gtk_cmiss_scene_viewer.h"
#endif /* defined (GTK_USER_INTERFACE) */
#include "help/help_interface.h"
#include "image_processing/computed_field_image_resample.h"
#if defined (USE_ITK)
#include "image_processing/computed_field_thresholdFilter.h"
#include "image_processing/computed_field_binaryThresholdFilter.h"
#include "image_processing/computed_field_cannyEdgeDetectionFilter.h"
#include "image_processing/computed_field_meanImageFilter.h"
#include "image_processing/computed_field_sigmoidImageFilter.h"
#include "image_processing/computed_field_discreteGaussianImageFilter.h"
#include "image_processing/computed_field_curvatureAnisotropicDiffusionImageFilter.h"
#include "image_processing/computed_field_derivativeImageFilter.h"
#include "image_processing/computed_field_rescaleIntensityImageFilter.h"
#include "image_processing/computed_field_connected_threshold_image_filter.h"
#include "image_processing/computed_field_gradient_magnitude_recursive_gaussian_image_filter.h"
#include "image_processing/computed_field_fast_marching_image_filter.h"
#include "image_processing/computed_field_binary_dilate_image_filter.h"
#include "image_processing/computed_field_binary_erode_image_filter.h"
#endif /* defined (USE_ITK) */
#if defined (MOTIF)
#include "interaction/interactive_tool.h"
#include "interaction/select_tool.h"
#endif /* defined (MOTIF) */
#if defined (SELECT_DESCRIPTORS)
#include "io_devices/io_device.h"
#endif /* !defined (SELECT_DESCRIPTORS) */
#if defined (HAPTIC)
#include "io_devices/haptic_input_module.h"
#endif /* defined (HAPTIC) */
#if defined (MOTIF)
#include "io_devices/input_module_dialog.h"
#endif /* defined (MOTIF) */
#if defined (LINK_CMISS)
#include "link/cmiss.h"
#endif /* defined (LINK_CMISS) */
#if defined (MOTIF)
#include "material/material_editor_dialog.h"
#endif /* defined (MOTIF) */
#include "minimise/minimise.h"
#include "node/node_operations.h"
#include "node/node_tool.h"
#if defined (MOTIF)
#include "node/node_viewer.h"
#endif /* defined (MOTIF) */
#if defined (WX_USER_INTERFACE)
#include "node/node_viewer_wx.h"
#endif /* defined (MOTIF) */
#include "region/cmiss_region.h"
#include "selection/any_object_selection.h"
#if defined (MOTIF)
#include "three_d_drawing/movie_extensions.h"
#endif /* defined (MOTIF) */
#include "three_d_drawing/graphics_buffer.h"
#include "graphics/font.h"
#if defined (MOTIF)
#include "time/time_editor_dialog.h"
#endif /* defined (MOTIF) */
#include "time/time_keeper.h"
#include "user_interface/filedir.h"
#include "user_interface/confirmation.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "curve/curve.h"
#if defined (MOTIF)
#include "curve/curve_editor_dialog.h"
#include "view/coord_trans.h"
#endif /* defined (MOTIF) */
#if defined (PERL_INTERPRETER)
#include "perl_interpreter.h"
#endif /* defined (PERL_INTERPRETER) */
#if defined (UNEMAP)
#include "unemap_application/unemap_command.h"
#endif /* defined (UNEMAP) */
#include "user_interface/fd_io.h"
#include "user_interface/idle.h"
#include "user_interface/timer.h"
#include "command/cmiss.h"

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
	char *cm_examples_directory,*cm_parameters_file_name,*example_directory,
		*examples_directory,*example_comfile,
		*example_requirements,*help_directory,*help_url;
#if defined (CELL)
	struct Cell_interface *cell_interface;
#endif /* defined (CELL) */
	struct Console *command_console;
#if defined (USE_CMGUI_COMMAND_WINDOW)
	struct Command_window *command_window;
#endif /* USE_CMGUI_COMMAND_WINDOW */
	struct Colour background_colour,foreground_colour;
	struct Execute_command *execute_command,*set_command;
	 struct Element_point_tool *element_point_tool;
	 struct Element_tool *element_tool;
	struct Event_dispatcher *event_dispatcher;
	 struct Node_tool *data_tool,*node_tool;
#if defined (MOTIF)
	struct Select_tool *select_tool;
#endif /* defined (MOTIF) */
	struct Interactive_tool *transform_tool;
#if defined (PERL_INTERPRETER)
	struct Interpreter *interpreter;
#endif /* defined (PERL_INTERPRETER) */
#if defined (SELECT_DESCRIPTORS)
	struct LIST(Io_device) *device_list;
#endif /* defined (SELECT_DESCRIPTORS) */
	/*???RC.  Single list of graphics objects - eventually set up manager ? */
	struct LIST(GT_object) *graphics_object_list;
	/* list of glyphs = simple graphics objects with only geometry */
	struct LIST(GT_object) *glyph_list;
#if defined (MOTIF) || (WX_USER_INTERFACE)
	struct MANAGER(Comfile_window) *comfile_window_manager;
#endif /* defined (MOTIF) || (WX_USER_INTERFACE)*/
	struct Cmiss_region *root_region;
		/*???RC data_root_region is temporary until data is removed */
	struct Cmiss_region *data_root_region;
	struct Computed_field_package *computed_field_package;
	struct MANAGER(Environment_map) *environment_map_manager;
	struct MANAGER(FE_basis) *basis_manager;
	struct LIST(FE_element_shape) *element_shape_list;
	/* Always want the entry for graphics_buffer_package even if it will
		not be available on this implementation */
	struct Graphics_buffer_package *graphics_buffer_package;
	struct Cmiss_scene_viewer_package *scene_viewer_package;
	struct Graphics_font_package *graphics_font_package;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
	struct MANAGER(Graphics_window) *graphics_window_manager;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct IO_stream_package *io_stream_package;
	struct MANAGER(Light) *light_manager;
	struct Light *default_light;
	struct MANAGER(Light_model) *light_model_manager;
	struct Light_model *default_light_model;
	 struct Material_package *material_package;
	struct Graphics_font *default_font;
#if defined (SGI_MOVIE_FILE) && defined (MOTIF)
	struct MANAGER(Movie_graphics) *movie_graphics_manager;
#endif /* defined (SGI_MOVIE_FILE) && defined (MOTIF) */
	struct MANAGER(Texture) *texture_manager;
	struct MANAGER(Curve) *curve_manager;
	struct MANAGER(Scene) *scene_manager;
	struct Scene *default_scene;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;
	struct Scene_object_transformation_data *scene_object_transformation_data;
#if defined (MOTIF)
	struct Prompt_window *prompt_window;
	struct Projection_window *projection_window;
#endif /* defined (MOTIF) */
	/* global list of selected objects */
	struct Any_object_selection *any_object_selection;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct FE_element_selection *element_selection;
	struct FE_node_selection *data_selection,*node_selection;
#if defined (MOTIF)
	struct Socket *command_socket;
	XtInputId command_socket_input_id;
#endif /* defined (MOTIF) */
	struct Spectrum *default_spectrum;
	struct Streampoint *streampoint_list;
	struct Time_keeper *default_time_keeper;
	struct User_interface *user_interface;
	struct Emoter_dialog *emoter_slider_dialog;
#if defined (MOTIF)
	Widget curve_editor_dialog,data_grabber_dialog,
		grid_field_calculator_dialog,input_module_dialog,
		sync_2d_3d_dialog;
	struct Node_viewer *data_viewer,*node_viewer;
	struct Element_point_viewer *element_point_viewer;
	struct Element_creator *element_creator;
	struct Material_editor_dialog *material_editor_dialog;
	struct Time_editor_dialog *time_editor_dialog;
#endif /* defined (MOTIF) */
#if defined (WX_USER_INTERFACE)
	struct Node_viewer *data_viewer,*node_viewer;
	struct Element_point_viewer *element_point_viewer;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (MOTIF) || defined (WX_USER_INTERFACE)
	struct Scene_editor *scene_editor;
	struct Spectrum_editor_dialog *spectrum_editor_dialog;
#endif /* defined (MOTIF) || defined (WX_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
	HINSTANCE hInstance;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (UNEMAP)
	struct Unemap_command_data *unemap_command_data;
#endif /* defined (UNEMAP) */
}; /* struct Cmiss_command_data */

typedef struct
/*******************************************************************************
LAST MODIFIED : 12 December 1996

DESCRIPTION :
==============================================================================*/
{
#if defined (MOTIF)
	Pixel background_colour,foreground_colour;
#endif /* defined (MOTIF) */
	char *examples_directory,*help_directory,*help_url,*startup_comfile;
} User_settings;

struct Startup_material_definition
{
	char *name;
	MATERIAL_PRECISION ambient[3];
	MATERIAL_PRECISION diffuse[3];
	MATERIAL_PRECISION emission[3];
	MATERIAL_PRECISION specular[3];
	MATERIAL_PRECISION alpha;
	MATERIAL_PRECISION shininess;
};

/*
Module variables
----------------
*/
#if defined (LINK_CMISS)
/*???GMH.  This is a hack - when we register it will disappear (used in
	user_interface.c) */
struct CMISS_connection *CMISS = (struct CMISS_connection *)NULL;
#endif /* LINK_CMISS */

/* only the default material is not in this list because its colour changes
	 to contrast with the background; colours are R G B */
struct Startup_material_definition
	startup_materials[] =
{
	{"black",
	 /*ambient*/ { 0.00, 0.00, 0.00},
	 /*diffuse*/ { 0.00, 0.00, 0.00},
	 /*emission*/{ 0.00, 0.00, 0.00},
	 /*specular*/{ 0.30, 0.30, 0.30},
	 /*alpha*/1.0,
	 /*shininess*/0.2},
	{"blue",
	 /*ambient*/ { 0.00, 0.00, 0.50},
	 /*diffuse*/ { 0.00, 0.00, 1.00},
	 /*emission*/{ 0.00, 0.00, 0.00},
	 /*specular*/{ 0.20, 0.20, 0.20},
	 /*alpha*/1.0,
	 /*shininess*/0.2},
	{"bone",
	 /*ambient*/ { 0.70, 0.70, 0.60},
	 /*diffuse*/ { 0.90, 0.90, 0.70},
	 /*emission*/{ 0.00, 0.00, 0.00},
	 /*specular*/{ 0.10, 0.10, 0.10},
	 /*alpha*/1.0,
	 /*shininess*/0.2},
	{"gray50",
	 /*ambient*/ { 0.50, 0.50, 0.50},
	 /*diffuse*/ { 0.50, 0.50, 0.50},
	 /*emission*/{ 0.50, 0.50, 0.50},
	 /*specular*/{ 0.50, 0.50, 0.50},
	 /*alpha*/1.0,
	 /*shininess*/0.2},
	{"gold",
	 /*ambient*/ { 1.00, 0.40, 0.00},
	 /*diffuse*/ { 1.00, 0.70, 0.00},
	 /*emission*/{ 0.00, 0.00, 0.00},
	 /*specular*/{ 0.50, 0.50, 0.50},
	 /*alpha*/1.0,
	 /*shininess*/0.3},
	{"green",
	 /*ambient*/ { 0.00, 0.50, 0.00},
	 /*diffuse*/ { 0.00, 1.00, 0.00},
	 /*emission*/{ 0.00, 0.00, 0.00},
	 /*specular*/{ 0.20, 0.20, 0.20},
	 /*alpha*/1.0,
	 /*shininess*/0.1},
	{"muscle",
	 /*ambient*/ { 0.40, 0.14, 0.11},
	 /*diffuse*/ { 0.50, 0.12, 0.10},
	 /*emission*/{ 0.00, 0.00, 0.00},
	 /*specular*/{ 0.30, 0.50, 0.50},
	 /*alpha*/1.0,
	 /*shininess*/0.2},
	{"red",
	 /*ambient*/ { 0.50, 0.00, 0.00},
	 /*diffuse*/ { 1.00, 0.00, 0.00},
	 /*emission*/{ 0.00, 0.00, 0.00},
	 /*specular*/{ 0.20, 0.20, 0.20},
	 /*alpha*/1.0,
	 /*shininess*/0.2},
	{"silver",
	 /*ambient*/ { 0.40, 0.40, 0.40},
	 /*diffuse*/ { 0.70, 0.70, 0.70},
	 /*emission*/{ 0.00, 0.00, 0.00},
	 /*specular*/{ 0.50, 0.50, 0.50},
	 /*alpha*/1.0,
	 /*shininess*/0.3},
	{"tissue",
	 /*ambient*/ { 0.90, 0.70, 0.50},
	 /*diffuse*/ { 0.90, 0.70, 0.50},
	 /*emission*/{ 0.00, 0.00, 0.00},
	 /*specular*/{ 0.20, 0.20, 0.30},
	 /*alpha*/1.0,
	 /*shininess*/0.2},
	/* Used as the default fail_material for texture evaluation. */
	{"transparent_gray50",
	 /*ambient*/ { 0.50, 0.50, 0.50},
	 /*diffuse*/ { 0.50, 0.50, 0.50},
	 /*emission*/{ 0.50, 0.50, 0.50},
	 /*specular*/{ 0.50, 0.50, 0.50},
	 /*alpha*/0.0,
	 /*shininess*/0.2},
	{"white",
	 /*ambient*/ { 1.00, 1.00, 1.00},
	 /*diffuse*/ { 1.00, 1.00, 1.00},
	 /*emission*/{ 0.00, 0.00, 0.00},
	 /*specular*/{ 0.00, 0.00, 0.00},
	 /*alpha*/1.0,
	 /*shininess*/0.0}
};

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
	if (wx_interactive_tool=
		FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(
		(char *)tool_name,interactive_tool_manager))
	{
		Interactive_tool_copy(wx_interactive_tool,
			global_interactive_tool, (struct MANAGER(Interactive_tool) *)NULL);
	}
    DEALLOCATE(tool_name);
	return 1;
}
#endif /*(WX_USER_INTERFACE)*/

#if defined (WX_USER_INTERFACE) && (__WIN32__)
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

static int set_command_prompt(char *prompt, struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 26 June 2002

DESCRIPTION :
Changes the command prompt provided to the user.
==============================================================================*/
{
	int return_code;

	ENTER(set_command_prompt);
	if (prompt && command_data)
	{
#if defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE)
		if (command_data->command_window)
		{
			return_code = Command_window_set_command_prompt(command_data->command_window,
				prompt);
		}
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
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

static int gfx_change_identifier(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 January 2003

DESCRIPTION :
==============================================================================*/
{
	char data_flag, element_flag, face_flag, line_flag, node_flag, *region_path;
	FE_value time;
	int data_offset, element_offset, face_offset, line_offset, node_offset,
		return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct Computed_field *sort_by_field;
	struct FE_region *fe_region;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_sort_by_field_data;

	ENTER(gfx_change_identifier);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		Cmiss_region_get_root_region_path(&region_path);
		data_flag = 0;
		data_offset = 0;
		element_flag = 0;
		element_offset = 0;
		face_flag = 0;
		face_offset = 0;
		line_flag = 0;
		line_offset = 0;
		node_flag = 0;
		node_offset = 0;
		sort_by_field = (struct Computed_field *)NULL;
		if (command_data->default_time_keeper)
		{
			time = Time_keeper_get_time(command_data->default_time_keeper);
		}
		else
		{
			time = 0;
		}

		option_table = CREATE(Option_table)();
		/* data_offset */
		Option_table_add_entry(option_table, "data_offset", &data_offset,
			&data_flag, set_int_and_char_flag);
		/* element_offset */
		Option_table_add_entry(option_table, "element_offset", &element_offset,
			&element_flag, set_int_and_char_flag);
		/* face_offset */
		Option_table_add_entry(option_table, "face_offset", &face_offset,
			&face_flag, set_int_and_char_flag);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* line_offset */
		Option_table_add_entry(option_table, "line_offset", &line_offset,
			&line_flag, set_int_and_char_flag);
		/* node_offset */
		Option_table_add_entry(option_table, "node_offset", &node_offset,
			&node_flag, set_int_and_char_flag);
		/* sort_by */
		set_sort_by_field_data.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_sort_by_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_sort_by_field_data.conditional_function_user_data = (void *)NULL;
		Option_table_add_entry(option_table, "sort_by", &sort_by_field,
			&set_sort_by_field_data, set_Computed_field_conditional);
		/* time */
		Option_table_add_entry(option_table, "time", &time, NULL, set_FE_value);

		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (data_flag)
			{
				if (Cmiss_region_get_region_from_path(command_data->data_root_region,
					region_path, &region) &&
					(fe_region = Cmiss_region_get_FE_region(region)))
				{
					FE_region_begin_change(fe_region);
					if (!FE_region_change_node_identifiers(fe_region,
						data_offset, sort_by_field, time))
					{
						return_code = 0;
					}
					FE_region_end_change(fe_region);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_change_identifier.  Invalid data region");
					return_code = 0;
				}
			}
			if (element_flag || face_flag || line_flag || node_flag)
			{
				if (Cmiss_region_get_region_from_path(command_data->root_region,
					region_path, &region) &&
					(fe_region = Cmiss_region_get_FE_region(region)))
				{
					FE_region_begin_change(fe_region);
					if (element_flag)
					{
						if (!FE_region_change_element_identifiers(fe_region,
							CM_ELEMENT,	element_offset, sort_by_field, time))
						{
							return_code = 0;
						}
					}
					if (face_flag)
					{
						if (!FE_region_change_element_identifiers(fe_region,
							CM_FACE,	face_offset, sort_by_field, time))
						{
							return_code = 0;
						}
					}
					if (line_flag)
					{
						if (!FE_region_change_element_identifiers(fe_region,
							CM_LINE,	line_offset, sort_by_field, time))
						{
							return_code = 0;
						}
					}
					if (node_flag)
					{
						if (!FE_region_change_node_identifiers(fe_region,
							node_offset, sort_by_field, time))
						{
							return_code = 0;
						}
					}
					FE_region_end_change(fe_region);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_change_identifier.  Invalid region");
					return_code = 0;
				}
			}
		}
		DESTROY(Option_table)(&option_table);
		DEALLOCATE(region_path);
		if (sort_by_field)
		{
			DEACCESS(Computed_field)(&sort_by_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_change_identifier.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_change_identifier */

static int gfx_create_annotation(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 2001

DESCRIPTION :
Executes a GFX CREATE ANNOTATION command. Creates a graphics object containing
a single point in 3-D space with a text string drawn beside it.
==============================================================================*/
{
	char *annotation_text,*font_name,*graphics_object_name,**text;
	float time;
	int number_of_components,return_code;
	struct Cmiss_command_data *command_data;
	struct Graphical_material *material;
	struct Graphics_font *font;
	struct GT_object *graphics_object;
	struct GT_pointset *point_set;
	struct Option_table *option_table;
	Triple *pointlist,position;

	ENTER(gfx_create_annotation);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			graphics_object_name = duplicate_string("annotation");
			/* must access it now, because we deaccess it later */
			material=
				ACCESS(Graphical_material)(Material_package_get_default_material(command_data->material_package));
			position[0]=0.0;
			position[1]=0.0;
			position[2]=0.0;
			annotation_text = duplicate_string("\"annotation text\"");
			time=0.0;
			font_name = (char *)NULL;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* font */
			Option_table_add_name_entry(option_table, "font",
				&font_name);
			/* material */
			Option_table_add_set_Material_entry(option_table, "material", &material,
				command_data->material_package);
			/* position */
			number_of_components=3;
			Option_table_add_entry(option_table,"position",position,
				&number_of_components,set_float_vector);
			/* text */
			Option_table_add_entry(option_table,"text",&annotation_text,
				(void *)1,set_name);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			return_code=Option_table_multi_parse(option_table,state);
			if (return_code)
			{
				if (annotation_text)
				{
					if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
						graphics_object_name,command_data->graphics_object_list))
					{
						if (g_POINTSET==GT_object_get_type(graphics_object))
						{
							if (GT_object_has_time(graphics_object,time))
							{
								display_message(WARNING_MESSAGE,
									"Overwriting time %g in graphics object '%s'",time,
									graphics_object_name);
								return_code = GT_object_remove_primitives_at_time(
									graphics_object, time,
									(GT_object_primitive_object_name_conditional_function *)NULL,
									(void *)NULL);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Object of different type named '%s' already exists",
								graphics_object_name);
							return_code=0;
						}
					}
					else
					{
						if (!((graphics_object=CREATE(GT_object)(graphics_object_name,
							g_POINTSET,material))&&
							ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list)))
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_points.  Could not create graphics object");
							DESTROY(GT_object)(&graphics_object);
							return_code=0;
						}
					}
					if (return_code)
					{
						/* create the pointset used to display the annotation */
						pointlist=(Triple *)NULL;
						text=(char **)NULL;
						point_set=(struct GT_pointset *)NULL;
						if (ALLOCATE(pointlist,Triple,1)&&ALLOCATE(text,char *,1))
						{
							*text = annotation_text;
							(*pointlist)[0]=position[0];
							(*pointlist)[1]=position[1];
							(*pointlist)[2]=position[2];
							if (!(font_name && (font = ACCESS(Graphics_font)
								(Graphics_font_package_get_font(command_data->graphics_font_package, font_name)))))
							{
								font = ACCESS(Graphics_font)(command_data->default_font);
							}
							if ((point_set=CREATE(GT_pointset)(1,pointlist,text,g_NO_MARKER,
								0.0,g_NO_DATA,(GTDATA *)NULL,(int *)NULL,font))&&
								GT_OBJECT_ADD(GT_pointset)(graphics_object,time,point_set))
							{
								return_code = 1;
							}
							else
							{
								return_code = 0;
							}
							DEACCESS(Graphics_font)(&font);
						}
						else
						{
							return_code=0;
						}
						if (return_code)
						{
							/* annotation string now owned by point set */
							annotation_text=(char *)NULL;
						}
						else
						{
							display_message(WARNING_MESSAGE,"Could not create annotation");
							if (point_set)
							{
								DESTROY(GT_pointset)(&point_set);
							}
							else
							{
								DEALLOCATE(pointlist);
								DEALLOCATE(text);
							}
							if (0==GT_object_get_number_of_times(graphics_object))
							{
								REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
									command_data->graphics_object_list);
							}
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing annotation text");
					return_code=0;
				}
			}
			DESTROY(Option_table)(&option_table);
			if (annotation_text)
			{
				DEALLOCATE(annotation_text);
			}
			DEACCESS(Graphical_material)(&material);
			DEALLOCATE(graphics_object_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_annotation.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_annotation.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_annotation */

static int gfx_create_axes(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Executes a GFX CREATE AXES command. Creates a graphics object containing
a single point in 3-D space with an axes glyph.
==============================================================================*/
{
	char *glyph_name, *graphics_object_name;
	float time;
	int number_of_components, return_code;
	struct Cmiss_command_data *command_data;
	struct Graphical_material *material;
	struct GT_object *glyph, *graphics_object;
	struct GT_glyph_set *glyph_set;
	struct Option_table *option_table;
	Triple axis_lengths, *axis1_list, *axis2_list, *axis3_list, axis_origin,
		*point_list, *scale_list;

	ENTER(gfx_create_axes);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		graphics_object_name = duplicate_string("axes");
		glyph = (struct GT_object *)NULL;
		glyph_name = duplicate_string("axes_xyz");
		material =
			ACCESS(Graphical_material)(Material_package_get_default_material(command_data->material_package));
		axis_origin[0] = 0.0;
		axis_origin[1] = 0.0;
		axis_origin[2] = 0.0;
		axis_lengths[0] = 1.0;
		axis_lengths[1] = 1.0;
		axis_lengths[2] = 1.0;
		time = 0.0;

		option_table=CREATE(Option_table)();
		/* as */
		Option_table_add_entry(option_table, "as", &graphics_object_name,
			(void *)1, set_name);
		/* glyph */
		Option_table_add_entry(option_table, "glyph", &glyph_name,
			(void *)1, set_name);
		/* material */
		Option_table_add_set_Material_entry(option_table, "material", &material,
			command_data->material_package);
		/* lengths */
		Option_table_add_entry(option_table, "lengths", axis_lengths,
			"*", set_special_float3);
		/* origin */
		number_of_components = 3;
		Option_table_add_entry(option_table, "origin", axis_origin,
			&number_of_components, set_float_vector);
		/* time */
		Option_table_add_entry(option_table, "time", &time, NULL, set_float);
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code)
		{
			if (graphics_object = FIND_BY_IDENTIFIER_IN_LIST(GT_object, name)(
				graphics_object_name, command_data->graphics_object_list))
			{
				if (g_GLYPH_SET == GT_object_get_type(graphics_object))
				{
					if (GT_object_has_time(graphics_object, time))
					{
						display_message(WARNING_MESSAGE,
							"Overwriting time %g in graphics object '%s'", time,
							graphics_object_name);
						return_code = GT_object_remove_primitives_at_time(
							graphics_object, time,
							(GT_object_primitive_object_name_conditional_function *)NULL,
							(void *)NULL);
					}
					if (material != get_GT_object_default_material(graphics_object))
					{
						set_GT_object_default_material(graphics_object, material);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Object of different type named '%s' already exists",
						graphics_object_name);
					return_code = 0;
				}
			}
			else
			{
				if (!((graphics_object = CREATE(GT_object)(graphics_object_name,
								g_GLYPH_SET, material)) &&
					ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list)))
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_points.  Could not create graphics object");
					DESTROY(GT_object)(&graphics_object);
					return_code = 0;
				}
			}
			if (return_code)
			{
				if (glyph = FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(glyph_name,
					command_data->glyph_list))
				{
					ACCESS(GT_object)(glyph);
				}
			}

			if (return_code)
			{
				/* create the pointset used to display the axes */
				glyph_set = (struct GT_glyph_set *)NULL;

				ALLOCATE(point_list, Triple, 1);
				ALLOCATE(axis1_list, Triple, 1);
				ALLOCATE(axis2_list, Triple, 1);
				ALLOCATE(axis3_list, Triple, 1);
				ALLOCATE(scale_list, Triple, 1);

				if (point_list && axis1_list && axis2_list && axis3_list &&
					scale_list && (glyph_set = CREATE(GT_glyph_set)(/*number_of_points*/1,
					point_list, axis1_list, axis2_list, axis3_list, scale_list, glyph, command_data->default_font,
					/*labels*/(char **)NULL, /*n_data_components*/0, /*data*/(GTDATA *)NULL,
					/*label_bounds_dimension*/0, /*label_bounds_components*/0, /*label_bounds*/(float *)NULL,
					/*object_name*/0, /*names*/(int *)NULL)))
				{
					(*point_list)[0] = axis_origin[0];
					(*point_list)[1] = axis_origin[1];
					(*point_list)[2] = axis_origin[2];
					(*axis1_list)[0] = axis_lengths[0];
					(*axis1_list)[1] = 0.0;
					(*axis1_list)[2] = 0.0;
					(*axis2_list)[0] = 0.0;
					(*axis2_list)[1] = axis_lengths[1];
					(*axis2_list)[2] = 0.0;
					(*axis3_list)[0] = 0.0;
					(*axis3_list)[1] = 0.0;
					(*axis3_list)[2] = axis_lengths[2];
					(*scale_list)[0] = 1.0;
					(*scale_list)[1] = 1.0;
					(*scale_list)[2] = 1.0;
					if (!GT_OBJECT_ADD(GT_glyph_set)(graphics_object, time, glyph_set))
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_axes.  Could not add axes graphics object");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_axes.  Could not create axes");
					DEALLOCATE(point_list);
					DEALLOCATE(axis1_list);
					DEALLOCATE(axis2_list);
					DEALLOCATE(axis3_list);
					DEALLOCATE(scale_list);
					return_code = 0;
				}
				if ((!return_code) &&
					(0 == GT_object_get_number_of_times(graphics_object)))
				{
					REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list);
				}
			}
		}
		DESTROY(Option_table)(&option_table);
		DEACCESS(Graphical_material)(&material);
		DEALLOCATE(graphics_object_name);
		if (glyph)
		{
			DEACCESS(GT_object)(&glyph);
		}
		DEALLOCATE(glyph_name);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_create_axes.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_axes */

static int gfx_create_colour_bar(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2001

DESCRIPTION :
Executes a GFX CREATE COLOUR_BAR command. Creates a colour bar graphics object
with tick marks and labels for showing the scale of a spectrum.
==============================================================================*/
{
	char *font_name, *graphics_object_name,*number_format;
	float bar_length,bar_radius,extend_length,tick_length;
	int number_of_components,return_code,tick_divisions;
	struct Cmiss_command_data *command_data;
	struct Graphical_material *label_material,*material;
	struct Graphics_font *font;
	struct GT_object *graphics_object;
	struct Option_table *option_table;
	struct Spectrum *spectrum;
	Triple bar_axis,bar_centre,side_axis;

	ENTER(gfx_create_colour_bar);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			graphics_object_name = duplicate_string("colour_bar");
			number_format = duplicate_string("%+.4e");
			/* must access it now, because we deaccess it later */
			label_material=
				ACCESS(Graphical_material)(Material_package_get_default_material(command_data->material_package));
			material=
				ACCESS(Graphical_material)(Material_package_get_default_material(command_data->material_package));
			spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
			number_of_components=3;
			bar_centre[0]=-0.9;
			bar_centre[1]=0.0;
			bar_centre[2]=0.5;
			bar_axis[0]=0.0;
			bar_axis[1]=1.0;
			bar_axis[2]=0.0;
			side_axis[0]=1.0;
			side_axis[1]=0.0;
			side_axis[2]=0.0;
			bar_length=1.6;
			extend_length=0.06;
			bar_radius=0.06;
			tick_length=0.04;
			tick_divisions=10;
			font_name = (char *)NULL;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&graphics_object_name,
				(void *)1,set_name);
			/* axis */
			Option_table_add_entry(option_table,"axis",bar_axis,
				&number_of_components,set_float_vector);
			/* centre */
			Option_table_add_entry(option_table,"centre",bar_centre,
				&number_of_components,set_float_vector);
			/* divisions */
			Option_table_add_entry(option_table,"divisions",&tick_divisions,
				NULL,set_int_non_negative);
			/* extend_length */
			Option_table_add_entry(option_table,"extend_length",&extend_length,
				NULL,set_float_non_negative);
			/* font */
			Option_table_add_name_entry(option_table, "font",
				&font_name);
			/* label_material */
			Option_table_add_set_Material_entry(option_table, "label_material", &label_material,
				command_data->material_package);
			/* length */
			Option_table_add_entry(option_table,"length",&bar_length,
				NULL,set_float_positive);
			/* number_format */
			Option_table_add_entry(option_table,"number_format",&number_format,
				(void *)1,set_name);
			/* material */
			Option_table_add_set_Material_entry(option_table, "material", &material,
				command_data->material_package);
			/* radius */
			Option_table_add_entry(option_table,"radius",&bar_radius,
				NULL,set_float_positive);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* tick_direction */
			Option_table_add_entry(option_table,"tick_direction",side_axis,
				&number_of_components,set_float_vector);
			/* tick_length */
			Option_table_add_entry(option_table,"tick_length",&tick_length,
				NULL,set_float_non_negative);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				if (100 < tick_divisions)
				{
					display_message(WARNING_MESSAGE,"Limited to 100 tick_divisions");
					tick_divisions=100;
				}
				if (!(font_name && (font = ACCESS(Graphics_font)
					(Graphics_font_package_get_font(command_data->graphics_font_package, font_name)))))
				{
					font = ACCESS(Graphics_font)(command_data->default_font);
				}
				/* try to find existing colour_bar for updating */
				graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list);
				if (create_Spectrum_colour_bar(&graphics_object,
						graphics_object_name,spectrum,/*component_number*/0,
						bar_centre,bar_axis,side_axis,
						bar_length,bar_radius,extend_length,tick_divisions,tick_length,
						number_format,material,label_material, font))
				{
					ACCESS(GT_object)(graphics_object);
					if (IS_OBJECT_IN_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list) ||
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_colour_bar.  Could not add graphics object to list");
						return_code=0;
					}
					DEACCESS(GT_object)(&graphics_object);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_colour_bar.  Could not create colour bar");
					return_code=0;
				}
				DEACCESS(Graphics_font)(&font);
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			DEACCESS(Graphical_material)(&label_material);
			DEACCESS(Graphical_material)(&material);
			DEACCESS(Spectrum)(&spectrum);
			DEALLOCATE(graphics_object_name);
			DEALLOCATE(number_format);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_colour_bar.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_colour_bar.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_colour_bar */

static int gfx_create_cylinders(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 January 2003

DESCRIPTION :
Executes a GFX CREATE CYLINDERS command.
==============================================================================*/
{
	char exterior_flag, *graphics_object_name, *region_path;
	float constant_radius,scale_factor,time;
	gtObject *graphics_object;
	int face_number,return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct Element_discretization discretization;
	struct Element_to_cylinder_data element_to_cylinder_data;
	struct Computed_field *coordinate_field, *data_field, *radius_field,
		*texture_coordinate_field;
	struct FE_region *fe_region;
	struct Graphical_material *material;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_radius_field_data,
		set_texture_coordinate_field_data;
	struct Spectrum *spectrum;

	ENTER(gfx_create_cylinders);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		graphics_object_name = duplicate_string("cylinders");
		Cmiss_region_get_root_region_path(&region_path);
		constant_radius=0.0;
		scale_factor=1.0;
		coordinate_field=(struct Computed_field *)NULL;
		data_field=(struct Computed_field *)NULL;
		radius_field=(struct Computed_field *)NULL;
		texture_coordinate_field=(struct Computed_field *)NULL;
		time=0;
		/* must access it now, because we deaccess it later */
		material=
			ACCESS(Graphical_material)(Material_package_get_default_material(command_data->material_package));
		spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
		discretization.number_in_xi1=4;
		discretization.number_in_xi2=6;
		discretization.number_in_xi3=0;
		exterior_flag=0;
		face_number=-1;

		option_table=CREATE(Option_table)();
		/* as */
		Option_table_add_entry(option_table,"as",&graphics_object_name,
			(void *)1,set_name);
		/* constant_radius */
		Option_table_add_entry(option_table,"constant_radius",&constant_radius,
			NULL,set_float);
		/* coordinate */
		set_coordinate_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_coordinate_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"coordinate",&coordinate_field,
			&set_coordinate_field_data,set_Computed_field_conditional);
		/* data */
		set_data_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_data_field_data.conditional_function=
			Computed_field_has_numerical_components;
		set_data_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"data",&data_field,
			&set_data_field_data,set_Computed_field_conditional);
		/* exterior */
		Option_table_add_entry(option_table,"exterior",&exterior_flag,
			NULL,set_char_flag);
		/* face */
		Option_table_add_entry(option_table,"face",&face_number,
			NULL,set_exterior);
		/* from */
		Option_table_add_entry(option_table, "from", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* material */
		Option_table_add_set_Material_entry(option_table, "material", &material,
			command_data->material_package);
		/* radius_scalar */
		set_radius_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_radius_field_data.conditional_function=Computed_field_is_scalar;
		set_radius_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"radius_scalar",&radius_field,
			&set_radius_field_data,set_Computed_field_conditional);
		/* scale_factor */
		Option_table_add_entry(option_table,"scale_factor",&scale_factor,
			NULL,set_float);
		/* spectrum */
		Option_table_add_entry(option_table,"spectrum",&spectrum,
			command_data->spectrum_manager,set_Spectrum);
		/* texture_coordinates */
		set_texture_coordinate_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_texture_coordinate_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_texture_coordinate_field_data.conditional_function_user_data=
			(void *)NULL;
		Option_table_add_entry(option_table,"texture_coordinates",
			&texture_coordinate_field,&set_texture_coordinate_field_data,
			set_Computed_field_conditional);
		/* time */
		Option_table_add_entry(option_table,"time",&time,NULL,set_float);
		/* with */
		Option_table_add_entry(option_table,"with",&discretization,
			command_data->user_interface,set_Element_discretization);
		return_code=Option_table_multi_parse(option_table,state);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (!(Cmiss_region_get_region_from_path(command_data->root_region,
				region_path, &region) &&
				(fe_region = Cmiss_region_get_FE_region(region))))
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_cylinders.  Invalid region");
				return_code = 0;
			}
			if (!coordinate_field)
			{
				display_message(WARNING_MESSAGE, "Must specify a coordinate field");
				return_code = 0;
			}
		}
		if (return_code)
		{
			if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
				graphics_object_name,command_data->graphics_object_list))
			{
				if (g_SURFACE==GT_object_get_type(graphics_object))
				{
					if (GT_object_has_time(graphics_object,time))
					{
						display_message(WARNING_MESSAGE,
							"Overwriting time %g in graphics object '%s'",time,
							graphics_object_name);
						return_code = GT_object_remove_primitives_at_time(
							graphics_object, time,
							(GT_object_primitive_object_name_conditional_function *)NULL,
							(void *)NULL);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Object of different type named '%s' already exists",
						graphics_object_name);
					return_code=0;
				}
			}
			else
			{
				if (!((graphics_object=CREATE(GT_object)(graphics_object_name,
					g_SURFACE,material))&&
					ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list)))
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_cylinders.  Could not create graphics object");
					DESTROY(GT_object)(&graphics_object);
					return_code=0;
				}
			}
		}
		if (return_code)
		{
			element_to_cylinder_data.coordinate_field=
				Computed_field_begin_wrap_coordinate_field(coordinate_field);
			element_to_cylinder_data.fe_region = fe_region;
			element_to_cylinder_data.exterior=exterior_flag;
			element_to_cylinder_data.face_number=face_number;
			/* radius = constant_radius + scale_factor*radius_scalar */
			element_to_cylinder_data.constant_radius=constant_radius;
			element_to_cylinder_data.radius_field=radius_field;
			element_to_cylinder_data.texture_coordinate_field =
				texture_coordinate_field;
			element_to_cylinder_data.scale_factor=scale_factor;
			element_to_cylinder_data.graphics_object=graphics_object;
			element_to_cylinder_data.time=time;
			element_to_cylinder_data.material=material;
			element_to_cylinder_data.data_field=data_field;
			element_to_cylinder_data.number_of_segments_along=
				discretization.number_in_xi1;
			element_to_cylinder_data.number_of_segments_around=
				discretization.number_in_xi2;
			return_code = FE_region_for_each_FE_element(fe_region,
				element_to_cylinder,(void *)&element_to_cylinder_data);
			if (return_code)
			{
				if (!GT_object_has_time(graphics_object,time))
				{
					/* add a NULL surface to make an empty time */
					GT_OBJECT_ADD(GT_surface)(graphics_object,time,
						(struct GT_surface *)NULL);
				}
				if (data_field)
				{
					return_code=set_GT_object_Spectrum(graphics_object,spectrum);
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,"No cylinders created");
				if (0==GT_object_get_number_of_times(graphics_object))
				{
					REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list);
				}
			}
			Computed_field_end_wrap(&(element_to_cylinder_data.coordinate_field));
		}
		DESTROY(Option_table)(&option_table);
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		if (data_field)
		{
			DEACCESS(Computed_field)(&data_field);
		}
		if (radius_field)
		{
			DEACCESS(Computed_field)(&radius_field);
		}
		if (texture_coordinate_field)
		{
			DEACCESS(Computed_field)(&texture_coordinate_field);
		}
		DEALLOCATE(region_path);
		DEACCESS(Graphical_material)(&material);
		DEACCESS(Spectrum)(&spectrum);
		DEALLOCATE(graphics_object_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_cylinders.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_cylinders */

#if defined (MOTIF) || defined (WX_USER_INTERFACE)
static int gfx_create_element_creator(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 January 2003

DESCRIPTION :
Executes a GFX CREATE ELEMENT_CREATOR command.
==============================================================================*/
{
	int return_code;
#if defined (MOTIF)
	char *current_token, *initial_region_path;
	struct Cmiss_command_data *command_data;
#endif /* defined (MOTIF) */
	USE_PARAMETER(dummy_to_be_modified);
	ENTER(gfx_create_element_creator);
#if defined (MOTIF)

	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (command_data->element_creator)
				{
					return_code = Element_creator_bring_window_to_front(
						command_data->element_creator);
				}
				else
				{
					Cmiss_region_get_root_region_path(&initial_region_path);
					if (CREATE(Element_creator)(&(command_data->element_creator),
						command_data->root_region, initial_region_path,
						command_data->element_selection, command_data->node_selection,
						command_data->user_interface))
					{
						return_code = 1;
					}
					else
					{
						return_code = 0;
					}
					DEALLOCATE(initial_region_path);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_element_creator.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_element_creator.  Missing state");
		return_code=0;
	}
#endif /* defined (MOTIF) */
#if defined (WX_USER_INTERFACE)
	USE_PARAMETER(state);
	USE_PARAMETER(command_data_void);
	display_message(ERROR_MESSAGE,"command has been removed from the cmgui-wx, please use gfx modify window (NAME) node ? for further instruction for creating elements or directly create new elements using the node tool");
		return_code=0;
#endif /*defined (WX_USER_INTERFACE) */
	LEAVE;
	return (return_code);
} /* gfx_create_element_creator */
#endif /* defined (MOTIF) || defined (WX_USER_INTERFACE)*/

static int gfx_create_element_points(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
Executes a GFX CREATE ELEMENT_POINTS command.
==============================================================================*/
{
	char exterior_flag, *graphics_object_name, *region_path,
		*use_element_type_string, **valid_strings, *xi_discretization_mode_string;
	enum Use_element_type use_element_type;
	enum Xi_discretization_mode xi_discretization_mode;
	float time;
	int face_number,number_of_components,number_of_valid_strings,return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct Computed_field *coordinate_field, *data_field, *label_field,
		*orientation_scale_field, *variable_scale_field, *xi_point_density_field;
	struct Element_discretization discretization;
	struct Element_to_glyph_set_data element_to_glyph_set_data;
	struct FE_field *native_discretization_field;
	struct FE_region *fe_region;
	struct Graphical_material *material;
	struct GT_object *glyph,*graphics_object;
	struct Spectrum *spectrum;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_label_field_data, set_orientation_scale_field_data,
		set_variable_scale_field_data, set_xi_point_density_field_data;
	struct Set_FE_field_conditional_FE_region_data
		set_native_discretization_field_data;
	Triple exact_xi,glyph_centre,glyph_scale_factors,glyph_size;

	ENTER(gfx_create_element_points);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		graphics_object_name = duplicate_string("element_points");
		Cmiss_region_get_root_region_path(&region_path);
		number_of_components=3;
		exact_xi[0]=0.5;
		exact_xi[1]=0.5;
		exact_xi[2]=0.5;
		glyph_centre[0]=0.0;
		glyph_centre[1]=0.0;
		glyph_centre[2]=0.0;
		coordinate_field=(struct Computed_field *)NULL;
		data_field = (struct Computed_field *)NULL;
		xi_point_density_field = (struct Computed_field *)NULL;
		exterior_flag=0;
		face_number=-1;
		if (glyph=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("point",
			command_data->glyph_list))
		{
			ACCESS(GT_object)(glyph);
		}
		label_field=(struct Computed_field *)NULL;
		material=
			ACCESS(Graphical_material)(Material_package_get_default_material(command_data->material_package));
		native_discretization_field=(struct FE_field *)NULL;
		orientation_scale_field = (struct Computed_field *)NULL;
		variable_scale_field = (struct Computed_field *)NULL;
		glyph_scale_factors[0]=1.0;
		glyph_scale_factors[1]=1.0;
		glyph_scale_factors[2]=1.0;
		glyph_size[0]=1.0;
		glyph_size[1]=1.0;
		glyph_size[2]=1.0;
		spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
		time=0.0;
		discretization.number_in_xi1=1;
		discretization.number_in_xi2=1;
		discretization.number_in_xi3=1;

		option_table=CREATE(Option_table)();
		/* as */
		Option_table_add_entry(option_table,"as",&graphics_object_name,
			(void *)1,set_name);
		/* cell_centres/cell_corners/cell_random */ 
		xi_discretization_mode = XI_DISCRETIZATION_CELL_CENTRES;
		xi_discretization_mode_string =
			ENUMERATOR_STRING(Xi_discretization_mode)(xi_discretization_mode);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Xi_discretization_mode)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Xi_discretization_mode) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table,number_of_valid_strings,
			valid_strings,&xi_discretization_mode_string);
		DEALLOCATE(valid_strings);
		/* centre [of glyph] */
		Option_table_add_entry(option_table,"centre",glyph_centre,
			&(number_of_components),set_float_vector);
		/* coordinate */
		set_coordinate_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_coordinate_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"coordinate",&coordinate_field,
			&set_coordinate_field_data,set_Computed_field_conditional);
		/* data */
		set_data_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_data_field_data.conditional_function=
			Computed_field_has_numerical_components;
		set_data_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"data",&data_field,
			&set_data_field_data,set_Computed_field_conditional);
		/* density */
		set_xi_point_density_field_data.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_xi_point_density_field_data.conditional_function =
			Computed_field_is_scalar;
		set_xi_point_density_field_data.conditional_function_user_data =
			(void *)NULL;
		Option_table_add_entry(option_table, "density",
			&xi_point_density_field, &set_xi_point_density_field_data,
			set_Computed_field_conditional);
		/* exterior */
		Option_table_add_entry(option_table,"exterior",&exterior_flag,
			NULL,set_char_flag);
		/* face */
		Option_table_add_entry(option_table,"face",&face_number,
			NULL,set_exterior);
		/* from */
		Option_table_add_entry(option_table, "from", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* glyph */
		Option_table_add_entry(option_table,"glyph",&glyph,
			command_data->glyph_list,set_Graphics_object);
		/* label */
		set_label_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_label_field_data.conditional_function=
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_label_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"label",&label_field,
			&set_label_field_data,set_Computed_field_conditional);
		/* material */
		Option_table_add_set_Material_entry(option_table, "material", &material,
			command_data->material_package);
		/* native_discretization */
		set_native_discretization_field_data.conditional_function =
			(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL;
		set_native_discretization_field_data.user_data = (void *)NULL;
		set_native_discretization_field_data.fe_region =
			Cmiss_region_get_FE_region(command_data->root_region);
		Option_table_add_entry(option_table, "native_discretization",
			&native_discretization_field,
			(void *)&set_native_discretization_field_data,
			set_FE_field_conditional_FE_region);
		/* orientation */
		set_orientation_scale_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_orientation_scale_field_data.conditional_function=
			Computed_field_is_orientation_scale_capable;
		set_orientation_scale_field_data.conditional_function_user_data=
			(void *)NULL;
		Option_table_add_entry(option_table,"orientation",
			&orientation_scale_field,&set_orientation_scale_field_data,
			set_Computed_field_conditional);
		/* scale_factors */
		Option_table_add_entry(option_table,"scale_factors",
			glyph_scale_factors,"*",set_special_float3);
		/* size [of glyph] */
		Option_table_add_entry(option_table,"size",
			glyph_size,"*",set_special_float3);
		/* spectrum */
		Option_table_add_entry(option_table,"spectrum",
			&spectrum,command_data->spectrum_manager,set_Spectrum);
		/* time */
		Option_table_add_entry(option_table,"time",&time,NULL,set_float);
		/* use_elements/use_faces/use_lines */
		use_element_type = USE_ELEMENTS;
		use_element_type_string =
			ENUMERATOR_STRING(Use_element_type)(use_element_type);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Use_element_type)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Use_element_type) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table,number_of_valid_strings,
			valid_strings,&use_element_type_string);
		DEALLOCATE(valid_strings);
		/* variable_scale */
		set_variable_scale_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_variable_scale_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_variable_scale_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"variable_scale", &variable_scale_field,
			&set_variable_scale_field_data, set_Computed_field_conditional);
		/* with */
		Option_table_add_entry(option_table,"with",&discretization,
			command_data->user_interface,set_Element_discretization);
		/* xi */
		Option_table_add_entry(option_table,"xi",
			exact_xi,&number_of_components,set_float_vector);
		if (return_code = Option_table_multi_parse(option_table, state))
		{
			if (!(Cmiss_region_get_region_from_path(command_data->root_region,
				region_path, &region) &&
				(fe_region = Cmiss_region_get_FE_region(region))))
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_element_points.  Invalid region");
				return_code = 0;
			}
			STRING_TO_ENUMERATOR(Xi_discretization_mode)(
				xi_discretization_mode_string, &xi_discretization_mode);
			if (((XI_DISCRETIZATION_CELL_DENSITY == xi_discretization_mode) ||
				(XI_DISCRETIZATION_CELL_POISSON == xi_discretization_mode)) &&
				((struct Computed_field *)NULL == xi_point_density_field))
			{
				display_message(ERROR_MESSAGE,
					"No density field specified for cell_density|cell_poisson");
				return_code = 0;
			}
			if (!coordinate_field)
			{
				display_message(WARNING_MESSAGE, "Must specify a coordinate field");
				return_code = 0;
			}
			if (return_code)
			{
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					if (g_GLYPH_SET == GT_object_get_type(graphics_object))
					{
						if (GT_object_has_time(graphics_object,time))
						{
							display_message(WARNING_MESSAGE,
								"Overwriting time %g in graphics object '%s'",time,
								graphics_object_name);
							return_code = GT_object_remove_primitives_at_time(
								graphics_object, time,
								(GT_object_primitive_object_name_conditional_function *)NULL,
								(void *)NULL);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Object of different type named '%s' already exists",
							graphics_object_name);
						return_code=0;
					}
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,
						g_GLYPH_SET,material))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_element_points.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
			}
			if (return_code)
			{
				element_to_glyph_set_data.font = command_data->default_font;
				element_to_glyph_set_data.time=time;
				element_to_glyph_set_data.xi_discretization_mode =
					xi_discretization_mode;
				element_to_glyph_set_data.xi_point_density_field =
					xi_point_density_field;
				element_to_glyph_set_data.coordinate_field=
					Computed_field_begin_wrap_coordinate_field(coordinate_field);
				if (orientation_scale_field)
				{
					element_to_glyph_set_data.orientation_scale_field=
						Computed_field_begin_wrap_orientation_scale_field(
							orientation_scale_field,
							element_to_glyph_set_data.coordinate_field);
				}
				else
				{
					element_to_glyph_set_data.orientation_scale_field=
						(struct Computed_field *)NULL;
				}
				STRING_TO_ENUMERATOR(Use_element_type)(use_element_type_string,
					&use_element_type);
				element_to_glyph_set_data.use_element_type = use_element_type;
				element_to_glyph_set_data.fe_region = fe_region;
				element_to_glyph_set_data.exterior=exterior_flag;
				element_to_glyph_set_data.face_number=face_number;
				element_to_glyph_set_data.number_in_xi[0]=
					discretization.number_in_xi1;
				element_to_glyph_set_data.number_in_xi[1]=
					discretization.number_in_xi2;
				element_to_glyph_set_data.number_in_xi[2]=
					discretization.number_in_xi3;
				element_to_glyph_set_data.variable_scale_field = variable_scale_field;
				element_to_glyph_set_data.data_field = data_field;
				element_to_glyph_set_data.label_field = label_field;
				element_to_glyph_set_data.native_discretization_field =
					native_discretization_field;
				element_to_glyph_set_data.glyph = glyph;
				element_to_glyph_set_data.graphics_object=graphics_object;
				element_to_glyph_set_data.exact_xi[0] = exact_xi[0];
				element_to_glyph_set_data.exact_xi[1] = exact_xi[1];
				element_to_glyph_set_data.exact_xi[2] = exact_xi[2];
				element_to_glyph_set_data.base_size[0] = (FE_value)(glyph_size[0]);
				element_to_glyph_set_data.base_size[1] = (FE_value)(glyph_size[1]);
				element_to_glyph_set_data.base_size[2] = (FE_value)(glyph_size[2]);
				element_to_glyph_set_data.centre[0] = (FE_value)(glyph_centre[0]);
				element_to_glyph_set_data.centre[1] = (FE_value)(glyph_centre[1]);
				element_to_glyph_set_data.centre[2] = (FE_value)(glyph_centre[2]);
				element_to_glyph_set_data.scale_factors[0] =
					(FE_value)(glyph_scale_factors[0]);
				element_to_glyph_set_data.scale_factors[1] =
					(FE_value)(glyph_scale_factors[1]);
				element_to_glyph_set_data.scale_factors[2] =
					(FE_value)(glyph_scale_factors[2]);
				element_to_glyph_set_data.select_mode = GRAPHICS_NO_SELECT;
				return_code = FE_region_for_each_FE_element(fe_region,
					element_to_glyph_set, (void *)&element_to_glyph_set_data);
				if (return_code)
				{
					if (!GT_object_has_time(graphics_object,time))
					{
						/* add a NULL glyph_set to make an empty time */
						GT_OBJECT_ADD(GT_glyph_set)(graphics_object,time,
							(struct GT_glyph_set *)NULL);
					}
					if (data_field)
					{
						return_code=set_GT_object_Spectrum(graphics_object,spectrum);
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"No element_points created");
					if (0==GT_object_get_number_of_times(graphics_object))
					{
						REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list);
					}
				}
				if (element_to_glyph_set_data.orientation_scale_field)
				{
					Computed_field_end_wrap(
						&(element_to_glyph_set_data.orientation_scale_field));
				}
				Computed_field_end_wrap(
					&(element_to_glyph_set_data.coordinate_field));
			}
		} /* parse error, help */
		DESTROY(Option_table)(&option_table);
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		if (data_field)
		{
			DEACCESS(Computed_field)(&data_field);
		}
		DEACCESS(Spectrum)(&spectrum);
		DEACCESS(Graphical_material)(&material);
		DEALLOCATE(graphics_object_name);
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
		if (native_discretization_field)
		{
			DEACCESS(FE_field)(&native_discretization_field);
		}
		DEALLOCATE(region_path);
		if (xi_point_density_field)
		{
			DEACCESS(Computed_field)(&xi_point_density_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_element_points.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_element_points */

struct Interpreter_command_element_selection_callback_data
{
	char *perl_action;
	struct Interpreter *interpreter;
}; /* struct Interpreter_command_element_selection_callback_data */

static void interpreter_command_element_selection_callback(
	struct FE_element_selection *element_selection,
	struct FE_element_selection_changes *element_selection_changes,
	void *data_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2005

DESCRIPTION :
==============================================================================*/
{
	char *callback_result;
	int return_code;
	struct Interpreter_command_element_selection_callback_data *data;

	ENTER(interpreter_command_element_selection_callback);

	if (element_selection && element_selection_changes && 
		(data = (struct Interpreter_command_element_selection_callback_data *)data_void))
	{
		callback_result = (char *)NULL;
		interpreter_evaluate_string(data->interpreter,
			  data->perl_action, &callback_result, &return_code);
		if (callback_result)
		{
			DEALLOCATE(callback_result);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"interpreter_command_element_selection_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return;
} /* interpreter_command_element_selection_callback */

static int gfx_create_element_selection_callback(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2005

DESCRIPTION :
Executes a GFX CREATE ELEMENT_SELECTION_CALLBACK command.
==============================================================================*/
{
	char *perl_action;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Interpreter_command_element_selection_callback_data *data;
	struct Option_table *option_table;

	ENTER(gfx_create_element_selection_callback);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		perl_action = (char *)NULL;
		
		option_table=CREATE(Option_table)();
		/* perl_action */
		Option_table_add_entry(option_table,"perl_action", &perl_action, (void *)1,
			set_name);
		return_code = Option_table_multi_parse(option_table,state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			if (!perl_action)
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_element_selection_callback.  "
					"Specify a perl_action.");
				return_code=0;
			}
			if (return_code)
			{
				if (ALLOCATE(data, struct Interpreter_command_element_selection_callback_data, 1))
				{
					data->perl_action = duplicate_string(perl_action);
					data->interpreter = command_data->interpreter;

					FE_element_selection_add_callback(
						command_data->element_selection,
						interpreter_command_element_selection_callback,
						(void *)data);

					/* Should add these callbacks to a list in the command data so that 
						they can be cleaned up when quitting and retrieved for a destroy command */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_element_selection_callback.  "
						"Unable to allocate callback data.");
					return_code=0;
				}
			}
		}
		if (perl_action)
		{
			DEALLOCATE(perl_action);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_element_selection_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_element_selection_callback */

static int gfx_create_group(struct Parse_state *state,
	void *use_nodes, void *root_region_void)
/*******************************************************************************
LAST MODIFIED : 15 August 2006

DESCRIPTION :
Executes a GFX CREATE GROUP command.
==============================================================================*/
{
	char *from_region_path, *name, *region_path;
	int return_code;
	struct CM_element_type_Multi_range_data element_data;
	struct Cmiss_region *base_region, *from_region, *region, *root_region;
	struct FE_region *fe_region, *from_fe_region;
	struct Multi_range *add_ranges;
	struct Option_table *option_table;

	ENTER(gfx_create_group);
	if (state && (root_region = (struct Cmiss_region *)root_region_void))
	{
		name = (char *)NULL;
		if (set_name(state, (void *)&name, (void *)1))
		{
			return_code = 1;
			if (name && Cmiss_region_get_region_from_path(root_region, name, &region) &&
				region)
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_group.  Group '%s' already exists", name);
				return_code = 0;
			}
			if (return_code)
			{
				/* initialise defaults */
				Cmiss_region_get_root_region_path(&from_region_path);
				Cmiss_region_get_root_region_path(&region_path);
				add_ranges = CREATE(Multi_range)();
				option_table = CREATE(Option_table)();
				/* add_ranges */
				Option_table_add_entry(option_table, "add_ranges", add_ranges,
					NULL, set_Multi_range);
				/* from */
				Option_table_add_entry(option_table, "from", &from_region_path,
					root_region, set_Cmiss_region_path);
				/* region */
				Option_table_add_entry(option_table, "region", &region_path,
					root_region, set_Cmiss_region_path);
				if (return_code = Option_table_multi_parse(option_table, state))
				{
					Cmiss_region_get_region_from_path(root_region, region_path,
						&base_region);
					region = CREATE(Cmiss_region)();
					ACCESS(Cmiss_region)(region);
					fe_region = CREATE(FE_region)(/*master_fe_region*/
						Cmiss_region_get_FE_region(base_region),
						(struct MANAGER(FE_basis) *)NULL, (struct LIST(FE_element_shape) *)NULL);
					ACCESS(FE_region)(fe_region);
					if (0 < Multi_range_get_number_of_ranges(add_ranges))
					{
						if (Cmiss_region_get_region_from_path(root_region, from_region_path,
							&from_region) &&
							(from_fe_region = Cmiss_region_get_FE_region(from_region)))
						{
							if (use_nodes)
							{
								/* add nodes */
								return_code =
									FE_region_for_each_FE_node_conditional(from_fe_region,
										FE_node_is_in_Multi_range, (void *)add_ranges,
										FE_region_merge_FE_node_iterator, (void *)fe_region);
							}
							else
							{
								/* add elements */
								element_data.cm_element_type = CM_ELEMENT;
								element_data.multi_range = add_ranges;
								return_code =
									FE_region_for_each_FE_element_conditional(from_fe_region,
										FE_element_of_CM_element_type_is_in_Multi_range,
										(void *)&element_data,
										FE_region_merge_FE_element_and_faces_and_nodes_iterator,
										(void *)fe_region);
							}
						}
						else
						{
							return_code = 0;
						}
					}
					return_code = return_code &&
						Cmiss_region_attach_FE_region(region, fe_region) &&
						Cmiss_region_add_child_region(base_region, region, name,
							/*child_position*/-1);
					DEACCESS(FE_region)(&fe_region);
					DEACCESS(Cmiss_region)(&region);
				}
				DESTROY(Option_table)(&option_table);
				DESTROY(Multi_range)(&add_ranges);
				DEALLOCATE(from_region_path);
				DEALLOCATE(region_path);
			}
		}
		if (name)
		{
			DEALLOCATE(name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_group.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_group */

#if defined (MOTIF)
static int gfx_create_environment_map(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 2 October 1996

DESCRIPTION :
Executes a GFX CREATE ENVIRONMENT_MAP command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Environment_map *environment_map;
	struct Modify_environment_map_data modify_environment_map_data;

	ENTER(gfx_create_environment_map);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(Environment_map,name)(
						current_token,command_data->environment_map_manager))
					{
						if (environment_map=CREATE(Environment_map)(current_token))
						{
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								modify_environment_map_data.graphical_material_manager=
									Material_package_get_material_manager(command_data->material_package);
								modify_environment_map_data.environment_map_manager=
									command_data->environment_map_manager;
								return_code=modify_Environment_map(state,
									(void *)environment_map,
									(void *)(&modify_environment_map_data));
							}
							else
							{
								return_code=1;
							}
							ADD_OBJECT_TO_MANAGER(Environment_map)(environment_map,
								command_data->environment_map_manager);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_environment_map.  Error creating environment_map");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Environment map already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					modify_environment_map_data.graphical_material_manager=
						Material_package_get_material_manager(command_data->material_package);
					modify_environment_map_data.environment_map_manager=
						command_data->environment_map_manager;
					return_code=modify_Environment_map(state,
						(void *)NULL,(void *)(&modify_environment_map_data));
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_environment_map.  Missing command_data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing environment_map_name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_environment_map.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_environment_map */
#endif /* defined (MOTIF) */

static int gfx_create_flow_particles(struct Parse_state *state,
	void *create_more,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 May 2003

DESCRIPTION :
Executes a GFX CREATE FLOW_PARTICLES command.
==============================================================================*/
{
	char *graphics_object_name, *region_path;
	float time, xi[3];
	gtObject *graphics_object;
	struct GT_pointset *pointset;
	int current_number_of_particles, element_number, number_of_particles,
		return_code, vector_components;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct Element_to_particle_data element_to_particle_data;
	struct Computed_field *coordinate_field, *stream_vector_field;
	struct FE_region *fe_region;
	struct Graphical_material *material;
	struct Spectrum *spectrum;
	Triple *new_particle_positions, *old_particle_positions,
		*final_particle_positions;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_stream_vector_field_data;

	ENTER(gfx_create_flow_particles);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		graphics_object_name = duplicate_string("particles");
		Cmiss_region_get_root_region_path(&region_path);
		element_number=0;  /* Zero gives all elements in group */
		coordinate_field=(struct Computed_field *)NULL;
		stream_vector_field=(struct Computed_field *)NULL;
		vector_components=3;
		xi[0]=0.5;
		xi[1]=0.5;
		xi[2]=0.5;
		time=0;
		/* must access it now,because we deaccess it later */
		material=
			ACCESS(Graphical_material)(Material_package_get_default_material(command_data->material_package));
		spectrum=
			ACCESS(Spectrum)(command_data->default_spectrum);

		option_table=CREATE(Option_table)();
		/* as */
		Option_table_add_entry(option_table, "as", &graphics_object_name,
			(void *)1, set_name);
		/* coordinate */
		set_coordinate_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_coordinate_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"coordinate",&coordinate_field,
			&set_coordinate_field_data,set_Computed_field_conditional);
		/* element */
		Option_table_add_entry(option_table, "element", &element_number,
			NULL, set_int_non_negative);
		/* from */
		Option_table_add_entry(option_table, "from", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* initial_xi */
		Option_table_add_entry(option_table, "initial_xi", xi,
			&(vector_components), set_FE_value_array);
		/* material */
		Option_table_add_set_Material_entry(option_table, "material", &material,
			command_data->material_package);
		/* spectrum */
		Option_table_add_entry(option_table,"spectrum",&spectrum,
			command_data->spectrum_manager,set_Spectrum);
		/* time */
		Option_table_add_entry(option_table,"time",&time,NULL,set_float);
		/* vector */
		set_stream_vector_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_stream_vector_field_data.conditional_function=
			Computed_field_is_stream_vector_capable;
		set_stream_vector_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"vector",&stream_vector_field,
			&set_stream_vector_field_data,set_Computed_field_conditional);
		return_code=Option_table_multi_parse(option_table,state);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (!coordinate_field)
			{
				display_message(WARNING_MESSAGE, "Must specify a coordinate field");
				return_code = 0;
			}
			if (!(Cmiss_region_get_region_from_path(command_data->root_region,
				region_path, &region) &&
				(fe_region = Cmiss_region_get_FE_region(region))))
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_flow_particles.  Invalid region");
				return_code = 0;
			}
		}
		if (return_code)
		{
			if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
				graphics_object_name,command_data->graphics_object_list))
			{
				if (create_more)
				{
					if (pointset = GT_OBJECT_GET(GT_pointset)(graphics_object, time))
					{
						GT_pointset_get_point_list(pointset,
							&current_number_of_particles, &old_particle_positions);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_flow_particles.  Missing pointlist for adding more");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_flow_particles.  Object already exists");
					return_code = 0;
				}
			}
			else
			{
				if (create_more)
				{
					display_message(ERROR_MESSAGE, "gfx_create_flow_particles.  "
						"Graphics object does not exist for adding more to");
					return_code=0;
				}
				else
				{
					if ((graphics_object=CREATE(GT_object)(graphics_object_name,
						g_POINTSET,material))&&
						ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_flow_particles.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
			}
			if (return_code)
			{
				number_of_particles = FE_region_get_number_of_FE_elements(fe_region);
				if (create_more)
				{
					number_of_particles += current_number_of_particles;
					if (REALLOCATE(new_particle_positions,old_particle_positions,Triple,
						number_of_particles))
					{
						GT_pointset_set_point_list(pointset, number_of_particles,
							new_particle_positions);
						element_to_particle_data.pointlist = &new_particle_positions;
						element_to_particle_data.index = current_number_of_particles;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_flow_particles.  Unable to reallocate pointset");
						return_code = 0;
					}
				}
				else
				{
					if (ALLOCATE(new_particle_positions,Triple,number_of_particles) &&
						(pointset=CREATE(GT_pointset)(number_of_particles,
							new_particle_positions,(char **)NULL,	g_POINT_MARKER,
							1,g_NO_DATA,(GTDATA *) NULL,(int *)NULL,command_data->default_font)))
					{
						element_to_particle_data.pointlist= &new_particle_positions;
						element_to_particle_data.index=0;
					}
					else
					{
						DEALLOCATE(new_particle_positions);
						display_message(ERROR_MESSAGE,
							"gfx_create_flow_particles.  Unable to create pointset");
						return_code = 0;
					}
				}
				if (return_code)
				{
					element_to_particle_data.coordinate_field=coordinate_field;
					element_to_particle_data.element_number=element_number;
					element_to_particle_data.stream_vector_field=stream_vector_field;
					element_to_particle_data.graphics_object=graphics_object;
					element_to_particle_data.xi[0]=xi[0];
					element_to_particle_data.xi[1]=xi[1];
					element_to_particle_data.xi[2]=xi[2];
					element_to_particle_data.number_of_particles=0;
					element_to_particle_data.list= &(command_data->streampoint_list);
					return_code = FE_region_for_each_FE_element(fe_region,
						element_to_particle, (void *)&element_to_particle_data);
					number_of_particles = element_to_particle_data.number_of_particles;
					if (create_more)
					{
						number_of_particles += current_number_of_particles;
					}
					if (return_code && number_of_particles &&
						REALLOCATE(final_particle_positions, new_particle_positions,
							Triple, number_of_particles))
					{
						GT_pointset_set_point_list(pointset, number_of_particles,
							final_particle_positions);
						if (create_more)
						{
							GT_object_changed(graphics_object);
						}
						else
						{
							if (GT_OBJECT_ADD(GT_pointset)(graphics_object,time,pointset))
							{
								return_code = set_GT_object_Spectrum(graphics_object,spectrum);
							}
							else
							{
								DESTROY(GT_pointset)(&pointset);
								display_message(ERROR_MESSAGE, "gfx_create_flow_particles.  "
									"Could not add pointset to graphics object");
								return_code = 0;
							}
						}
					}
					else
					{
						if (create_more)
						{
							DEALLOCATE(old_particle_positions);
						}
						else
						{
							DESTROY(GT_pointset)(&pointset);
						}
						if (return_code)
						{
							display_message(WARNING_MESSAGE,"No particles created");
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_flow_particles.  Error creating particles");
						}
					}
				}
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		if (stream_vector_field)
		{
			DEACCESS(Computed_field)(&stream_vector_field);
		}
		DEALLOCATE(region_path);
		DEACCESS(Spectrum)(&spectrum);
		DEACCESS(Graphical_material)(&material);
		DEALLOCATE(graphics_object_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_flow_particles.  Invalid argument(s)");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* gfx_create_flow_particles */

static int gfx_modify_flow_particles(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 January 2000

DESCRIPTION :
Executes a GFX MODIFY FLOW_PARTICLES command.
==============================================================================*/
{
	int return_code;
	float stepsize,time;
	struct Cmiss_command_data *command_data;
	struct Computed_field *coordinate_field,*stream_vector_field;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_stream_vector_field_data;

	ENTER(gfx_modify_flow_particles);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			coordinate_field=(struct Computed_field *)NULL;
			stream_vector_field=(struct Computed_field *)NULL;
			stepsize=1;
			/* If time of 0 is sent the previous points are updated at the previous
				time value */
			time=0;

			option_table=CREATE(Option_table)();
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* stepsize */
			Option_table_add_entry(option_table,"stepsize",&stepsize,
				NULL,set_float);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* vector */
			set_stream_vector_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_stream_vector_field_data.conditional_function=
				Computed_field_is_stream_vector_capable;
			set_stream_vector_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"vector",&stream_vector_field,
				&set_stream_vector_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code=update_flow_particle_list(
					command_data->streampoint_list,coordinate_field,stream_vector_field,
					stepsize,time);
			}
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (stream_vector_field)
			{
				DEACCESS(Computed_field)(&stream_vector_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_flow_particles.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_flow_particles.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_flow_particles */

#if defined (MOTIF)
static int gfx_create_graphical_material_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE GRAPHICAL_MATERIAL_EDITOR command.
If there is a material editor dialog in existence, then bring it to the front,
otherwise it creates a new one.  Assumes we will only ever want one material
editor at a time.  This implementation may be changed later.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_graphical_material_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				return_code=bring_up_material_editor_dialog(
					&(command_data->material_editor_dialog),
					User_interface_get_application_shell(command_data->user_interface),
					Material_package_get_material_manager(command_data->material_package),
					command_data->texture_manager,(struct Graphical_material *)NULL,
					command_data->graphics_buffer_package,command_data->user_interface);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_graphical_material_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_graphical_material_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_graphical_material_editor */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static int gfx_create_grid_field_calculator(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
Executes a GFX CREATE GRID_FIELD_CALCULATOR command.
Invokes the grid field calculator dialog.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_grid_field_calculator);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				return_code=bring_up_grid_field_calculator(
					&(command_data->grid_field_calculator_dialog),
					User_interface_get_application_shell(command_data->user_interface),
					command_data->computed_field_package,
					&(command_data->curve_editor_dialog),
					command_data->root_region,
					command_data->curve_manager,
					command_data->user_interface);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_grid_field_calculator.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_grid_field_calculator.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_grid_field_calculator */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static int gfx_create_input_module_control(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Executes a GFX CREATE IM_CONTROL command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_input_module_control);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
#if defined (EXT_INPUT)
				return_code=bring_up_input_module_dialog(
					&(command_data->input_module_dialog),
					User_interface_get_application_shell(command_data->user_interface),
					Material_package_get_default_material(command_data->material_package),
					Material_package_get_material_manager(command_data->material_package),command_data->default_scene,
					command_data->scene_manager, command_data->user_interface);
					/*???DB.  commmand_data should not be used outside of command.c */
#else /* defined (EXT_INPUT) */
				display_message(ERROR_MESSAGE,"External input module was not linked");
#endif /* defined (EXT_INPUT) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_input_module_control.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_input_module_control.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_input_module_control */
#endif /* defined (MOTIF) */

static int gfx_create_iso_surfaces(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 February 2003

DESCRIPTION :
Executes a GFX CREATE ISO_SURFACES command.
==============================================================================*/
{
	char exterior_flag, *graphics_object_name, *region_path, *render_type_string,
		*surface_data_region_path, *use_element_type_string, **valid_strings;
	enum GT_object_type graphics_object_type;
	enum Render_type render_type;
	enum Use_element_type use_element_type;
	float time;
	int face_number,number_of_valid_strings,return_code;
	struct Clipping *clipping;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region, *surface_data_region;
	struct Computed_field *coordinate_field, *data_field, *scalar_field,
		*surface_data_coordinate_field, *surface_data_density_field;
	struct Element_discretization discretization;
	struct Element_to_iso_scalar_data element_to_iso_scalar_data;
	struct FE_element *first_element;
	struct FE_field *native_discretization_field;
	struct FE_region *fe_region, *surface_data_fe_region;
	struct Graphical_material *material;
	struct GT_object *graphics_object;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_scalar_field_data,
		set_surface_data_coordinate_field_data, set_surface_data_density_field_data;
	struct Set_FE_field_conditional_FE_region_data
		set_native_discretization_field_data;
	struct Spectrum *spectrum;

	ENTER(gfx_create_iso_surfaces);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void)
		&& (computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package)))
	{
		/* initialise defaults */
		graphics_object_name = duplicate_string("iso_surfaces");
		Cmiss_region_get_root_region_path(&region_path);
		coordinate_field=(struct Computed_field *)NULL;
		data_field=(struct Computed_field *)NULL;
		surface_data_region_path = (char *)NULL;
		surface_data_coordinate_field = (struct Computed_field *)NULL;
		surface_data_density_field = (struct Computed_field *)NULL;
		time=0;
		material=ACCESS(Graphical_material)(
			Material_package_get_default_material(command_data->material_package));
		clipping=(struct Clipping *)NULL;
		if (scalar_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
			Computed_field_is_scalar,(void *)NULL,computed_field_manager))
		{
			ACCESS(Computed_field)(scalar_field);
		}
		element_to_iso_scalar_data.iso_value=0;
		native_discretization_field=(struct FE_field *)NULL;
		spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
		discretization.number_in_xi1=4;
		discretization.number_in_xi2=4;
		discretization.number_in_xi3=4;
		exterior_flag=0;
		face_number=-1;

		option_table=CREATE(Option_table)();
		/* as */
		Option_table_add_entry(option_table,"as",&graphics_object_name,
			(void *)1,set_name);
		/* clipping */
		Option_table_add_entry(option_table,"clipping",&clipping,
			NULL,set_Clipping);
		/* coordinate */
		set_coordinate_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_coordinate_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"coordinate",&coordinate_field,
			&set_coordinate_field_data,set_Computed_field_conditional);
		/* data */
		set_data_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_data_field_data.conditional_function=
			Computed_field_has_numerical_components;
		set_data_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"data",&data_field,
			&set_data_field_data,set_Computed_field_conditional);
		/* exterior */
		Option_table_add_entry(option_table,"exterior",&exterior_flag,
			NULL,set_char_flag);
		/* face */
		Option_table_add_entry(option_table,"face",&face_number,
			NULL,set_exterior);
		/* from */
		Option_table_add_entry(option_table, "from", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* iso_scalar */
		set_scalar_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_scalar_field_data.conditional_function=Computed_field_is_scalar;
		set_scalar_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"iso_scalar",&scalar_field,
			&set_scalar_field_data,set_Computed_field_conditional);
		/* iso_value */
		Option_table_add_entry(option_table,"iso_value",
			&(element_to_iso_scalar_data.iso_value),NULL,set_double);
		/* material */
		Option_table_add_set_Material_entry(option_table, "material", &material,
			command_data->material_package);
		/* native_discretization */
		set_native_discretization_field_data.conditional_function =
			(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL;
		set_native_discretization_field_data.user_data = (void *)NULL;
		set_native_discretization_field_data.fe_region =
			Cmiss_region_get_FE_region(command_data->root_region);
		Option_table_add_entry(option_table, "native_discretization",
			&native_discretization_field,
			(void *)&set_native_discretization_field_data,
			set_FE_field_conditional_FE_region);
		/* render_type */
		render_type = RENDER_TYPE_SHADED;
		render_type_string = ENUMERATOR_STRING(Render_type)(render_type);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Render_type)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Render_type) *)NULL, (void *)NULL);
		Option_table_add_enumerator(option_table,number_of_valid_strings,
			valid_strings,&render_type_string);
		DEALLOCATE(valid_strings);
		/* spectrum */
		Option_table_add_entry(option_table,"spectrum",&spectrum,
			command_data->spectrum_manager,set_Spectrum);
		/* surface_data_coordinate */
		set_surface_data_coordinate_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_surface_data_coordinate_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_surface_data_coordinate_field_data.conditional_function_user_data =
			(void *)NULL;
		Option_table_add_entry(option_table,"surface_data_coordinate",
			&surface_data_coordinate_field,
			&set_surface_data_coordinate_field_data,set_Computed_field_conditional);
		/* surface_data_density */
		set_surface_data_density_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_surface_data_density_field_data.conditional_function=
			Computed_field_has_up_to_4_numerical_components;
		set_surface_data_density_field_data.conditional_function_user_data=
			(void *)NULL;
		Option_table_add_entry(option_table,"surface_data_density",
			&surface_data_density_field,&set_surface_data_density_field_data,
			set_Computed_field_conditional);
		/* surface_data_group */
		Option_table_add_entry(option_table, "surface_data_group",
			&surface_data_region_path,
			command_data->data_root_region, set_Cmiss_region_path);
		/* time */
		Option_table_add_entry(option_table,"time",&time,NULL,set_float);
		/* use_elements/use_faces/use_lines */
		use_element_type = USE_ELEMENTS;
		use_element_type_string =
			ENUMERATOR_STRING(Use_element_type)(use_element_type);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Use_element_type)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Use_element_type) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table,number_of_valid_strings,
			valid_strings, &use_element_type_string);
		DEALLOCATE(valid_strings);
		/* with */
		Option_table_add_entry(option_table,"with",&discretization,
			command_data->user_interface,set_Element_discretization);
		return_code = Option_table_multi_parse(option_table, state);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (!(Cmiss_region_get_region_from_path(command_data->root_region,
				region_path, &region) &&
				(fe_region = Cmiss_region_get_FE_region(region))))
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_iso_surfaces.  Invalid region");
				return_code = 0;
			}
			surface_data_fe_region = (struct FE_region *)NULL;
			if (surface_data_region_path)
			{
				if (!(Cmiss_region_get_region_from_path(command_data->data_root_region,
					surface_data_region_path, &surface_data_region) &&
					(surface_data_fe_region =
						Cmiss_region_get_FE_region(surface_data_region))))
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_iso_surfaces.  Invalid surface_data_region");
					return_code = 0;
				}
			}
			if ((surface_data_fe_region && (!surface_data_density_field)) ||
				((!surface_data_fe_region) && surface_data_density_field))
			{
				display_message(ERROR_MESSAGE, "gfx_create_volumes.  Must supply "
					"a surface_data_density_field with a surface_data_group");
				return_code = 0;
			}
			if (!scalar_field)
			{
				display_message(WARNING_MESSAGE,"Missing iso_scalar field");
				return_code=0;
			}
			if (!coordinate_field)
			{
				display_message(WARNING_MESSAGE, "Missing coordinate field");
				return_code = 0;
			}
			STRING_TO_ENUMERATOR(Use_element_type)(use_element_type_string,
				&use_element_type);
			element_to_iso_scalar_data.use_element_type = use_element_type;
			switch (element_to_iso_scalar_data.use_element_type)
			{
				case USE_ELEMENTS:
				{
					graphics_object_type = g_VOLTEX;
					first_element = FE_region_get_first_FE_element_that(fe_region,
						FE_element_is_top_level, (void *)NULL);
					if (first_element && (2 == get_FE_element_dimension(first_element)))
					{
						graphics_object_type=g_POLYLINE;
					}
				} break;
				case USE_FACES:
				{
					graphics_object_type=g_POLYLINE;
				} break;
				case USE_LINES:
				{
					display_message(ERROR_MESSAGE,
						".  "
						"USE_LINES not supported for iso_scalar");
					return_code=0;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						".  "
						"Unknown use_element_type");
					return_code=0;
				} break;
			}
			if (!graphics_object_name)
			{
				display_message(WARNING_MESSAGE,"Missing name");
				return_code=0;
			}
		}
		if (return_code)
		{
			if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
				graphics_object_name,command_data->graphics_object_list))
			{
				if (graphics_object_type==GT_object_get_type(graphics_object))
				{
					if (GT_object_has_time(graphics_object,time))
					{
						return_code = GT_object_remove_primitives_at_time(
							graphics_object, time,
							(GT_object_primitive_object_name_conditional_function *)NULL,
							(void *)NULL);
						display_message(WARNING_MESSAGE,
							"Overwriting time %g in graphics object '%s'",time,
							graphics_object_name);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Object of different type named '%s' already exists",
						graphics_object_name);
					return_code=0;
				}
			}
			else
			{
				if (!((graphics_object=CREATE(GT_object)(graphics_object_name,
					graphics_object_type,material))&&
					ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list)))
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_iso_surfaces.  Could not create graphics object");
					DESTROY(GT_object)(&graphics_object);
					return_code=0;
				}
			}
		}
		if (return_code)
		{
			if (surface_data_fe_region)
			{
				FE_region_begin_change(surface_data_fe_region);
			}
			element_to_iso_scalar_data.coordinate_field=
				Computed_field_begin_wrap_coordinate_field(coordinate_field);
			element_to_iso_scalar_data.data_field=data_field;
			element_to_iso_scalar_data.scalar_field=scalar_field;
			element_to_iso_scalar_data.graphics_object=graphics_object;
			element_to_iso_scalar_data.clipping=clipping;
			element_to_iso_scalar_data.texture_coordinate_field = 
				(struct Computed_field *)NULL;
			element_to_iso_scalar_data.time=time;
			element_to_iso_scalar_data.number_in_xi[0]=
				discretization.number_in_xi1;
			element_to_iso_scalar_data.number_in_xi[1]=
				discretization.number_in_xi2;
			element_to_iso_scalar_data.number_in_xi[2]=
				discretization.number_in_xi3;
			STRING_TO_ENUMERATOR(Render_type)(render_type_string, &render_type);
			element_to_iso_scalar_data.render_type = render_type;
			element_to_iso_scalar_data.fe_region = fe_region;
			element_to_iso_scalar_data.exterior=exterior_flag;
			element_to_iso_scalar_data.face_number=face_number;
			element_to_iso_scalar_data.native_discretization_field=
				native_discretization_field;
			return_code = FE_region_for_each_FE_element(fe_region,
				element_to_iso_scalar, (void *)&element_to_iso_scalar_data);
			if (return_code)
			{
				if (!GT_object_has_time(graphics_object,time))
				{
					/* add a NULL primitive of the correct type to make an empty time
						 so there are no gaps in any time animation */
					switch (graphics_object_type)
					{
						case g_POLYLINE:
						{
							GT_OBJECT_ADD(GT_polyline)(graphics_object,time,
								(struct GT_polyline *)NULL);
						} break;
						case g_VOLTEX:
						{
							GT_OBJECT_ADD(GT_voltex)(graphics_object,time,
								(struct GT_voltex *)NULL);
						} break;
					}
				}
				if (data_field)
				{
					return_code=set_GT_object_Spectrum(
						element_to_iso_scalar_data.graphics_object,spectrum);
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,"No iso_surface created");
				if (0==GT_object_get_number_of_times(graphics_object))
				{
					REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list);
				}
			}
			Computed_field_end_wrap(
				&(element_to_iso_scalar_data.coordinate_field));
			if (surface_data_fe_region)
			{
				FE_region_end_change(surface_data_fe_region);
			}
		}
		DESTROY(Option_table)(&option_table);
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		if (data_field)
		{
			DEACCESS(Computed_field)(&data_field);
		}
		if (scalar_field)
		{
			DEACCESS(Computed_field)(&scalar_field);
		}
		if (surface_data_coordinate_field)
		{
			DEACCESS(Computed_field)(&surface_data_coordinate_field);
		}
		if (surface_data_density_field)
		{
			DEACCESS(Computed_field)(&surface_data_density_field);
		}
		if (surface_data_region_path)
		{
			DEALLOCATE(surface_data_region_path);
		}
		if (clipping)
		{
			DESTROY(Clipping)(&clipping);
		}
		if (native_discretization_field)
		{
			DEACCESS(FE_field)(&native_discretization_field);
		}
		DEALLOCATE(region_path);
		DEACCESS(Spectrum)(&spectrum);
		DEACCESS(Graphical_material)(&material);
		DEALLOCATE(graphics_object_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_iso_surfaces.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_iso_surfaces */

static int gfx_create_light(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE LIGHT command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Light *light;
	struct Modify_light_data modify_light_data;

	ENTER(gfx_create_light);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(Light,name)(current_token,
						command_data->light_manager))
					{
						if (light=CREATE(Light)(current_token))
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Light,name)(light,
								command_data->default_light);
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								modify_light_data.default_light=command_data->default_light;
								modify_light_data.light_manager=command_data->light_manager;
								return_code=modify_Light(state,(void *)light,
									(void *)(&modify_light_data));
							}
							else
							{
								return_code=1;
							}
							ADD_OBJECT_TO_MANAGER(Light)(light,command_data->light_manager);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_light.  Could not create light");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Light already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					modify_light_data.default_light=command_data->default_light;
					modify_light_data.light_manager=command_data->light_manager;
					return_code=modify_Light(state,(void *)NULL,
						(void *)(&modify_light_data));
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_light.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing light name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_light.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_light */

static int gfx_create_light_model(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE LMODEL command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Light_model *light_model;
	struct Modify_light_model_data modify_light_model_data;

	ENTER(gfx_create_light_model);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(Light_model,name)(current_token,
						command_data->light_model_manager))
					{
						if (light_model=CREATE(Light_model)(current_token))
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Light_model,name)(light_model,
								command_data->default_light_model);
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								modify_light_model_data.default_light_model=
									command_data->default_light_model;
								modify_light_model_data.light_model_manager=
									command_data->light_model_manager;
								return_code=modify_Light_model(state,(void *)light_model,
									(void *)(&modify_light_model_data));
							}
							else
							{
								return_code=1;
							}
							ADD_OBJECT_TO_MANAGER(Light_model)(light_model,
								command_data->light_model_manager);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_light_model.  Could not create light model");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Light_model already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					modify_light_model_data.default_light_model=
						command_data->default_light_model;
					modify_light_model_data.light_model_manager=
						command_data->light_model_manager;
					return_code=modify_Light_model(state,(void *)NULL,
						(void *)(&modify_light_model_data));
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_light_model.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing light model name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_light_model.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_light_model */

static int gfx_create_lines(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 January 2003

DESCRIPTION :
Executes a GFX CREATE LINES command.
==============================================================================*/
{
	char exterior_flag,*graphics_object_name, *region_path;
	float time;
	struct GT_object *graphics_object;
	int face_number,return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct Element_discretization discretization;
	struct Element_to_polyline_data element_to_polyline_data;
	struct Computed_field *coordinate_field,*data_field;
	struct FE_region *fe_region;
	struct Graphical_material *material;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data;
	struct Spectrum *spectrum;

	ENTER(gfx_create_lines);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		graphics_object_name = duplicate_string("lines");
		Cmiss_region_get_root_region_path(&region_path);
		coordinate_field=(struct Computed_field *)NULL;
		data_field=(struct Computed_field *)NULL;
		time=0;
		/* must access it now, because we deaccess it later */
		material=
			ACCESS(Graphical_material)(Material_package_get_default_material(command_data->material_package));
		spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
		discretization.number_in_xi1=4;
		discretization.number_in_xi2=0;
		discretization.number_in_xi3=0;
		exterior_flag=0;
		face_number=-1;

		option_table=CREATE(Option_table)();
		/* as */
		Option_table_add_entry(option_table,"as",&graphics_object_name,
			(void *)1,set_name);
		/* coordinate */
		set_coordinate_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_coordinate_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"coordinate",&coordinate_field,
			&set_coordinate_field_data,set_Computed_field_conditional);
		/* data */
		set_data_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_data_field_data.conditional_function=
			Computed_field_has_numerical_components;
		set_data_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"data",&data_field,
			&set_data_field_data,set_Computed_field_conditional);
		/* exterior */
		Option_table_add_entry(option_table,"exterior",&exterior_flag,
			NULL,set_char_flag);
		/* face */
		Option_table_add_entry(option_table,"face",&face_number,
			NULL,set_exterior);
		/* from */
		Option_table_add_entry(option_table, "from", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* material */
		Option_table_add_set_Material_entry(option_table, "material", &material,
			command_data->material_package);
		/* spectrum */
		Option_table_add_entry(option_table,"spectrum",&spectrum,
			command_data->spectrum_manager,set_Spectrum);
		/* time */
		Option_table_add_entry(option_table,"time",&time,NULL,set_float);
		/* with */
		Option_table_add_entry(option_table,"with",&discretization,
			command_data->user_interface,set_Element_discretization);
		return_code=Option_table_multi_parse(option_table,state);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (!(Cmiss_region_get_region_from_path(command_data->root_region,
				region_path, &region) &&
				(fe_region = Cmiss_region_get_FE_region(region))))
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_lines.  Invalid region");
				return_code = 0;
			}
			if (!coordinate_field)
			{
				display_message(WARNING_MESSAGE, "Missing coordinate field");
				return_code = 0;
			}
		}
		if (return_code)
		{
			if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
				graphics_object_name,command_data->graphics_object_list))
			{
				if (g_POLYLINE==GT_object_get_type(graphics_object))
				{
					if (GT_object_has_time(graphics_object,time))
					{
						display_message(WARNING_MESSAGE,
							"Overwriting time %g in graphics object '%s'",time,
							graphics_object_name);
						return_code = GT_object_remove_primitives_at_time(
							graphics_object, time,
							(GT_object_primitive_object_name_conditional_function *)NULL,
							(void *)NULL);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Object of different type named '%s' already exists",
						graphics_object_name);
					return_code=0;
				}
			}
			else
			{
				if ((graphics_object=CREATE(GT_object)(graphics_object_name,
					g_POLYLINE,material))&&
					ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list))
				{
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_lines.  Could not create graphics object");
					DESTROY(GT_object)(&graphics_object);
					return_code=0;
				}
			}
			if (return_code)
			{
				element_to_polyline_data.coordinate_field=
					Computed_field_begin_wrap_coordinate_field(coordinate_field);
				element_to_polyline_data.fe_region = fe_region;
				element_to_polyline_data.exterior=exterior_flag;
				element_to_polyline_data.face_number=face_number;
				element_to_polyline_data.data_field=data_field;
				element_to_polyline_data.graphics_object=graphics_object;
				element_to_polyline_data.time=time;
				element_to_polyline_data.number_of_segments_in_xi1=
					discretization.number_in_xi1;
				return_code = FE_region_for_each_FE_element(fe_region,
					element_to_polyline, (void *)&element_to_polyline_data);
				if (return_code)
				{
					if (!GT_object_has_time(graphics_object,time))
					{
						/* add a NULL polyline to make an empty time */
						GT_OBJECT_ADD(GT_polyline)(graphics_object,time,
							(struct GT_polyline *)NULL);
					}
					if (data_field)
					{
						return_code=set_GT_object_Spectrum(graphics_object,spectrum);
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"No lines created");
					if (0==GT_object_get_number_of_times(graphics_object))
					{
						REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list);
					}
				}
				Computed_field_end_wrap(&(element_to_polyline_data.coordinate_field));
			}
		} /* parse error, help */
		DESTROY(Option_table)(&option_table);
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		if (data_field)
		{
			DEACCESS(Computed_field)(&data_field);
		}
		DEALLOCATE(region_path);
		DEACCESS(Spectrum)(&spectrum);
		DEACCESS(Graphical_material)(&material);
		DEALLOCATE(graphics_object_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_lines.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_lines */

static int gfx_create_morph(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2001

DESCRIPTION :
Executes a GFX CREATE MORPH command.  This command interpolates between two
graphics objects, and produces a new object
==============================================================================*/
{
	char *graphics_object_name;
	float proportion;
	gtObject *graphics_object;
	gtObject *initial,*final;
	int return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"as",NULL,(void *)1,set_name},
		{"final",NULL,NULL,set_Graphics_object},
		{"initial",NULL,NULL,set_Graphics_object},
		{"proportion",NULL,NULL,set_float_0_to_1_inclusive},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_create_morph);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			graphics_object_name = duplicate_string("morph");
			proportion = 0.0;
			initial = (gtObject *)NULL;
			final = (gtObject *)NULL;
			(option_table[0]).to_be_modified= &graphics_object_name;
			(option_table[1]).to_be_modified= &final;
			(option_table[1]).user_data= (void *)command_data->graphics_object_list;
			(option_table[2]).to_be_modified= &initial;
			(option_table[2]).user_data= (void *)command_data->graphics_object_list;
			(option_table[3]).to_be_modified= &proportion;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				/* check for valid arguments */
				if (initial&&final)
				{
					if (graphics_object=morph_gtObject(graphics_object_name,
						proportion,initial,final))
					{
						if (!ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list))
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_morph.  Could not add graphics object to list");
							DESTROY(GT_object)(&graphics_object);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_morph.  Could not create morph from surface");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Must specify initial and final graphics objects for morph");
					return_code=0;
				}
			} /* parse error, help */
			DEALLOCATE(graphics_object_name);
			if (initial)
			{
				DEACCESS(GT_object)(&initial);
			}
			if (final)
			{
				DEACCESS(GT_object)(&final);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_create_morph.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_morph.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_morph */

struct Interpreter_command_node_selection_callback_data
{
	char *perl_action;
	struct Interpreter *interpreter;
}; /* struct Interpreter_command_node_selection_callback_data */

static void interpreter_command_node_selection_callback(
	struct FE_node_selection *node_selection,
	struct FE_node_selection_changes *node_selection_changes,
	void *data_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2005

DESCRIPTION :
==============================================================================*/
{
	char *callback_result;
	int return_code;
	struct Interpreter_command_node_selection_callback_data *data;

	ENTER(interpreter_command_node_selection_callback);

	if (node_selection && node_selection_changes && 
		(data = (struct Interpreter_command_node_selection_callback_data *)data_void))
	{
		callback_result = (char *)NULL;
		interpreter_evaluate_string(data->interpreter,
			  data->perl_action, &callback_result, &return_code);
		if (callback_result)
		{
			DEALLOCATE(callback_result);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"interpreter_command_node_selection_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return;
} /* interpreter_command_node_selection_callback */

static int gfx_create_node_selection_callback(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2005

DESCRIPTION :
Executes a GFX CREATE NODE_SELECTION_CALLBACK command.
==============================================================================*/
{
	char *perl_action;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Interpreter_command_node_selection_callback_data *data;
	struct Option_table *option_table;

	ENTER(gfx_create_node_selection_callback);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		perl_action = (char *)NULL;
		
		option_table=CREATE(Option_table)();
		/* perl_action */
		Option_table_add_entry(option_table,"perl_action", &perl_action, (void *)1,
			set_name);
		return_code = Option_table_multi_parse(option_table,state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			if (!perl_action)
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_node_selection_callback.  "
					"Specify a perl_action.");
				return_code=0;
			}
			if (return_code)
			{
				if (ALLOCATE(data, struct Interpreter_command_node_selection_callback_data, 1))
				{
					data->perl_action = duplicate_string(perl_action);
					data->interpreter = command_data->interpreter;

					FE_node_selection_add_callback(
						command_data->node_selection,
						interpreter_command_node_selection_callback,
						(void *)data);

					/* Should add these callbacks to a list in the command data so that 
						they can be cleaned up when quitting and retrieved for a destroy command */
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_node_selection_callback.  "
						"Unable to allocate callback data.");
					return_code=0;
				}
			}
		}
		if (perl_action)
		{
			DEALLOCATE(perl_action);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_node_selection_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_node_selection_callback */

#if defined (MOTIF) || defined (WX_USER_INTERFACE)
static int gfx_create_node_viewer(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 July 2001

DESCRIPTION :
Executes a GFX CREATE NODE_VIEWER command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Time_object *time_object;

	ENTER(gfx_create_node_viewer);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (command_data->node_viewer)
				{
					return_code=Node_viewer_bring_window_to_front(
						command_data->node_viewer);
				}
				else
				{
					if ((time_object = CREATE(Time_object)("node_viewer_time"))
						&&(Time_object_set_time_keeper(time_object,
						command_data->default_time_keeper)))
					{
						if (command_data->node_viewer = CREATE(Node_viewer)(
							&(command_data->node_viewer),
							"Node Viewer",
							(struct FE_node *)NULL,
							command_data->root_region,
							command_data->node_selection,
							command_data->computed_field_package,
							time_object, command_data->user_interface))
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
						display_message(ERROR_MESSAGE,
							"gfx_create_node_viewer.  Unable to make time object.");
						return_code=0;
					}						
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_node_viewer.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_node_viewer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_node_viewer */
#endif /* defined (MOTIF) || defined (WX_USER_INTERFACE) */

#if defined (MOTIF) || defined (WX_USER_INTERFACE)
static int gfx_create_data_viewer(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 July 2001

DESCRIPTION :
Executes a GFX CREATE DATA_VIEWER command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Time_object *time_object;

	ENTER(gfx_create_data_viewer);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (command_data->data_viewer)
				{
					return_code=Node_viewer_bring_window_to_front(
						command_data->data_viewer);
				}
				else
				{
					if ((time_object = CREATE(Time_object)("data_viewer_time"))
						&&(Time_object_set_time_keeper(time_object,
						command_data->default_time_keeper)))
					{
						if (command_data->data_viewer = CREATE(Node_viewer)(
							&(command_data->data_viewer),
							"Data Viewer",
							(struct FE_node *)NULL,
							command_data->data_root_region,
							command_data->data_selection,
							command_data->computed_field_package,
							time_object, command_data->user_interface))
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
						display_message(ERROR_MESSAGE,
							"gfx_create_node_viewer.  Missing command_data");
						return_code=0;
					}						
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_data_viewer.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_data_viewer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_data_viewer */
#endif /* defined (MOTIF)  || defined (WX_USER_INTERFACE)*/

#if defined (MOTIF)  || defined (WX_USER_INTERFACE)
static int gfx_create_element_point_viewer(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
Executes a GFX CREATE ELEMENT_POINT_VIEWER command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Time_object *time_object;

	ENTER(gfx_create_element_point_viewer);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (command_data->element_point_viewer)
				{
					return_code=Element_point_viewer_bring_window_to_front(
						command_data->element_point_viewer);
				}
				else
				{
					if ((time_object = CREATE(Time_object)("element_point_viewer_time"))
						&&(Time_object_set_time_keeper(time_object,
						command_data->default_time_keeper)))
					{
					if (command_data->element_point_viewer=CREATE(Element_point_viewer)(
						&(command_data->element_point_viewer),
						command_data->root_region,
						command_data->element_point_ranges_selection,
						command_data->computed_field_package,
						time_object,
						command_data->user_interface))
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
						display_message(ERROR_MESSAGE,
							"gfx_create_element_point_viewer.  Unable to make time object.");
						return_code=0;
					}						
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_element_point_viewer.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_element_point_viewer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_element_point_viewer */
#endif /* defined (MOTIF)  || defined (WX_USER_INTERFACE) */

static int gfx_create_node_points(struct Parse_state *state,
	void *use_data,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 February 2003

DESCRIPTION :
Executes a GFX CREATE NODE_POINTS or GFX CREATE DATA_POINTS command.
If <use_data> is set, creating data points, otherwise creating node points.
==============================================================================*/
{
	char *graphics_object_name, *region_path;
	FE_value base_size[3], centre[3], scale_factors[3], time;
	int number_of_components, return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region, *root_region;
	struct Computed_field *coordinate_field, *data_field, *label_field,
		*orientation_scale_field, *rc_coordinate_field, *variable_scale_field,
		*wrapper_orientation_scale_field;
	struct FE_region *fe_region;
	struct Graphical_material *material;
	struct GT_glyph_set *glyph_set;
	struct GT_object *glyph,*graphics_object;
	struct Option_table *option_table;
	struct Spectrum *spectrum;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_label_field_data, set_orientation_scale_field_data,
		set_variable_scale_field_data;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	ENTER(gfx_create_node_points);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		if (use_data)
		{
			root_region = command_data->data_root_region;
			graphics_object_name = duplicate_string("data_points");
		}
		else
		{
			root_region = command_data->root_region;
			graphics_object_name = duplicate_string("node_points");
		}
		/* default to point glyph for fastest possible display */
		if (glyph = FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("point",
			command_data->glyph_list))
		{
			ACCESS(GT_object)(glyph);
		}
		Cmiss_region_get_root_region_path(&region_path);
		coordinate_field = (struct Computed_field *)NULL;
		data_field = (struct Computed_field *)NULL;
		/* must access it now, because we deaccess it later */
		material =
			ACCESS(Graphical_material)(Material_package_get_default_material(command_data->material_package));
		label_field = (struct Computed_field *)NULL;
		orientation_scale_field = (struct Computed_field *)NULL;
		variable_scale_field = (struct Computed_field *)NULL;
		spectrum = ACCESS(Spectrum)(command_data->default_spectrum);
		if (command_data->default_time_keeper)
		{
			time = Time_keeper_get_time(command_data->default_time_keeper);
		}
		else
		{
			time = 0;
		}
		/* final_size = size + scale_factors*magnitude */
		glyph_scale_factors[0] = 1.0;
		glyph_scale_factors[1] = 1.0;
		glyph_scale_factors[2] = 1.0;
		glyph_size[0] = 1.0;
		glyph_size[1] = 1.0;
		glyph_size[2] = 1.0;
		number_of_components = 3;
		glyph_centre[0] = 0.0;
		glyph_centre[1] = 0.0;
		glyph_centre[2] = 0.0;

		option_table=CREATE(Option_table)();
		/* as */
		Option_table_add_entry(option_table,"as",&graphics_object_name,
			(void *)1,set_name);
		/* centre [of glyph] */
		Option_table_add_entry(option_table,"centre",glyph_centre,
			&(number_of_components),set_float_vector);
		/* coordinate */
		set_coordinate_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_coordinate_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"coordinate",&coordinate_field,
			&set_coordinate_field_data,set_Computed_field_conditional);
		/* data */
		set_data_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_data_field_data.conditional_function=
			Computed_field_has_numerical_components;
		set_data_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"data",&data_field,
			&set_data_field_data,set_Computed_field_conditional);
		/* from */
		Option_table_add_entry(option_table, "from", &region_path,
			root_region, set_Cmiss_region_path);
		/* glyph */
		Option_table_add_entry(option_table,"glyph",&glyph,
			command_data->glyph_list,set_Graphics_object);
		/* label */
		set_label_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_label_field_data.conditional_function=
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_label_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"label",&label_field,
			&set_label_field_data,set_Computed_field_conditional);
		/* material */
		Option_table_add_set_Material_entry(option_table, "material", &material,
			command_data->material_package);
		/* orientation */
		set_orientation_scale_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_orientation_scale_field_data.conditional_function=
			Computed_field_is_orientation_scale_capable;
		set_orientation_scale_field_data.conditional_function_user_data=
			(void *)NULL;
		Option_table_add_entry(option_table,"orientation",
			&orientation_scale_field,&set_orientation_scale_field_data,
			set_Computed_field_conditional);
		/* scale_factors */
		Option_table_add_entry(option_table,"scale_factors",
			glyph_scale_factors,"*",set_special_float3);
		/* size [of glyph] */
		Option_table_add_entry(option_table,"size",
			glyph_size,"*",set_special_float3);
		/* spectrum */
		Option_table_add_entry(option_table,"spectrum",
			&spectrum,command_data->spectrum_manager,set_Spectrum);
		/* time */
		Option_table_add_entry(option_table,"time",&time,NULL,set_FE_value);
		/* variable_scale */
		set_variable_scale_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_variable_scale_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_variable_scale_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"variable_scale",&variable_scale_field,
			&set_variable_scale_field_data,set_Computed_field_conditional);
		return_code = Option_table_multi_parse(option_table,state);
		if (return_code)
		{
			if (!(Cmiss_region_get_region_from_path(root_region, region_path, &region)
				&& (fe_region = Cmiss_region_get_FE_region(region))))
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_node_points.  Invalid region");
				return_code = 0;
			}
			if (!coordinate_field)
			{
				display_message(WARNING_MESSAGE, "Must specify a coordinate field");
				return_code = 0;
			}
		}
		if (return_code)
		{
			if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
				graphics_object_name,command_data->graphics_object_list))
			{
				if (g_GLYPH_SET==GT_object_get_type(graphics_object))
				{
					if (GT_object_has_time(graphics_object,time))
					{
						display_message(WARNING_MESSAGE,
							"Overwriting time %g in graphics object '%s'",time,
							graphics_object_name);
						return_code = GT_object_remove_primitives_at_time(
							graphics_object, time,
							(GT_object_primitive_object_name_conditional_function *)NULL,
							(void *)NULL);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Object of different type named '%s' already exists",
						graphics_object_name);
					return_code=0;
				}
			}
			else
			{
				if ((graphics_object=CREATE(GT_object)(graphics_object_name,
					g_GLYPH_SET,material))&&
					ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list))
				{
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_node_points.  Could not create graphics object");
					DESTROY(GT_object)(&graphics_object);
					return_code=0;
				}
			}
			if (return_code)
			{
				rc_coordinate_field=
					Computed_field_begin_wrap_coordinate_field(coordinate_field);
				if (orientation_scale_field)
				{
					wrapper_orientation_scale_field=
						Computed_field_begin_wrap_orientation_scale_field(
							orientation_scale_field,rc_coordinate_field);
				}
				else
				{
					wrapper_orientation_scale_field=(struct Computed_field *)NULL;
				}
				base_size[0] = (FE_value)glyph_size[0];
				base_size[1] = (FE_value)glyph_size[1];
				base_size[2] = (FE_value)glyph_size[2];
				centre[0] = (FE_value)glyph_centre[0];
				centre[1] = (FE_value)glyph_centre[1];
				centre[2] = (FE_value)glyph_centre[2];
				scale_factors[0] = (FE_value)glyph_scale_factors[0];
				scale_factors[1] = (FE_value)glyph_scale_factors[1];
				scale_factors[2] = (FE_value)glyph_scale_factors[2];
				if (glyph_set = create_GT_glyph_set_from_FE_region_nodes(fe_region,
					rc_coordinate_field, glyph, base_size, centre, scale_factors, time,
					wrapper_orientation_scale_field, variable_scale_field,
					data_field, command_data->default_font, label_field, GRAPHICS_NO_SELECT,
					(struct LIST(FE_node) *)NULL))
				{
					if (!GT_OBJECT_ADD(GT_glyph_set)(graphics_object,time,glyph_set))
					{
						DESTROY(GT_glyph_set)(&glyph_set);
						return_code=0;
					}
				}
				if (wrapper_orientation_scale_field)
				{
					Computed_field_end_wrap(&wrapper_orientation_scale_field);
				}
				Computed_field_end_wrap(&rc_coordinate_field);
				if (return_code)
				{
					if (!GT_object_has_time(graphics_object,time))
					{
						/* add a NULL glyph_set to make an empty time */
						GT_OBJECT_ADD(GT_glyph_set)(graphics_object,time,
							(struct GT_glyph_set *)NULL);
					}
					if (data_field)
					{
						return_code=set_GT_object_Spectrum(graphics_object,spectrum);
					}
				}
				else
				{
					if (use_data)
					{
						display_message(WARNING_MESSAGE,"No data_glyphs created");
					}
					else
					{
						display_message(WARNING_MESSAGE,"No node_glyphs created");
					}
					if (0==GT_object_get_number_of_times(graphics_object))
					{
						REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list);
					}
				}
			} /* not duplicate name */
		} /* parse error, help */
		DESTROY(Option_table)(&option_table);
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		if (data_field)
		{
			DEACCESS(Computed_field)(&data_field);
		}
		if (orientation_scale_field)
		{
			DEACCESS(Computed_field)(&orientation_scale_field);
		}
		if (variable_scale_field)
		{
			DEACCESS(Computed_field)(&variable_scale_field);
		}
		DEACCESS(Spectrum)(&spectrum);
		DEACCESS(Graphical_material)(&material);
		DEALLOCATE(graphics_object_name);
		if (glyph)
		{
			DEACCESS(GT_object)(&glyph);
		}
		DEALLOCATE(region_path);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_node_points.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_node_points */

static int gfx_create_region(struct Parse_state *state,
	void *dummy, void *root_region_void)
/*******************************************************************************
LAST MODIFIED : 9 December 2005

DESCRIPTION :
Executes a GFX CREATE REGION command.
==============================================================================*/
{
	char *name;
	int return_code;
	struct Cmiss_region *region, *root_region;
	struct FE_region *fe_region;

	ENTER(gfx_create_region);
	USE_PARAMETER(dummy);
	if (state && (root_region = (struct Cmiss_region *)root_region_void))
	{
		name = (char *)NULL;
		if (set_name(state, (void *)&name, (void *)1))
		{
			if ((!state->current_token) ||
				(strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
			{
				return_code = 1;
				if (name && Cmiss_region_get_region_from_path(root_region, name, &region) &&
					region)
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_region.  Region '%s' already exists", name);
					return_code = 0;
				}
				if (return_code)
				{
					if (region = CREATE(Cmiss_region)())
					{
						ACCESS(Cmiss_region)(region);
						if (fe_region = CREATE(FE_region)(
							/*master_fe_region*/(struct FE_region *)NULL,
							/*the region should have it's own independent basis manager
							  and fe_shape_list */
							(struct MANAGER(FE_basis) *)NULL,
							(struct LIST(FE_element_shape) *)NULL))
						{
							ACCESS(FE_region)(fe_region);
							return_code = return_code &&
								Cmiss_region_attach_FE_region(region, fe_region) &&
								Cmiss_region_add_child_region(root_region, region, name,
									/*child_position*/-1);
							DEACCESS(FE_region)(&fe_region);
						}
						DEACCESS(Cmiss_region)(&region);
					}
				}
			}
		}
		if (name)
		{
			DEALLOCATE(name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_region */

static int gfx_create_scene(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 February 2003

DESCRIPTION :
Executes a GFX CREATE SCENE command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Modify_scene_data modify_scene_data;
	struct Scene *scene;

	ENTER(gfx_create_scene);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				/* set up data for modify_Scene since called in two places */
				modify_scene_data.light_manager=command_data->light_manager;
				modify_scene_data.scene_manager=command_data->scene_manager;
				modify_scene_data.default_scene=command_data->default_scene;
				/* following used for enabling GFEs */
				modify_scene_data.computed_field_manager=
					Computed_field_package_get_computed_field_manager(
						command_data->computed_field_package);
				modify_scene_data.root_region = command_data->root_region;
				modify_scene_data.data_root_region = command_data->data_root_region;
				modify_scene_data.element_point_ranges_selection=
					command_data->element_point_ranges_selection;
				modify_scene_data.element_selection=command_data->element_selection;
				modify_scene_data.node_selection=command_data->node_selection;
				modify_scene_data.data_selection=command_data->data_selection;
				modify_scene_data.user_interface=command_data->user_interface;
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(Scene,name)(
						current_token,command_data->scene_manager))
					{
						if (scene=CREATE(Scene)(current_token))
						{
							Scene_enable_graphics(scene,command_data->glyph_list,
								Material_package_get_material_manager(command_data->material_package),
								Material_package_get_default_material(command_data->material_package),
								command_data->default_font,
								command_data->light_manager,
								command_data->spectrum_manager,
								command_data->default_spectrum,
								command_data->texture_manager);
							Scene_enable_time_behaviour(scene,
								command_data->default_time_keeper);
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								return_code=modify_Scene(state,(void *)scene,
									(void *)(&modify_scene_data));
							}
							else
							{
								return_code=1;
							}
							ADD_OBJECT_TO_MANAGER(Scene)(scene,
								command_data->scene_manager);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_scene.  Error creating scene");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Scene already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					return_code=
						modify_Scene(state,(void *)NULL,(void *)(&modify_scene_data));
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_scene.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing scene name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_scene.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_scene */

static int gfx_create_snake(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 May 2006

DESCRIPTION :
Executes a GFX CREATE SNAKE command.
==============================================================================*/
{
	char *region_path;
	float density_factor, stiffness;
	int i, number_of_elements, number_of_fitting_fields, 
		previous_state_index, return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct Computed_field *coordinate_field, **fitting_fields,
		*weight_field;
	struct FE_region *fe_region;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_fitting_field_data, set_weight_field_data;
	struct Set_Computed_field_array_data set_fitting_field_array_data;

	ENTER(gfx_create_snake);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		region_path = (char *)NULL;
		number_of_fitting_fields = 1;
		coordinate_field = (struct Computed_field *)NULL;
		weight_field = (struct Computed_field *)NULL;
		density_factor = 0.0;
		number_of_elements = 1;
		stiffness = 0.0;

		if (strcmp(PARSER_HELP_STRING,state->current_token)&&
			strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
		{
			/* Skip this preprocessing if we are just getting the help */
			number_of_fitting_fields = 1;
			previous_state_index = state->current_index;

			option_table = CREATE(Option_table)();
			/* number_of_fitting_fields */
			Option_table_add_entry(option_table, "number_of_fitting_fields",
				&number_of_fitting_fields, NULL, set_int_positive);
			/* absorb everything else */
			Option_table_ignore_all_unmatched_entries(option_table);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			/* Return back to where we were */
			shift_Parse_state(state, previous_state_index - state->current_index);
		}

		if (number_of_fitting_fields)
		{
			ALLOCATE(fitting_fields, struct Computed_field *, number_of_fitting_fields);
			for (i = 0; i < number_of_fitting_fields; i++)
			{
				fitting_fields[i] = (struct Computed_field *)NULL;
			}
		}
		else
		{
			fitting_fields = (struct Computed_field **)NULL;
		}
		
		option_table = CREATE(Option_table)();
		/* coordinate */
		set_coordinate_field_data.conditional_function = 
			Computed_field_has_numerical_components;
		set_coordinate_field_data.conditional_function_user_data = (void *)NULL;
		set_coordinate_field_data.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		Option_table_add_entry(option_table, "coordinate",
			&coordinate_field, (void *)&set_coordinate_field_data,
			set_Computed_field_conditional);
		/* density_factor */
		Option_table_add_entry(option_table, "density_factor",
			&density_factor, NULL, set_float_0_to_1_inclusive);
		/* destination_group */
		Option_table_add_entry(option_table, "destination_group", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* fitting_fields */
		set_fitting_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_fitting_field_data.conditional_function_user_data = (void *)NULL;
		set_fitting_field_data.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_fitting_field_array_data.number_of_fields = number_of_fitting_fields;
		set_fitting_field_array_data.conditional_data = &set_fitting_field_data;
		Option_table_add_entry(option_table, "fitting_fields", fitting_fields,
			&set_fitting_field_array_data, set_Computed_field_array);
		/* number_of_fitting_fields */
		Option_table_add_entry(option_table, "number_of_fitting_fields",
			&number_of_fitting_fields, NULL, set_int_positive);
		/* number_of_elements */
		Option_table_add_entry(option_table, "number_of_elements",
			&number_of_elements, NULL, set_int_positive);
		/* stiffness */
		Option_table_add_entry(option_table, "stiffness",
			&stiffness, NULL, set_float_non_negative);
		/* weight_field */
		set_weight_field_data.conditional_function = 
			Computed_field_is_scalar;
		set_weight_field_data.conditional_function_user_data = (void *)NULL;
		set_weight_field_data.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		Option_table_add_entry(option_table, "weight_field",
			&weight_field, (void *)&set_weight_field_data,
			set_Computed_field_conditional);
		return_code = Option_table_multi_parse(option_table, state);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (!coordinate_field)
			{
				display_message(ERROR_MESSAGE, "gfx create snake.  "
					"Must specify a coordinate_field to define on elements in snake");
				return_code = 0;
			}
			if (!(region_path &&
				Cmiss_region_get_region_from_path(command_data->root_region,
					region_path, &region) &&
				(fe_region = Cmiss_region_get_FE_region(region))))
			{
				display_message(ERROR_MESSAGE, "gfx create snake.  "
					"Must specify a destination_group to put the snake elements in");
				return_code = 0;
			}
			if (return_code)
			{
				return_code = create_FE_element_snake_from_data_points(
					fe_region, coordinate_field, weight_field,
					number_of_fitting_fields, fitting_fields,
					FE_node_selection_get_node_list(command_data->data_selection),
					number_of_elements,
					density_factor,
					stiffness);
			}
		} /* parse error, help */
		DESTROY(Option_table)(&option_table);
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		if (weight_field)
		{
			DEACCESS(Computed_field)(&weight_field);
		}
		if (fitting_fields)
		{				
			for (i = 0; i < number_of_fitting_fields; i++)
			{
				if (fitting_fields[i])
				{
					DEACCESS(Computed_field)(&fitting_fields[i]);
				}
			}
			DEALLOCATE(fitting_fields);
		}
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_snake.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_snake */

static int gfx_modify_Spectrum(struct Parse_state *state,void *spectrum_void,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 30 April 1999

DESCRIPTION :
Modifier function that parses all the command line options for creating or
modifying a spectrum.
I would put this with the other gfx modify routines but then it can't be
static and referred to by gfx_create_Spectrum
==============================================================================*/
{
	char autorange, blue_to_red, blue_white_red, clear, *current_token, lg_blue_to_red,
		lg_red_to_blue, overlay_colour, overwrite_colour, red_to_blue;
	int process, range_set, return_code;
	float maximum, minimum;
	struct Cmiss_command_data *command_data;
	struct Modify_spectrum_data modify_spectrum_data;
	struct Option_table *option_table;
	struct Scene *autorange_scene;
	struct Spectrum *spectrum_to_be_modified,*spectrum_to_be_modified_copy;
	struct Spectrum_command_data spectrum_command_data;

	ENTER(gfx_modify_Spectrum);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				process=0;
				if (spectrum_to_be_modified=(struct Spectrum *)spectrum_void)
				{
					if (IS_MANAGED(Spectrum)(spectrum_to_be_modified,
						command_data->spectrum_manager))
					{
						if (spectrum_to_be_modified_copy=CREATE(Spectrum)(
							"spectrum_modify_temp"))
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)(
								spectrum_to_be_modified_copy,spectrum_to_be_modified);
							process=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Spectrum.  Could not create spectrum copy.");
							return_code=0;
						}
					}
					else
					{
						spectrum_to_be_modified_copy=spectrum_to_be_modified;
						spectrum_to_be_modified=(struct Spectrum *)NULL;
						process=1;
					}
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						if (spectrum_to_be_modified=FIND_BY_IDENTIFIER_IN_MANAGER(
							Spectrum,name)(current_token,
							command_data->spectrum_manager))
						{
							if (return_code=shift_Parse_state(state,1))
							{
								if (spectrum_to_be_modified_copy=CREATE(Spectrum)(
									"spectrum_modify_temp"))
								{
									MANAGER_COPY_WITH_IDENTIFIER(Spectrum,name)(
										spectrum_to_be_modified_copy,spectrum_to_be_modified);
									process=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
									"gfx_modify_Spectrum.  Could not create spectrum copy");
									return_code=0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown spectrum : %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						if (spectrum_to_be_modified=CREATE(Spectrum)("dummy"))
						{
							option_table=CREATE(Option_table)();
							Option_table_add_entry(option_table,"SPECTRUM_NAME",
								(void *)spectrum_to_be_modified,command_data_void,
								gfx_modify_Spectrum);
							return_code=Option_table_parse(option_table,state);
							DESTROY(Option_table)(&option_table);
							DESTROY(Spectrum)(&spectrum_to_be_modified);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Spectrum.  Could not create dummy spectrum");
							return_code=0;
						}
					}
				}
				if (process)
				{
					autorange = 0;
					autorange_scene = ACCESS(Scene)(command_data->default_scene);
					blue_to_red = 0;
					clear = 0;
					lg_blue_to_red = 0;
					lg_red_to_blue = 0;
					overlay_colour = 0;
					overwrite_colour = 0;
					red_to_blue = 0;
					blue_white_red = 0;
					modify_spectrum_data.position = 0;
					modify_spectrum_data.settings = (struct Spectrum_settings *)NULL;
					modify_spectrum_data.spectrum_minimum = get_Spectrum_minimum(
						spectrum_to_be_modified_copy);
					modify_spectrum_data.spectrum_maximum = get_Spectrum_maximum(
						spectrum_to_be_modified_copy);
					modify_spectrum_data.computed_field_manager 
						= Computed_field_package_get_computed_field_manager(
							command_data->computed_field_package);
					spectrum_command_data.spectrum_manager 
						= command_data->spectrum_manager;
					option_table=CREATE(Option_table)();
					Option_table_add_entry(option_table,"autorange",&autorange,NULL,
						set_char_flag);					
					Option_table_add_entry(option_table,"blue_to_red",&blue_to_red,NULL,
						set_char_flag);
					Option_table_add_entry(option_table,"blue_white_red",&blue_white_red,NULL,
						set_char_flag);
					Option_table_add_entry(option_table,"clear",&clear,NULL,
						set_char_flag);
					Option_table_add_entry(option_table,"field",&modify_spectrum_data,
						&spectrum_command_data,gfx_modify_spectrum_settings_field);
					Option_table_add_entry(option_table,"linear",&modify_spectrum_data,
						&spectrum_command_data,gfx_modify_spectrum_settings_linear);
					Option_table_add_entry(option_table,"log",&modify_spectrum_data,
						&spectrum_command_data,gfx_modify_spectrum_settings_log);
					Option_table_add_entry(option_table,"lg_blue_to_red",&lg_blue_to_red,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"lg_red_to_blue",&lg_red_to_blue,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"maximum",&spectrum_to_be_modified_copy,
						NULL,set_Spectrum_maximum_command);
					Option_table_add_entry(option_table,"minimum",&spectrum_to_be_modified_copy,
						NULL,set_Spectrum_minimum_command);
					Option_table_add_entry(option_table,"overlay_colour",&overlay_colour,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"overwrite_colour",&overwrite_colour,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"scene_for_autorange",&autorange_scene,
						command_data->scene_manager,set_Scene);
					Option_table_add_entry(option_table,"red_to_blue",&red_to_blue,
						NULL,set_char_flag);									
					if (return_code=Option_table_multi_parse(option_table,state))
					{
						if (return_code)
						{
							if ( clear )
							{
								Spectrum_remove_all_settings(spectrum_to_be_modified_copy);
							}
							if (blue_to_red + blue_white_red +red_to_blue + lg_red_to_blue + 
								lg_blue_to_red > 1 )
							{
								display_message(ERROR_MESSAGE,
									"gfx_modify_Spectrum.  Specify only one simple spectrum type\n "
									"   (blue_to_red, blue_white_red, red_to_blue, lg_red_to_blue, lg_blue_to_red)");
								return_code=0;
							}
							else if (red_to_blue)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									RED_TO_BLUE_SPECTRUM);
							}
							else if (blue_to_red)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									BLUE_TO_RED_SPECTRUM);
							}
							else if (blue_white_red)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									BLUE_WHITE_RED_SPECTRUM);
							}
							else if (lg_red_to_blue)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									LOG_RED_TO_BLUE_SPECTRUM);
							}
							else if (lg_blue_to_red)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									LOG_BLUE_TO_RED_SPECTRUM);
							}
							if ( modify_spectrum_data.settings )
							{
								/* add new settings */
								return_code=Spectrum_add_settings(spectrum_to_be_modified_copy,
									modify_spectrum_data.settings,
									modify_spectrum_data.position);
							}
							if (overlay_colour && overwrite_colour)
							{
								display_message(ERROR_MESSAGE,
									"gfx_modify_Spectrum.  Specify only one colour mode, overwrite_colour or overlay_colour");
								return_code=0;
							}
							else if (overlay_colour)
							{
								Spectrum_set_opaque_colour_flag(spectrum_to_be_modified_copy,
									0);
							}
							else if (overwrite_colour)
							{
								Spectrum_set_opaque_colour_flag(spectrum_to_be_modified_copy,
									1);
							}
							if (autorange)
							{
								/* Could also do all scenes */
								range_set = 0;
								Scene_get_data_range_for_spectrum(autorange_scene,
									spectrum_to_be_modified
									/* Not spectrum_to_be_modified_copy as this ptr 
										identifies the valid graphics objects */,
									&minimum, &maximum, &range_set);
								if ( range_set )
								{
									Spectrum_set_minimum_and_maximum(spectrum_to_be_modified_copy,
										minimum, maximum );
								}
							}
							if (spectrum_to_be_modified)
							{

								MANAGER_MODIFY_NOT_IDENTIFIER(Spectrum,name)(
									spectrum_to_be_modified,spectrum_to_be_modified_copy,
									command_data->spectrum_manager);
								DESTROY(Spectrum)(&spectrum_to_be_modified_copy);
							}
							else
							{
								spectrum_to_be_modified=spectrum_to_be_modified_copy;
							}
						}
						else
						{
							DESTROY(Spectrum)(&spectrum_to_be_modified_copy);
						}
					}
					if(option_table)
					{
						DESTROY(Option_table)(&option_table);
					}
					if ( modify_spectrum_data.settings )
					{
						DEACCESS(Spectrum_settings)(&(modify_spectrum_data.settings));
					}
					DEACCESS(Scene)(&autorange_scene);
				}
			}
			else
			{
				if (spectrum_void)
				{
					display_message(ERROR_MESSAGE,"Missing spectrum modifications");
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing spectrum name");
				}
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_Spectrum.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
				"gfx_modify_Spectrum.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Spectrum */

static int gfx_create_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE SPECTRUM command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Spectrum *spectrum;

	ENTER(gfx_create_spectrum);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(Spectrum,name)(
						current_token,command_data->spectrum_manager))
					{
						if (spectrum=CREATE(Spectrum)(current_token))
						{
							/*???DB.  Temporary */
							MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)(spectrum,
								command_data->default_spectrum);
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								return_code=gfx_modify_Spectrum(state,(void *)spectrum,
									command_data_void);
							}
							else
							{
								return_code=1;
							}
							if (return_code)
							{
								ADD_OBJECT_TO_MANAGER(Spectrum)(spectrum,
									command_data->spectrum_manager);
							}
							else
								DESTROY(Spectrum)(&spectrum);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_spectrum.  Error creating spectrum");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Spectrum already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					return_code=gfx_modify_Spectrum(state,(void *)NULL,command_data_void);
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_spectrum.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing spectrum name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_spectrum.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_spectrum */

static int gfx_create_streamlines(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
Executes a GFX CREATE STREAMLINES command.
==============================================================================*/
{
	char *graphics_object_name, *region_path, reverse_track,
		*seed_data_region_path, *streamline_data_type_string,
		*streamline_type_string, **valid_strings;
	enum GT_object_type graphics_object_type;
	enum Streamline_type streamline_type;
	enum Streamline_data_type streamline_data_type;
	FE_value seed_xi[3];
	float length,time,width;
	int number_of_components,number_of_valid_strings,return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region, *seed_data_region;
	struct Computed_field *coordinate_field,*data_field,*stream_vector_field;
	struct Element_to_streamline_data element_to_streamline_data;
	struct FE_element *seed_element;
	struct FE_field *seed_data_field;
	struct FE_region *fe_region, *seed_data_fe_region;
	struct Graphical_material *material;
	struct GT_object *graphics_object;
	struct Node_to_streamline_data node_to_streamline_data;
	struct Spectrum *spectrum;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data,set_stream_vector_field_data;
	struct Set_FE_field_conditional_FE_region_data set_seed_data_field_data;

	ENTER(gfx_create_streamlines);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		graphics_object_name = duplicate_string("streamlines");
		Cmiss_region_get_root_region_path(&region_path);
		coordinate_field=(struct Computed_field *)NULL;
		stream_vector_field=(struct Computed_field *)NULL;
		data_field=(struct Computed_field *)NULL;
		seed_element=(struct FE_element *)NULL;
		seed_data_region_path = (char *)NULL;
		seed_data_field = (struct FE_field *)NULL;
		time=0;
		length=1;
		width = 1;
		reverse_track = 0;
		number_of_components = 3;
		seed_xi[0] = 0.5;
		seed_xi[1] = 0.5;
		seed_xi[2] = 0.5;
		/* must access it now, because we deaccess it later */
		material=
			ACCESS(Graphical_material)(Material_package_get_default_material(command_data->material_package));
		spectrum=
			ACCESS(Spectrum)(command_data->default_spectrum);

		option_table=CREATE(Option_table)();
		/* as */
		Option_table_add_entry(option_table,"as",&graphics_object_name,
			(void *)1,set_name);
		/* coordinate */
		set_coordinate_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_coordinate_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"coordinate",&coordinate_field,
			&set_coordinate_field_data,set_Computed_field_conditional);
		/* data */
		set_data_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_data_field_data.conditional_function=
			Computed_field_has_numerical_components;
		set_data_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"data",&data_field,
			&set_data_field_data,set_Computed_field_conditional);
		/* ellipse/line/rectangle/ribbon */
		streamline_type = STREAM_LINE;
		streamline_type_string =
			ENUMERATOR_STRING(Streamline_type)(streamline_type);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Streamline_type)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Streamline_type) *)NULL, (void *)NULL);
		Option_table_add_enumerator(option_table,number_of_valid_strings,
			valid_strings,&streamline_type_string);
		DEALLOCATE(valid_strings);
		/* from */
		Option_table_add_entry(option_table, "from", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* length */
		Option_table_add_entry(option_table,"length",&length,NULL,set_float);
		/* material */
		Option_table_add_set_Material_entry(option_table, "material", &material,
			command_data->material_package);
		/* no_data/field_scalar/magnitude_scalar/travel_scalar */
		streamline_data_type = STREAM_NO_DATA;
		streamline_data_type_string =
			ENUMERATOR_STRING(Streamline_data_type)(streamline_data_type);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Streamline_data_type)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Streamline_data_type) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
			valid_strings, &streamline_data_type_string);
		DEALLOCATE(valid_strings);
		/* reverse */
		/*???RC use negative length to denote reverse track instead? */
		Option_table_add_entry(option_table,"reverse",
			&reverse_track,NULL,set_char_flag);
		/* seed_data_field */
		set_seed_data_field_data.conditional_function = FE_field_has_value_type;
		set_seed_data_field_data.user_data = (void *)ELEMENT_XI_VALUE;
		set_seed_data_field_data.fe_region =
			Cmiss_region_get_FE_region(command_data->data_root_region);
		Option_table_add_entry(option_table, "seed_data_field", &seed_data_field,
			(void *)&set_seed_data_field_data,
			set_FE_field_conditional_FE_region);
		/* seed_data_group */
		Option_table_add_entry(option_table, "seed_data_group",
			&seed_data_region_path,
			command_data->data_root_region, set_Cmiss_region_path);
		/* seed_element */
		Option_table_add_entry(option_table, "seed_element",
			&seed_element, Cmiss_region_get_FE_region(command_data->root_region),
			set_FE_element_dimension_3_FE_region);
		/* spectrum */
		Option_table_add_entry(option_table,"spectrum",&spectrum,
			command_data->spectrum_manager,set_Spectrum);
		/* time */
		Option_table_add_entry(option_table,"time",&time,NULL,set_float);
		/* vector */
		set_stream_vector_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_stream_vector_field_data.conditional_function=
			Computed_field_is_stream_vector_capable;
		set_stream_vector_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"vector",&stream_vector_field,
			&set_stream_vector_field_data,set_Computed_field_conditional);
		/* width */
		Option_table_add_entry(option_table,"width",&width,NULL,set_float);
		/* xi */
		Option_table_add_entry(option_table,"xi",
			seed_xi,&number_of_components,set_float_vector);
		if (return_code=Option_table_multi_parse(option_table,state))
		{
			fe_region = (struct FE_region *)NULL;
			if (!(Cmiss_region_get_region_from_path(command_data->root_region,
				region_path, &region) &&
				(fe_region = Cmiss_region_get_FE_region(region))))
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_iso_surfaces.  Invalid region");
				return_code = 0;
			}


			if (fe_region && seed_element &&
				(!FE_region_contains_FE_element(fe_region, seed_element)))
			{
				display_message(ERROR_MESSAGE,
					"seed_element is not in region");
				return_code=0;
			}
			if (!stream_vector_field)
			{
				display_message(ERROR_MESSAGE,"Must specify a vector");
				return_code=0;
			}
			if (!coordinate_field)
			{
				display_message(WARNING_MESSAGE, "Must specify a coordinate field");
				return_code = 0;
			}
			STRING_TO_ENUMERATOR(Streamline_type)(streamline_type_string,
				&streamline_type);
			STRING_TO_ENUMERATOR(Streamline_data_type)(streamline_data_type_string,
				&streamline_data_type);
			if (data_field)
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
			seed_data_fe_region = (struct FE_region *)NULL;
			if (seed_data_region_path)
			{
				if (!(Cmiss_region_get_region_from_path(
					command_data->data_root_region,
					seed_data_region_path, &seed_data_region) &&
					(seed_data_fe_region =
						Cmiss_region_get_FE_region(seed_data_region))))
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_streamlines.  Invalid seed_data_region");
					return_code = 0;
				}
			}
			if ((seed_data_field && (!seed_data_fe_region)) ||
				((!seed_data_field) && seed_data_fe_region))
			{
				display_message(ERROR_MESSAGE,
					"If you specify a seed_data_field then you must also specify a "
					"seed_data_group");
				return_code = 0;
			}
			if (return_code)
			{
				if (STREAM_LINE==streamline_type)
				{
					graphics_object_type=g_POLYLINE;
				}
				else
				{
					graphics_object_type=g_SURFACE;
				}
				if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
					graphics_object_name,command_data->graphics_object_list))
				{
					if (graphics_object_type == GT_object_get_type(graphics_object))
					{
						if (GT_object_has_time(graphics_object,time))
						{
							display_message(WARNING_MESSAGE,
								"Overwriting time %g in graphics object '%s'",time,
								graphics_object_name);
							return_code = GT_object_remove_primitives_at_time(
								graphics_object, time,
								(GT_object_primitive_object_name_conditional_function *)NULL,
								(void *)NULL);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Object of different type named '%s' already exists",
							graphics_object_name);
						return_code=0;
					}
				}
				else
				{
					if (STREAM_LINE==streamline_type)
					{
						graphics_object=CREATE(GT_object)(graphics_object_name,
							g_POLYLINE,material);
					}
					else
					{
						graphics_object=CREATE(GT_object)(graphics_object_name,
							g_SURFACE,material);
					}
					if (graphics_object&&ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_streamlines.  Could not create graphics object");
						DESTROY(GT_object)(&graphics_object);
						return_code=0;
					}
				}
				if (return_code)
				{
					if (seed_data_fe_region)
					{
						node_to_streamline_data.coordinate_field=
							Computed_field_begin_wrap_coordinate_field(coordinate_field);
						node_to_streamline_data.stream_vector_field=
							Computed_field_begin_wrap_orientation_scale_field(
								stream_vector_field,
								node_to_streamline_data.coordinate_field);
						node_to_streamline_data.type = streamline_type;
						node_to_streamline_data.data_type = streamline_data_type;
						node_to_streamline_data.data_field = data_field;
						node_to_streamline_data.fe_region = fe_region;
						node_to_streamline_data.graphics_object = graphics_object;
						node_to_streamline_data.length = length;
						node_to_streamline_data.width = width;
						node_to_streamline_data.time = time;
						/* reverse_track = track -vector and store -travel_scalar */
						node_to_streamline_data.reverse_track=(int)reverse_track;
						node_to_streamline_data.seed_data_field=seed_data_field;
						node_to_streamline_data.seed_element = seed_element;
						return_code = FE_region_for_each_FE_node(seed_data_fe_region,
							node_to_streamline, (void *)&node_to_streamline_data);
						Computed_field_end_wrap(
							&(node_to_streamline_data.stream_vector_field));
						Computed_field_end_wrap(
							&(node_to_streamline_data.coordinate_field));
					}
					else
					{
						element_to_streamline_data.coordinate_field=
							Computed_field_begin_wrap_coordinate_field(coordinate_field);
						element_to_streamline_data.stream_vector_field=
							Computed_field_begin_wrap_orientation_scale_field(
								stream_vector_field,
								element_to_streamline_data.coordinate_field);
						element_to_streamline_data.type = streamline_type;
						element_to_streamline_data.data_type = streamline_data_type;
						element_to_streamline_data.data_field = data_field;
						element_to_streamline_data.graphics_object = graphics_object;
						element_to_streamline_data.length = length;
						element_to_streamline_data.width = width;
						element_to_streamline_data.time = time;
						/* reverse_track = track -vector and store -travel_scalar */
						element_to_streamline_data.reverse_track=(int)reverse_track;
						element_to_streamline_data.seed_xi[0]=seed_xi[0];
						element_to_streamline_data.seed_xi[1]=seed_xi[1];
						element_to_streamline_data.seed_xi[2]=seed_xi[2];
						return_code = FE_region_for_each_FE_element(fe_region,
							element_to_streamline, (void *)&element_to_streamline_data);
						Computed_field_end_wrap(
							&(element_to_streamline_data.stream_vector_field));
						Computed_field_end_wrap(
							&(element_to_streamline_data.coordinate_field));
					}
					if (return_code)
					{
						if (!GT_object_has_time(graphics_object,time))
						{
							/* add a NULL primitive of the correct type to make an empty
								 time so there are no gaps in any time animation */
							switch (graphics_object_type)
							{
								case g_POLYLINE:
								{
									GT_OBJECT_ADD(GT_polyline)(graphics_object,time,
										(struct GT_polyline *)NULL);
								} break;
								case g_SURFACE:
								{
									GT_OBJECT_ADD(GT_surface)(graphics_object,time,
										(struct GT_surface *)NULL);
								} break;
							}
						}
						if (STREAM_NO_DATA != streamline_data_type)
						{
							return_code=set_GT_object_Spectrum(graphics_object,spectrum);
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"No streamlines created");
						if (0==GT_object_get_number_of_times(graphics_object))
						{
							REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
								command_data->graphics_object_list);
						}
					}
				}
			}
		} /* parse error, help */
		DESTROY(Option_table)(&option_table);
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		if (stream_vector_field)
		{
			DEACCESS(Computed_field)(&stream_vector_field);
		}
		if (data_field)
		{
			DEACCESS(Computed_field)(&data_field);
		}
		if (seed_data_field)
		{
			DEACCESS(FE_field)(&seed_data_field);
		}
		if (seed_data_region_path)
		{
			DEALLOCATE(seed_data_region_path);
		}
		if (seed_element)
		{
			DEACCESS(FE_element)(&seed_element);
		}
		DEALLOCATE(region_path);
		DEACCESS(Graphical_material)(&material);
		DEACCESS(Spectrum)(&spectrum);
		DEALLOCATE(graphics_object_name);
	}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_streamlines.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_streamlines */

static int gfx_create_surfaces(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 Februar 2002

DESCRIPTION :
Executes a GFX CREATE SURFACES command.
==============================================================================*/
{
	char exterior_flag,*graphics_object_name,nurb, *region_path,
		*render_type_string, reverse_normals,**valid_strings;
	enum GT_object_type object_type;
	enum Render_type render_type;
	float time;
	gtObject *graphics_object;
	int face_number,number_of_valid_strings,return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct Computed_field *coordinate_field,*data_field,*texture_coordinate_field;
	struct Element_discretization discretization;
	struct Element_to_surface_data element_to_surface_data;
	struct FE_region *fe_region;
	struct Graphical_material *material;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_texture_coordinate_field_data;
	struct Spectrum *spectrum;

	ENTER(gfx_create_surfaces);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		graphics_object_name = duplicate_string("surfaces");
		Cmiss_region_get_root_region_path(&region_path);
		coordinate_field=(struct Computed_field *)NULL;
		data_field=(struct Computed_field *)NULL;
		texture_coordinate_field=(struct Computed_field *)NULL;
		time=0;
		/* must access it now, because we deaccess it later */
		material=
			ACCESS(Graphical_material)(Material_package_get_default_material(command_data->material_package));
		spectrum=ACCESS(Spectrum)(command_data->default_spectrum);
		discretization.number_in_xi1=4;
		discretization.number_in_xi2=4;
		discretization.number_in_xi3=0;
		exterior_flag=0;
		face_number=-1;
		nurb=0;
		reverse_normals=0;

		option_table=CREATE(Option_table)();
		/* as */
		Option_table_add_entry(option_table,"as",&graphics_object_name,
			(void *)1,set_name);
		/* coordinate */
		set_coordinate_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_coordinate_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"coordinate",&coordinate_field,
			&set_coordinate_field_data,set_Computed_field_conditional);
		/* data */
		set_data_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_data_field_data.conditional_function=
			Computed_field_has_numerical_components;
		set_data_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"data",&data_field,
			&set_data_field_data,set_Computed_field_conditional);
		/* exterior */
		Option_table_add_entry(option_table,"exterior",&exterior_flag,
			NULL,set_char_flag);
		/* face */
		Option_table_add_entry(option_table,"face",&face_number,
			NULL,set_exterior);
		/* from */
		Option_table_add_entry(option_table, "from", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* material */
		Option_table_add_set_Material_entry(option_table, "material", &material,
			command_data->material_package);
		/* nurb */
		Option_table_add_entry(option_table,"nurb",&nurb,NULL,set_char_flag);
		/* reverse_normals */
		Option_table_add_entry(option_table,"reverse_normals",
			&reverse_normals,NULL,set_char_flag);
		/* render_type */
		render_type = RENDER_TYPE_SHADED;
		render_type_string = ENUMERATOR_STRING(Render_type)(render_type);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Render_type)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Render_type) *)NULL, (void *)NULL);
		Option_table_add_enumerator(option_table,number_of_valid_strings,
			valid_strings,&render_type_string);
		DEALLOCATE(valid_strings);
		/* spectrum */
		Option_table_add_entry(option_table,"spectrum",&spectrum,
			command_data->spectrum_manager,set_Spectrum);
		/* texture_coordinates */
		set_texture_coordinate_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_texture_coordinate_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_texture_coordinate_field_data.conditional_function_user_data=
			(void *)NULL;
		Option_table_add_entry(option_table,"texture_coordinates",
			&texture_coordinate_field,&set_texture_coordinate_field_data,
			set_Computed_field_conditional);
		/* time */
		Option_table_add_entry(option_table,"time",&time,NULL,set_float);
		/* with */
		Option_table_add_entry(option_table,"with",&discretization,
			command_data->user_interface,set_Element_discretization);
		return_code=Option_table_multi_parse(option_table,state);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (!(Cmiss_region_get_region_from_path(command_data->root_region,
				region_path, &region) &&
				(fe_region = Cmiss_region_get_FE_region(region))))
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_surfaces.  Invalid region");
				return_code = 0;
			}
			if (!coordinate_field)
			{
				display_message(WARNING_MESSAGE, "Missing coordinate field");
				return_code = 0;
			}
		}
		if (return_code)
		{
			if (nurb)
			{
				object_type = g_NURBS;
			}
			else
			{
				object_type = g_SURFACE;
			}
			if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
				graphics_object_name,command_data->graphics_object_list))
			{
				if (object_type==GT_object_get_type(graphics_object))
				{
					if (GT_object_has_time(graphics_object,time))
					{
						display_message(WARNING_MESSAGE,
							"Overwriting time %g in graphics object '%s'",time,
							graphics_object_name);
						return_code = GT_object_remove_primitives_at_time(
							graphics_object, time,
							(GT_object_primitive_object_name_conditional_function *)NULL,
							(void *)NULL);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Object of different type named '%s' already exists",
						graphics_object_name);
					return_code=0;
				}
			}
			else
			{
				if ((graphics_object=CREATE(GT_object)(graphics_object_name,
					object_type,material))&&
					ADD_OBJECT_TO_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list))
				{
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_surfaces.  Could not create graphics object");
					DESTROY(GT_object)(&graphics_object);
					return_code=0;
				}
			}
		}
		if (return_code)
		{
			element_to_surface_data.coordinate_field=
				Computed_field_begin_wrap_coordinate_field(coordinate_field);
			element_to_surface_data.fe_region = fe_region;
			element_to_surface_data.exterior=exterior_flag;
			element_to_surface_data.face_number=face_number;
			element_to_surface_data.object_type = object_type;
			element_to_surface_data.data_field=data_field;
			element_to_surface_data.reverse_normals=reverse_normals;
			element_to_surface_data.graphics_object=graphics_object;
			STRING_TO_ENUMERATOR(Render_type)(render_type_string, &render_type);
			element_to_surface_data.render_type = render_type;
			element_to_surface_data.texture_coordinate_field=
				texture_coordinate_field;
			element_to_surface_data.time=time;
			element_to_surface_data.number_of_segments_in_xi1=
				discretization.number_in_xi1;
			element_to_surface_data.number_of_segments_in_xi2=
				discretization.number_in_xi2;
			return_code = FE_region_for_each_FE_element(fe_region,
				element_to_surface,(void *)&element_to_surface_data);
			if (return_code)
			{
				if (!GT_object_has_time(graphics_object,time))
				{
					/* add a NULL surface to make an empty time */
					GT_OBJECT_ADD(GT_surface)(graphics_object,time,
						(struct GT_surface *)NULL);
				}
				if (data_field)
				{
					return_code=set_GT_object_Spectrum(graphics_object,spectrum);
				}
			}
			else
			{
				if (return_code)
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_surfaces.  No surfaces created");
					return_code=0;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_surfaces.  Error creating surfaces");
				}
				if (0==GT_object_get_number_of_times(graphics_object))
				{
					REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
						command_data->graphics_object_list);
				}
			}
			Computed_field_end_wrap(&(element_to_surface_data.coordinate_field));
		} /* parse error, help */
		DESTROY(Option_table)(&option_table);
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		if (data_field)
		{
			DEACCESS(Computed_field)(&data_field);
		}
		if (texture_coordinate_field)
		{
			DEACCESS(Computed_field)(&texture_coordinate_field);
		}
		DEALLOCATE(region_path);
		DEACCESS(Graphical_material)(&material);
		DEACCESS(Spectrum)(&spectrum);
		DEALLOCATE(graphics_object_name);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_create_surfaces.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_surfaces */

static int set_Texture_image_from_field(struct Texture *texture,
	struct Computed_field *field,
	struct Computed_field *texture_coordinate_field,
	int propagate_field,
	struct Spectrum *spectrum,
	struct Cmiss_region *region,
	int element_dimension,
	enum Texture_storage_type storage,
	int image_width, int image_height, int image_depth,
	struct Graphics_buffer_package *graphics_buffer_package,
	struct Graphical_material *fail_material)
/*******************************************************************************
LAST MODIFIED : 30 June 2006

DESCRIPTION :
Creates the image in the format given by sampling the <field> according to the
reverse mapping of the <texture_coordinate_field>.  The values returned by
field are converted to "colours" by applying the <spectrum>.
Currently limited to 1 byte per component.
An <element_dimension> of 0 searches in elements of all dimension, any other
value searches just elements of that dimension.
==============================================================================*/
{
	char *field_name;
	unsigned char *image_plane, *ptr;
	FE_value *data_values, values[3], xi[3];
	float hint_minimums[3] = {0.0, 0.0, 0.0};
	float hint_maximums[3];
	float hint_resolution[3];
	float	rgba[4], fail_alpha, texture_depth, texture_height,
		texture_width;
	int bytes_per_pixel, dimension, i, image_width_bytes, j, k,
		number_of_bytes_per_component, number_of_components,
		number_of_data_components, return_code, source_dimension,
		*source_sizes, tex_number_of_components, use_pixel_location;
	unsigned long field_evaluate_error_count, find_element_xi_error_count,
		spectrum_render_error_count, total_number_of_pixels;
	struct Colour fail_colour;
	struct Computed_field *source_texture_coordinate_field;
	struct Computed_field_find_element_xi_cache *cache;
	struct FE_element *element;

	ENTER(set_Texture_image_from_field);
	if (texture && field && spectrum && region &&
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
		if (image_depth > 1)
		{
			dimension = 3;
		}
		else
		{
			if (image_height > 1)
			{
				dimension = 2;
			}
			else
			{
				dimension = 1;
			}
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
		number_of_bytes_per_component = 1;
		/* allocate the texture image */
		field_name = (char *)NULL;
		use_pixel_location = 1;
		GET_NAME(Computed_field)(field, &field_name);
		if (Texture_allocate_image(texture, image_width, image_height,
			image_depth, storage, number_of_bytes_per_component, field_name))
		{
			cache = (struct Computed_field_find_element_xi_cache *)NULL;
			number_of_data_components =
				Computed_field_get_number_of_components(field);
			Texture_get_physical_size(texture, &texture_width, &texture_height,
				&texture_depth);
			/* allocate space for a single image plane */
			bytes_per_pixel = number_of_components*number_of_bytes_per_component;
			image_width_bytes = image_width*bytes_per_pixel;
			ALLOCATE(image_plane, unsigned char, image_height*image_width_bytes);
			ALLOCATE(data_values, FE_value,
				Computed_field_get_number_of_components(field));
			for (i = 0; (Computed_field_get_number_of_components(field) >i);  i++)
			{
				 data_values[i]=0.0;
			}
			Graphical_material_get_diffuse(fail_material, &fail_colour);
			Graphical_material_get_alpha(fail_material, &fail_alpha);
			if (image_plane && data_values)
			{
				hint_resolution[0] = image_width;
				hint_resolution[1] = image_height;
				hint_resolution[2] = image_depth;
				field_evaluate_error_count = 0;
				find_element_xi_error_count = 0;
				spectrum_render_error_count = 0;
				total_number_of_pixels = image_width*image_height*image_depth;
				hint_maximums[0] = texture_width;
				hint_maximums[1] = texture_height;
				hint_maximums[2] = texture_depth;
				for (i = 0; (i < image_depth) && return_code; i++)
				{
					/*???debug -- leave in so user knows something is happening! */
					if (1 < image_depth)
					{
						printf("Evaluating image plane %d of %d\n", i+1, image_depth);
					}
					ptr = (unsigned char *)image_plane;
					values[2] = texture_depth * ((float)i + 0.5) / (float)image_depth;
					for (j = 0; (j < image_height) && return_code; j++)
					{
						values[1] = texture_height * ((float)j + 0.5) / (float)image_height;
						for (k = 0; (k < image_width) && return_code; k++)
						{
							values[0] = texture_width * ((float)k + 0.5) / (float)image_width;
#if defined (DEBUG)
							/*???debug*/
							if ((1 < image_depth) && ((0 == j) || (image_height - 1 == j)) && ((0 == k) || (image_width - 1 == k)))
							{
								printf("  field pos = %10g %10g %10g\n", values[0], values[1], values[2]);
							}
#endif /* defined (DEBUG) */
							if (use_pixel_location)
							{
								/* Try to use a pixel coordinate first */
								if (Computed_field_evaluate_at_field_coordinates(field,
									texture_coordinate_field, dimension, values, 
										/*time*/0.0, data_values))
								{
									if (!Spectrum_value_to_rgba(spectrum,
											number_of_data_components, data_values,
											rgba))
									{
										rgba[0] = fail_colour.red;
										rgba[1] = fail_colour.green;
										rgba[2] = fail_colour.blue;
										rgba[3] = fail_alpha;
										spectrum_render_error_count++;
									}
								}
								else
								{
									use_pixel_location = 0;
								}
							}
							if (!use_pixel_location)
							{
								/* Otherwise find a valid element xi location */
								/* Computed_field_find_element_xi_special returns true if it has
									performed a valid calculation even if the element isn't found
									to stop the slow Computed_field_find_element_xi being called */
								if (Computed_field_find_element_xi_special(
										 texture_coordinate_field, &cache, values,
										 tex_number_of_components, &element, xi,
										 region, element_dimension,
										 graphics_buffer_package,
										 hint_minimums, hint_maximums, hint_resolution) ||
									Computed_field_find_element_xi(texture_coordinate_field,
										values, tex_number_of_components, &element, xi,
										element_dimension, region, propagate_field,
										/*find_nearest_location*/0))
								{
									if (element)
									{
#if defined (DEBUG)
										/*???debug*/
										if ((1 < image_depth) && ((0 == j) || (image_height - 1 == j)) && ((0 == k) || (image_width - 1 == k)))
										{
											printf("  xi = %10g %10g %10g\n", xi[0], xi[1], xi[2]);
										}
#endif /* defined (DEBUG) */
										if (Computed_field_evaluate_in_element(field,
												element, xi,/*time*/0,(struct FE_element *)NULL,
												data_values, (FE_value *)NULL))
										{
											if (!Spectrum_value_to_rgba(spectrum,
													number_of_data_components, data_values,
													rgba))
											{
												rgba[0] = fail_colour.red;
												rgba[1] = fail_colour.green;
												rgba[2] = fail_colour.blue;
												rgba[3] = fail_alpha;
												spectrum_render_error_count++;
											}
										}
										else
										{
											rgba[0] = fail_colour.red;
											rgba[1] = fail_colour.green;
											rgba[2] = fail_colour.blue;
											rgba[3] = fail_alpha;
											field_evaluate_error_count++;
										}
									}
									else
									{
										rgba[0] = fail_colour.red;
										rgba[1] = fail_colour.green;
										rgba[2] = fail_colour.blue;
										/* not in any element; set alpha to zero so invisible */
										rgba[3] = fail_alpha;
									}
								}
								else
								{
									rgba[0] = fail_colour.red;
									rgba[1] = fail_colour.green;
									rgba[2] = fail_colour.blue;
									/* error finding element:xi; set alpha to zero so invisible */
									rgba[3] = fail_alpha;
									find_element_xi_error_count++;
								}
							}
#if defined (DEBUG)
							/*???debug*/
							if ((1 < image_depth) && ((0 == j) || (image_height - 1 == j)) && ((0 == k) || (image_width - 1 == k)))
							{
								printf("  RGBA = %10g %10g %10g %10g\n", rgba[0], rgba[1], rgba[2], rgba[3]);
							}
#endif /* defined (DEBUG) */
							switch (storage)
							{
								case TEXTURE_LUMINANCE:
								{
									*ptr = (unsigned char)((rgba[0] + rgba[1] + rgba[2]) * 255.0 / 3.0);
									ptr++;
								} break;
								case TEXTURE_LUMINANCE_ALPHA:
								{
									*ptr = (unsigned char)((rgba[0] + rgba[1] + rgba[2]) * 255.0 / 3.0);
									ptr++;
									*ptr = (unsigned char)(rgba[3] * 255.0);
									ptr++;
								} break;
								case TEXTURE_RGB:
								{
									*ptr = (unsigned char)(rgba[0] * 255.0);
									ptr++;
									*ptr = (unsigned char)(rgba[1] * 255.0);
									ptr++;
									*ptr = (unsigned char)(rgba[2] * 255.0);
									ptr++;
								} break;
								case TEXTURE_RGBA:
								{
									*ptr = (unsigned char)(rgba[0] * 255.0);
									ptr++;
									*ptr = (unsigned char)(rgba[1] * 255.0);
									ptr++;
									*ptr = (unsigned char)(rgba[2] * 255.0);
									ptr++;
									*ptr = (unsigned char)(rgba[3] * 255.0);
									ptr++;
								} break;
								case TEXTURE_ABGR:
								{
									*ptr = (unsigned char)(rgba[3] * 255.0);
									ptr++;
									*ptr = (unsigned char)(rgba[2] * 255.0);
									ptr++;
									*ptr = (unsigned char)(rgba[1] * 255.0);
									ptr++;
									*ptr = (unsigned char)(rgba[0] * 255.0);
									ptr++;
								} break;
								default:
								{
									display_message(ERROR_MESSAGE,
										"set_Texture_image_from_field.  Unsupported storage type");
									return_code = 0;
								} break;
							}
						}
					}
					if (!Texture_set_image_block(texture,
						/*left*/0, /*bottom*/0, image_width, image_height, /*depth_plane*/i,
						image_width_bytes, image_plane))
					{
						display_message(ERROR_MESSAGE,
							"set_Texture_image_from_field.  Could not set texture block");
						return_code = 0;
					}
				}
				Computed_field_clear_cache(field);
				Computed_field_clear_cache(texture_coordinate_field);
				Spectrum_end_value_to_rgba(spectrum);
				if (0 < field_evaluate_error_count)
				{
					display_message(WARNING_MESSAGE, "set_Texture_image_from_field.  "
						"Field could not be evaluated in element for %d out of %d pixels",
						field_evaluate_error_count, total_number_of_pixels);
				}
				if (0 < spectrum_render_error_count)
				{
					display_message(WARNING_MESSAGE, "set_Texture_image_from_field.  "
						"Spectrum could not be evaluated for %d out of %d pixels",
						spectrum_render_error_count, total_number_of_pixels);
				}
				if (0 < find_element_xi_error_count)
				{
					display_message(WARNING_MESSAGE, "set_Texture_image_from_field.  "
						"Unable to find element:xi for %d out of %d pixels",
						find_element_xi_error_count, total_number_of_pixels);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_Texture_image_from_field.  Not enough memory");
				return_code = 0;
			}
			DEALLOCATE(data_values);
			DEALLOCATE(image_plane);
			if (cache)
			{
				DESTROY(Computed_field_find_element_xi_cache)(&cache);
			}
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

int set_element_dimension_or_all(struct Parse_state *state,
	void *value_address_void, void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
Allows either "all" - a return value of zero - or an element dimension up to
MAXIMUM_ELEMENT_XI_DIMENSIONS to be set.
==============================================================================*/
{
	char *current_token;
	int return_code, value, *value_address;

	ENTER(set_element_dimension_or_all);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (value_address = (int *)value_address_void)
				{
					if (fuzzy_string_compare_same_length(current_token, "ALL"))
					{
						*value_address = 0;
					}
					else if ((1 == sscanf(current_token, " %d ", &value)) &&
						(0 < value) && (value <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
					{
						*value_address = value;
						return_code = shift_Parse_state(state, 1);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Invalid element dimension: %s\n", current_token);
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_element_dimension_or_all.  Missing value_address");
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " #|ALL");
				if (value_address = (int *)value_address_void)
				{
					if (0 == *value_address)
					{
						display_message(INFORMATION_MESSAGE, "[ALL]");
					}
					else
					{
						display_message(INFORMATION_MESSAGE, "[%d]", *value_address);
					}
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing element dimension or ALL");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_element_dimension_or_all.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_element_dimension_or_all */

struct Texture_evaluate_image_data
{
	char *region_path;
	int element_dimension; /* where 0 is any dimension */
	int propagate_field;
	struct Computed_field *field, *texture_coordinates_field;
	struct Graphical_material *fail_material;
	struct Spectrum *spectrum;
};

static int gfx_modify_Texture_evaluate_image(struct Parse_state *state,
	void *data_void, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
Modifies the properties of a texture.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_field_data,
		set_texture_coordinates_field_data;
	struct Texture_evaluate_image_data *data;

	ENTER(gfx_modify_Texture_evaluate_image);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void)
		&& (data = (struct Texture_evaluate_image_data *)data_void))
	{
		if (state->current_token)
		{
			option_table = CREATE(Option_table)();
			/* element_dimension */
			Option_table_add_entry(option_table, "element_dimension",
				&data->element_dimension, NULL, set_element_dimension_or_all);
			/* element_group */
			Option_table_add_entry(option_table, "element_group", &data->region_path,
				command_data->root_region, set_Cmiss_region_path);
			/* fail_material */
			Option_table_add_set_Material_entry(option_table, "fail_material", 
				&data->fail_material, command_data->material_package);
			/* field */
			set_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table, "field", &data->field,
				&set_field_data, set_Computed_field_conditional);
			/* propagate_field/no_propagate_field */
			Option_table_add_switch(option_table, "propagate_field",
				"no_propagate_field", &data->propagate_field);
			/* spectrum */
			Option_table_add_entry(option_table, "spectrum", &data->spectrum, 
				command_data->spectrum_manager, set_Spectrum);
			/* texture_coordinates */
			set_texture_coordinates_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_texture_coordinates_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_texture_coordinates_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table, "texture_coordinates",
				&data->texture_coordinates_field,
				&set_texture_coordinates_field_data, set_Computed_field_conditional);

			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_Texture_evaluate_image.  Missing evaluate image options");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_Texture_evaluate_image.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Texture_evaluate_image */

struct Texture_image_data
{
	char *image_file_name;
	int crop_bottom_margin,crop_left_margin,crop_height,crop_width;
};

static int gfx_modify_Texture_image(struct Parse_state *state,
	void *data_void, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 January 2002

DESCRIPTION :
Modifies the properties of a texture.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Texture_image_data *data;

	ENTER(gfx_modify_Texture_image);
	if (state && (data = (struct Texture_image_data *)data_void) &&
		(command_data = (struct Cmiss_command_data *)command_data_void))
	{
		return_code=1;
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (fuzzy_string_compare("crop",current_token))
				{
					if (!(shift_Parse_state(state,1)&&
						(current_token=state->current_token)&&
						(1==sscanf(current_token," %d",&(data->crop_left_margin)))&&
						shift_Parse_state(state,1)&&(current_token=state->current_token)&&
						(1==sscanf(current_token," %d",&(data->crop_bottom_margin)))&&
						shift_Parse_state(state,1)&&(current_token=state->current_token)&&
						(1==sscanf(current_token," %d",&(data->crop_width)))&&
						shift_Parse_state(state,1)&&(current_token=state->current_token)&&
						(1==sscanf(current_token," %d",&(data->crop_height)))&&
						shift_Parse_state(state,1)))
					{
						display_message(WARNING_MESSAGE,"Missing/invalid crop value(s)");
						display_parse_state_location(state);
						return_code=0;
					}
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" <crop LEFT_MARGIN#[0] BOTTOM_MARGIN#[0] WIDTH#[0] HEIGHT#[0]>");
			}
		}
		if (return_code)
		{
			if (current_token=state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* example */
				Option_table_add_entry(option_table, CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
					&(data->image_file_name), &(command_data->example_directory),
					set_file_name);
				/* default */
				Option_table_add_entry(option_table, NULL, &(data->image_file_name),
					NULL, set_file_name);
				return_code = Option_table_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx modify texture image:  Missing image file name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_Texture_image.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Texture_image */

struct Texture_file_number_series_data
{
	int start, stop, increment;
};

static int gfx_modify_Texture_file_number_series(struct Parse_state *state,
	void *data_void, void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 8 February 2002

DESCRIPTION :
Modifies the properties of a texture.
==============================================================================*/
{
	char *current_token;
	int range, return_code;
	struct Texture_file_number_series_data *data;

	ENTER(gfx_modify_Texture_file_number_series);
	USE_PARAMETER(dummy_user_data);
	if (state && (data = (struct Texture_file_number_series_data *)data_void))
	{
		return_code = 1;
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if ((1 == sscanf(current_token, " %d", &(data->start))) &&
					shift_Parse_state(state, 1) &&
					(current_token = state->current_token) &&
					(1 == sscanf(current_token, " %d", &(data->stop))) &&
					shift_Parse_state(state, 1) &&
					(current_token = state->current_token) &&
					(1 == sscanf(current_token, " %d", &(data->increment))) &&
					shift_Parse_state(state, 1))
				{
					/* check range proceeds from start to stop with a whole number of
						 increments, and that increment is positive */
					if (!(((0 < data->increment) &&
						(0 <= (range = data->stop - data->start)) &&
						(0 == (range % data->increment))) ||
						((0 > data->increment) &&
							(0 <= (range = data->start - data->stop))
							&& (0 == (range % -data->increment)))))
					{
						display_message(ERROR_MESSAGE,
							"Invalid file number series");
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Missing 3-D image series START, STOP or INCREMENT");
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " START STOP INCREMENT");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_Texture_file_number_series.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Texture_file_number_series */

int gfx_modify_Texture(struct Parse_state *state,void *texture_void,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 February 2003

DESCRIPTION :
Modifies the properties of a texture.
==============================================================================*/
{
	char *combine_mode_string, *compression_mode_string, *current_token, 
		*file_number_pattern, *filter_mode_string, *raw_image_storage_string,
		*resize_filter_mode_string, **valid_strings, *wrap_mode_string;
	double texture_distortion[3];
	enum Raw_image_storage raw_image_storage;
	enum Texture_combine_mode combine_mode;
	enum Texture_compression_mode compression_mode;
	enum Texture_filter_mode filter_mode;
	enum Texture_resize_filter_mode resize_filter_mode;
	enum Texture_storage_type specify_format;
	enum Texture_wrap_mode wrap_mode;
	float alpha, depth, distortion_centre_x, distortion_centre_y,
		distortion_factor_k1, height, width;
	int file_number, i, number_of_file_names, number_of_valid_strings, process,
		return_code, specify_depth, specify_height,
		specify_number_of_bytes_per_component, specify_width, texture_is_managed;
	struct Cmgui_image *cmgui_image;
	struct Cmgui_image_information *cmgui_image_information;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *evaluate_data_region;
	struct Colour colour;
	struct Option_table *option_table;
	struct Texture *texture, *texture_copy;
	struct Texture_evaluate_image_data evaluate_data;
	struct Texture_image_data image_data;
	struct Texture_file_number_series_data file_number_series_data;
	/* do not make the following static as 'set' flag must start at 0 */
	struct Set_vector_with_help_data texture_distortion_data=
		{3," DISTORTION_CENTRE_X DISTORTION_CENTRE_Y DISTORTION_FACTOR_K1",0};
#if defined (SGI_MOVIE_FILE)
	struct Movie_graphics *movie, *old_movie;
	struct X3d_movie *x3d_movie;
#endif /* defined (SGI_MOVIE_FILE) */

	ENTER(gfx_modify_Texture);
	cmgui_image_information = NULL;
	if (state)
	{
		if (current_token = state->current_token)
		{
			if (command_data = (struct Cmiss_command_data *)command_data_void)
			{
				process = 0;
				if (texture = (struct Texture *)texture_void)
				{
					texture_is_managed =
						IS_MANAGED(Texture)(texture, command_data->texture_manager);
					process = 1;
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						if (texture = FIND_BY_IDENTIFIER_IN_MANAGER(Texture,name)(
							current_token, command_data->texture_manager))
						{
							texture_is_managed = 1;
							process = 1;
							return_code = shift_Parse_state(state, 1);
						}
						else
						{
							display_message(ERROR_MESSAGE, "Unknown texture : %s",
								current_token);
							display_parse_state_location(state);
							return_code = 0;
						}
					}
					else
					{
						if (texture = CREATE(Texture)((char *)NULL))
						{
							option_table = CREATE(Option_table)();
							Option_table_add_entry(option_table, "TEXTURE_NAME",
								(void *)texture, command_data_void, gfx_modify_Texture);
							return_code = Option_table_parse(option_table, state);
							/*???DB.  return_code will be 0 ? */
							DESTROY(Option_table)(&option_table);
							DESTROY(Texture)(&texture);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Texture.  Could not create dummy texture");
							return_code = 0;
						}
					}
				}
				if (process)
				{
#if defined (SGI_MOVIE_FILE)
					if (x3d_movie=Texture_get_movie(texture))
					{
						if (movie = FIRST_OBJECT_IN_MANAGER_THAT(Movie_graphics)(
							Movie_graphics_has_X3d_movie, (void *)x3d_movie,
							command_data->movie_graphics_manager))
						{
							ACCESS(Movie_graphics)(movie);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Texture.  Missing Movie_graphics for X3d_movie");
						}
					}
					else
					{
						movie = (struct Movie_graphics *)NULL;
					}
					old_movie = movie;
#endif /* defined (SGI_MOVIE_FILE) */
					Texture_get_combine_alpha(texture, &alpha);
					Texture_get_combine_colour(texture, &colour);
					Texture_get_physical_size(texture,
						&width, &height, &depth);
					Texture_get_distortion_info(texture,
						&distortion_centre_x,&distortion_centre_y,&distortion_factor_k1);
					texture_distortion[0]=(double)distortion_centre_x;
					texture_distortion[1]=(double)distortion_centre_y;
					texture_distortion[2]=(double)distortion_factor_k1;

					specify_format=TEXTURE_RGB;
					specify_width=0;
					specify_height=0;
					specify_depth=0;
					specify_number_of_bytes_per_component=0;

					image_data.image_file_name=(char *)NULL;
					image_data.crop_left_margin=0;
					image_data.crop_bottom_margin=0;
					image_data.crop_width=0;
					image_data.crop_height=0;

					evaluate_data.region_path = (char *)NULL;
					evaluate_data.element_dimension = 0; /* any dimension */
					evaluate_data.propagate_field = 1;
					evaluate_data.field = (struct Computed_field *)NULL;
					evaluate_data.texture_coordinates_field =
						(struct Computed_field *)NULL;
					/* Try for the special transparent gray material first */
					if (!(evaluate_data.fail_material = 
						FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material, name)(
						"transparent_gray50", Material_package_get_material_manager(
							command_data->material_package))))
					{
						/* Just use the default material */
						evaluate_data.fail_material = Material_package_get_default_material(
							command_data->material_package);
					}
					ACCESS(Graphical_material)(evaluate_data.fail_material);
					evaluate_data.spectrum = (struct Spectrum *)NULL;

					file_number_pattern = (char *)NULL;
					/* increment must be non-zero for following to be "set" */
					file_number_series_data.start = 0;
					file_number_series_data.stop = 0;
					file_number_series_data.increment = 0;

					option_table = CREATE(Option_table)();
					/* alpha */
					Option_table_add_entry(option_table, "alpha", &alpha,
					  NULL,set_float_0_to_1_inclusive);
					/* blend/decal/modulate */
					combine_mode_string = ENUMERATOR_STRING(Texture_combine_mode)(
						Texture_get_combine_mode(texture));
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Texture_combine_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Texture_combine_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table, number_of_valid_strings,
						valid_strings, &combine_mode_string);
					DEALLOCATE(valid_strings);
					/* clamp_wrap/repeat_wrap */
					wrap_mode_string = ENUMERATOR_STRING(Texture_wrap_mode)(
						Texture_get_wrap_mode(texture));
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Texture_wrap_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Texture_wrap_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&wrap_mode_string);
					DEALLOCATE(valid_strings);
					/* colour */
					Option_table_add_entry(option_table, "colour", &colour,
					  NULL,set_Colour);
					/* compressed_unspecified/uncompressed */
					compression_mode_string = ENUMERATOR_STRING(Texture_compression_mode)(
						Texture_get_compression_mode(texture));
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Texture_compression_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Texture_compression_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table, number_of_valid_strings,
						valid_strings, &compression_mode_string);
					DEALLOCATE(valid_strings);
					/* depth */
					Option_table_add_entry(option_table, "depth", &depth,
					  NULL, set_float_non_negative);
					/* distortion */
					Option_table_add_entry(option_table, "distortion",
						&texture_distortion,
					  &texture_distortion_data,set_double_vector_with_help);
					/* height */
					Option_table_add_entry(option_table, "height", &height,
					  NULL,set_float_non_negative);
					/* image */
					Option_table_add_entry(option_table, "image",
						&image_data, command_data, gfx_modify_Texture_image);
					/* linear_filter/nearest_filter */
					filter_mode_string = ENUMERATOR_STRING(Texture_filter_mode)(
						Texture_get_filter_mode(texture));
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Texture_filter_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Texture_filter_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&filter_mode_string);
					DEALLOCATE(valid_strings);
#if defined (SGI_MOVIE_FILE)
					/* movie */
					Option_table_add_entry(option_table, "movie", &movie,
					  command_data->movie_graphics_manager, set_Movie_graphics);
#endif /* defined (SGI_MOVIE_FILE) */
					/* number_pattern */
					Option_table_add_entry(option_table, "number_pattern",
						&file_number_pattern, (void *)1, set_name);
					/* number_series */
					Option_table_add_entry(option_table, "number_series",
						&file_number_series_data, NULL,
						gfx_modify_Texture_file_number_series);
					/* raw image storage mode */
					raw_image_storage_string =
						ENUMERATOR_STRING(Raw_image_storage)(RAW_PLANAR_RGB);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Raw_image_storage)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Raw_image_storage) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table, number_of_valid_strings,
						valid_strings, &raw_image_storage_string);
					DEALLOCATE(valid_strings);
					/* resize_linear_filter/resize_nearest_filter */
					resize_filter_mode_string =
						ENUMERATOR_STRING(Texture_resize_filter_mode)(
							Texture_get_resize_filter_mode(texture));
					valid_strings =
						ENUMERATOR_GET_VALID_STRINGS(Texture_resize_filter_mode)(
							&number_of_valid_strings, (ENUMERATOR_CONDITIONAL_FUNCTION(
								Texture_resize_filter_mode) *)NULL, (void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&resize_filter_mode_string);
					DEALLOCATE(valid_strings);
					/* specify_depth */
					Option_table_add_entry(option_table, "specify_depth",&specify_depth,
					  NULL,set_int_non_negative);
					/* specify_format */
					Option_table_add_entry(option_table, "specify_format", &specify_format,
						NULL, set_Texture_storage);
					/* specify_height */
					Option_table_add_entry(option_table, "specify_height",&specify_height,
					  NULL,set_int_non_negative);
					/* specify_number_of_bytes_per_component */
					Option_table_add_entry(option_table,
						"specify_number_of_bytes_per_component",
						&specify_number_of_bytes_per_component,NULL,set_int_non_negative);
					/* specify_width */
					Option_table_add_entry(option_table, "specify_width",&specify_width,
					  NULL,set_int_non_negative);
					/* width */
					Option_table_add_entry(option_table, "width", &width,
					  NULL,set_float_non_negative);
					/* evaluate_image */
					Option_table_add_entry(option_table, "evaluate_image",
					  &evaluate_data, command_data, gfx_modify_Texture_evaluate_image);
					return_code=Option_table_multi_parse(option_table, state);
					if (return_code)
					{
						evaluate_data_region = (struct Cmiss_region *)NULL;

						if (evaluate_data.field || evaluate_data.region_path ||
							evaluate_data.spectrum || evaluate_data.texture_coordinates_field)
						{
							if (!(evaluate_data.field &&
								Cmiss_region_get_region_from_path(command_data->root_region,
									evaluate_data.region_path, &evaluate_data_region) &&
								evaluate_data.spectrum &&
								evaluate_data.texture_coordinates_field))
							{
								display_message(ERROR_MESSAGE,
									"To evaluate the texture image from a field you must specify\n"
									"a field, element_group, spectrum and texture_coordinates");
								return_code = 0;
							}
						}
					}
					if (return_code)
					{
						if (texture_is_managed)
						{
							MANAGER_BEGIN_CHANGE(Texture)(command_data->texture_manager,
								MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Texture), texture);
						}
						/* must change filter modes etc. before reading new images since
							 some of them will apply immediately to the new images */
						Texture_set_combine_alpha(texture, alpha);
						Texture_set_combine_colour(texture, &colour);
						Texture_set_physical_size(texture, width,
							height, depth);

						STRING_TO_ENUMERATOR(Texture_combine_mode)(
							combine_mode_string, &combine_mode);
						Texture_set_combine_mode(texture, combine_mode);

						STRING_TO_ENUMERATOR(Texture_compression_mode)(
							compression_mode_string, &compression_mode);
						Texture_set_compression_mode(texture, compression_mode);

						STRING_TO_ENUMERATOR(Texture_filter_mode)(
							filter_mode_string, &filter_mode);
						Texture_set_filter_mode(texture, filter_mode);

						STRING_TO_ENUMERATOR(Texture_resize_filter_mode)(
							resize_filter_mode_string, &resize_filter_mode);
						Texture_set_resize_filter_mode(texture,
							resize_filter_mode);

						STRING_TO_ENUMERATOR(Texture_wrap_mode)(
							wrap_mode_string, &wrap_mode);
						Texture_set_wrap_mode(texture, wrap_mode);

						if (texture_distortion_data.set)
						{
							distortion_centre_x=(float)texture_distortion[0];
							distortion_centre_y=(float)texture_distortion[1];
							distortion_factor_k1=(float)texture_distortion[2];
							Texture_set_distortion_info(texture,
								distortion_centre_x,distortion_centre_y,distortion_factor_k1);
						}

						if (image_data.image_file_name)
						{
							cmgui_image_information = CREATE(Cmgui_image_information)();
							/* specify file name(s) */
							if (0 != file_number_series_data.increment)
							{
								if (strstr(image_data.image_file_name, file_number_pattern))
								{
									Cmgui_image_information_set_file_name_series(
										cmgui_image_information,
										/*file_name_template*/image_data.image_file_name,
										file_number_pattern,
										file_number_series_data.start,
										file_number_series_data.start,
										/*increment*/1);
								}
								else
								{
									display_message(ERROR_MESSAGE, "gfx modify texture:  "
										"File number pattern \"%s\" not found in file name \"%s\"",
										file_number_pattern, image_data.image_file_name);
									return_code = 0;
								}
							}
							else
							{
								Cmgui_image_information_add_file_name(cmgui_image_information,
									image_data.image_file_name);
							}
							/* specify width and height and raw_image_storage */
							Cmgui_image_information_set_width(cmgui_image_information,
								specify_width);
							Cmgui_image_information_set_height(cmgui_image_information,
								specify_height);
							Cmgui_image_information_set_io_stream_package(cmgui_image_information,
								command_data->io_stream_package);
							STRING_TO_ENUMERATOR(Raw_image_storage)(
								raw_image_storage_string, &raw_image_storage);
							Cmgui_image_information_set_raw_image_storage(
								cmgui_image_information, raw_image_storage);
							switch (specify_format)
							{
								case TEXTURE_LUMINANCE:
								{
									Cmgui_image_information_set_number_of_components(
										cmgui_image_information, 1);
								} break;
								case TEXTURE_LUMINANCE_ALPHA:
								{
									Cmgui_image_information_set_number_of_components(
										cmgui_image_information, 2);
								} break;
								case TEXTURE_RGB:
								{
									Cmgui_image_information_set_number_of_components(
										cmgui_image_information, 3);
								} break;
								case TEXTURE_RGBA:
								{
									Cmgui_image_information_set_number_of_components(
										cmgui_image_information, 4);
								} break;
								case TEXTURE_ABGR:
								{
									Cmgui_image_information_set_number_of_components(
										cmgui_image_information, 4);
								} break;
							}
							if (specify_number_of_bytes_per_component)
							{
								Cmgui_image_information_set_number_of_bytes_per_component(
									cmgui_image_information, specify_number_of_bytes_per_component);
							}
							if (return_code)
							{
								if (cmgui_image = Cmgui_image_read(cmgui_image_information))
								{
									return_code = Texture_set_image(texture, cmgui_image,
										image_data.image_file_name, file_number_pattern,
										file_number_series_data.start,
										file_number_series_data.stop,
										file_number_series_data.increment,
										image_data.crop_left_margin, image_data.crop_bottom_margin,
										image_data.crop_width, image_data.crop_height);
									DESTROY(Cmgui_image)(&cmgui_image);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"gfx modify texture:  Could not read image file");
									return_code = 0;
								}
								if (return_code && (0 != file_number_series_data.increment))
								{
									number_of_file_names = 1 + (file_number_series_data.stop -
										file_number_series_data.start) /
										file_number_series_data.increment;
									file_number = file_number_series_data.start + 
										file_number_series_data.increment;
									for (i = 1 ; return_code && (i < number_of_file_names) ; i++)
									{
										Cmgui_image_information_set_file_name_series(
											cmgui_image_information,
											/*file_name_template*/image_data.image_file_name,
											file_number_pattern, /*start*/file_number,
											/*end*/file_number, /*increment*/1);
										if (cmgui_image = Cmgui_image_read(cmgui_image_information))
										{
											return_code = Texture_add_image(texture, cmgui_image,
												image_data.crop_left_margin, image_data.crop_bottom_margin,
												image_data.crop_width, image_data.crop_height);
											DESTROY(Cmgui_image)(&cmgui_image);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"gfx modify texture:  Could not read image file");
											return_code = 0;
										}
										file_number += file_number_series_data.increment;
									}
								}
								if (! return_code)
								{
									/* Set a NULL image into texture so that an incomplete set isn't displayed */
									Texture_allocate_image(texture, /*image_width*/1, /*image_height*/1,
										/*image_depth*/1, TEXTURE_RGB, /*number_of_bytes_per_component*/1,
										"INCOMPLETEDTEXTURE");
									display_message(ERROR_MESSAGE,  "gfx modify texture:  "
										"Unable to read images into texture, setting it to black.");
								}
							}
							DESTROY(Cmgui_image_information)(&cmgui_image_information);
						}
#if defined (SGI_MOVIE_FILE)
						if ( movie != old_movie )
						{
							/* Movie is outside manager copy so that is updates
								the correct texture based on movie events */
							Texture_set_movie(texture,
								Movie_graphics_get_X3d_movie(movie),
								command_data->graphics_buffer_package, "movie");
						}
#endif /* defined (SGI_MOVIE_FILE) */

						if (evaluate_data_region &&
							evaluate_data.field && evaluate_data.spectrum && 
							evaluate_data.texture_coordinates_field)
						{
							if (Computed_field_depends_on_texture(evaluate_data.field,
									texture))
							{
								texture_copy = CREATE(Texture)("temporary");
								MANAGER_COPY_WITHOUT_IDENTIFIER(Texture,name)
									(texture_copy, texture);
							}
							else
							{
								texture_copy = texture;
							}
							
							set_Texture_image_from_field(texture_copy, 
								evaluate_data.field,
								evaluate_data.texture_coordinates_field,
								evaluate_data.propagate_field, 
								evaluate_data.spectrum, evaluate_data_region,
								evaluate_data.element_dimension,
								specify_format, specify_width, 
								specify_height, specify_depth,
								command_data->graphics_buffer_package,
								evaluate_data.fail_material);

							if (texture_copy != texture)
							{
								MANAGER_MODIFY_NOT_IDENTIFIER(Texture,name)(texture,
									texture_copy,command_data->texture_manager);
								DESTROY(Texture)(&texture_copy);
							}
						}
						if (texture_is_managed)
						{
							MANAGER_END_CHANGE(Texture)(command_data->texture_manager);
						}
					}
					if (image_data.image_file_name)
					{
						DEALLOCATE(image_data.image_file_name);
					}
					DESTROY(Option_table)(&option_table);
#if defined (SGI_MOVIE_FILE)
					if (movie)
					{
						DEACCESS(Movie_graphics)(&movie);
					}
#endif /* defined (SGI_MOVIE_FILE) */
					if (evaluate_data.region_path)
					{
						DEALLOCATE(evaluate_data.region_path);
					}
					if (evaluate_data.fail_material)
					{
						DEACCESS(Graphical_material)(&evaluate_data.fail_material);
					}
					if (evaluate_data.spectrum)
					{
						DEACCESS(Spectrum)(&evaluate_data.spectrum);
					}
					if (evaluate_data.field)
					{
						DEACCESS(Computed_field)(&evaluate_data.field);
					}
					if (evaluate_data.texture_coordinates_field)
					{
						DEACCESS(Computed_field)(&evaluate_data.texture_coordinates_field);
					}
					DEALLOCATE(file_number_pattern);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_Texture.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			if (texture_void)
			{
				display_message(WARNING_MESSAGE,"Missing texture modifications");
			}
			else
			{
				display_message(WARNING_MESSAGE,"Missing texture name");
			}
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_Texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Texture */

static int gfx_create_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE TEXTURE command.
==============================================================================*/ 
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Texture *texture;

	ENTER(gfx_create_texture);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (command_data->texture_manager)
					{
						if (!FIND_BY_IDENTIFIER_IN_MANAGER(Texture,name)(current_token,
							command_data->texture_manager))
						{
							if (texture=CREATE(Texture)(current_token))
							{
								shift_Parse_state(state,1);
								if (state->current_token)
								{
									return_code=gfx_modify_Texture(state,(void *)texture,
										command_data_void);
								}
								else
								{
									return_code=1;
								}
								if (return_code)
								{
									ADD_OBJECT_TO_MANAGER(Texture)(texture,
										command_data->texture_manager);
								}
								else
								{
									DESTROY(Texture)(&texture);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_create_texture.  Error creating texture");
								return_code=0;
							}
						}
						else
						{
							display_message(WARNING_MESSAGE,"Texture '%s' already exists",
								current_token);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_texture.  Missing texture_manager");
						return_code=0;
					}
				}
				else
				{
					return_code=gfx_modify_Texture(state,(void *)NULL,
						command_data_void);
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_texture.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing texture name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /*gfx_create_texture */

#if defined (MOTIF)
static int gfx_create_time_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 15 June 1999

DESCRIPTION :
Executes a GFX CREATE TIME_EDITOR command.
If there is a time editor dialog in existence, then bring it to the front,
otherwise it creates a new one.  Assumes we will only ever want one time
editor at a time.  This implementation may be changed later.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_graphical_time_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				return_code=bring_up_time_editor_dialog(
					&(command_data->time_editor_dialog),
					User_interface_get_application_shell(command_data->user_interface),
					command_data->default_time_keeper, 
					command_data->user_interface);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_graphical_time_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_graphical_time_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_graphical_time_editor */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static int gfx_create_curve_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Executes a GFX CREATE CURVE_EDITOR command.
If there is a variable editor dialog in existence, then bring it to the front,
otherwise it creates a new one.  Assumes we will only ever want one variable
editor at a time.  This implementation may be changed later.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_create_curve_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				return_code=bring_up_curve_editor_dialog(
					&(command_data->curve_editor_dialog),
					User_interface_get_application_shell(command_data->user_interface),
					command_data->curve_manager,
					(struct Curve *)NULL,
					command_data->user_interface);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_curve_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_curve_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_curve_editor */
#endif /* defined (MOTIF) */

#if defined (USE_CMGUI_GRAPHICS_WINDOW)
static int gfx_create_window(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX CREATE WINDOW command.
==============================================================================*/
{
	char any_buffering_mode_flag, any_stereo_mode_flag, double_buffer_flag,
		*name,mono_buffer_flag,single_buffer_flag,stereo_buffer_flag;
	enum Graphics_window_buffering_mode buffer_mode;
	enum Graphics_window_stereo_mode stereo_mode;
	int minimum_colour_buffer_depth, minimum_depth_buffer_depth,
		minimum_accumulation_buffer_depth, return_code;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	struct Option_table *buffer_option_table, *option_table, *stereo_option_table

	ENTER(gfx_create_window);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			return_code=1;
			/* set_defaults */
			name=Graphics_window_manager_get_new_name(
				command_data->graphics_window_manager);
			buffer_mode = GRAPHICS_WINDOW_DOUBLE_BUFFERING;
			stereo_mode = GRAPHICS_WINDOW_ANY_STEREO_MODE;
			minimum_depth_buffer_depth=8;
			minimum_accumulation_buffer_depth=8;
			minimum_colour_buffer_depth = 8;
			if (state->current_token)
			{
				/* change defaults */
				any_buffering_mode_flag=0;
				single_buffer_flag=0;
				double_buffer_flag=0;
				any_stereo_mode_flag=0;
				mono_buffer_flag=0;
				stereo_buffer_flag=0;

				option_table = CREATE(Option_table)();
				/* accumulation_buffer_depth */
				Option_table_add_entry(option_table, "accumulation_buffer_depth",
					&minimum_accumulation_buffer_depth, NULL, set_int_non_negative);
				/* any_buffer_mode/double_buffer/single_buffer */
				buffer_option_table=CREATE(Option_table)();
				Option_table_add_entry(buffer_option_table,"any_buffer_mode",
					&any_buffering_mode_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(buffer_option_table,"double_buffer",
					&double_buffer_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(buffer_option_table,"single_buffer",
					&single_buffer_flag,(void *)NULL,set_char_flag);
				Option_table_add_suboption_table(option_table, buffer_option_table);
				/* any_stereo_mode/mono_buffer/stereo_buffer */
				stereo_option_table=CREATE(Option_table)();
				Option_table_add_entry(stereo_option_table,"any_stereo_mode",
					&any_stereo_mode_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(stereo_option_table,"mono_buffer",
					&mono_buffer_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(stereo_option_table,"stereo_buffer",
					&stereo_buffer_flag,(void *)NULL,set_char_flag);
				Option_table_add_suboption_table(option_table, stereo_option_table);
				/* colour_buffer_depth */
				Option_table_add_entry(option_table, "colour_buffer_depth",
					&minimum_colour_buffer_depth, NULL, set_int_non_negative);
				/* depth_buffer_depth */
				Option_table_add_entry(option_table, "depth_buffer_depth",
					&minimum_depth_buffer_depth, NULL, set_int_non_negative);
				/* name */
				Option_table_add_entry(option_table,"name",&name,(void *)1,set_name);
				/* default */
				Option_table_add_entry(option_table,(char *)NULL,&name,(void *)NULL,
					set_name);
				return_code = Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
				if (return_code)
				{
					if (any_buffering_mode_flag + single_buffer_flag + double_buffer_flag > 1)
					{
						display_message(ERROR_MESSAGE,
							"Only one of any_buffer_mode/single_buffer/double_buffer");
						return_code=0;
					}
					if (any_stereo_mode_flag + mono_buffer_flag + stereo_buffer_flag > 1)
					{
						display_message(ERROR_MESSAGE,
							"Only one of any_stereo_mode/mono_buffer/stereo_buffer");
						return_code=0;
					}
				}
				if (return_code)
				{
					if (any_buffering_mode_flag)
					{
						buffer_mode = GRAPHICS_WINDOW_ANY_BUFFERING_MODE;
					}
					else if (single_buffer_flag)
					{
						buffer_mode = GRAPHICS_WINDOW_SINGLE_BUFFERING;
					}
					else if (double_buffer_flag)
					{
						buffer_mode = GRAPHICS_WINDOW_DOUBLE_BUFFERING;
					}
					if (any_stereo_mode_flag)
					{
						stereo_mode = GRAPHICS_WINDOW_ANY_STEREO_MODE;
					}
					else if (stereo_buffer_flag)
					{
						stereo_mode = GRAPHICS_WINDOW_STEREO;
					}
					else if (mono_buffer_flag)
					{
						stereo_mode = GRAPHICS_WINDOW_MONO;
					}
				}
			}
			if (!name)
			{
				display_message(ERROR_MESSAGE,"gfx_create_window.  Missing name");
				return_code=0;
			}
			if (return_code)
			{
				if (window=FIND_BY_IDENTIFIER_IN_MANAGER(Graphics_window,name)(name,
					command_data->graphics_window_manager))
				{
					display_message(WARNING_MESSAGE,
						"Graphics window '%s' already exists",name);
					return_code=0;
				}
				else
				{
				   if (command_data->user_interface)
					{
#if defined (WX_USER_INTERFACE)
						struct MANAGER(Interactive_tool) *interactive_tool_manager;
						struct Interactive_tool *transform_tool;
						interactive_tool_manager = CREATE(MANAGER(Interactive_tool))();
					    transform_tool=create_Interactive_tool_transform(
				             command_data->user_interface);
						ADD_OBJECT_TO_MANAGER(Interactive_tool)(transform_tool,
							 interactive_tool_manager);
						CREATE(Node_tool)(
								interactive_tool_manager,
								command_data->root_region, /*use_data*/0,
								command_data->node_selection,
								command_data->computed_field_package,
								Material_package_get_default_material(command_data->material_package),
								command_data->user_interface,
								command_data->default_time_keeper,
								command_data->execute_command);
						CREATE(Node_tool)(
								interactive_tool_manager,
								command_data->data_root_region, /*use_data*/1,
								command_data->data_selection,
								command_data->computed_field_package,
								Material_package_get_default_material(command_data->material_package),
								command_data->user_interface,
								command_data->default_time_keeper,
								command_data->execute_command);
						CREATE(Element_tool)(
								interactive_tool_manager,
								command_data->root_region,
								command_data->element_selection,
								command_data->element_point_ranges_selection,
								command_data->computed_field_package,
								Material_package_get_default_material(command_data->material_package),
								command_data->user_interface,
								command_data->default_time_keeper,
								command_data->execute_command);
						CREATE(Element_point_tool)(
								interactive_tool_manager,
								command_data->element_point_ranges_selection,
								command_data->computed_field_package,
								Material_package_get_default_material(command_data->material_package),
								command_data->user_interface,
								command_data->default_time_keeper,
								command_data->execute_command);
						if (window=CREATE(Graphics_window)(name,buffer_mode,stereo_mode,
							minimum_colour_buffer_depth, minimum_depth_buffer_depth,
							minimum_accumulation_buffer_depth,
							command_data->graphics_buffer_package,
							&(command_data->background_colour),
							command_data->light_manager,command_data->default_light,
							command_data->light_model_manager,command_data->default_light_model,
							command_data->scene_manager,command_data->default_scene,
							command_data->texture_manager,
							interactive_tool_manager,
							command_data->user_interface))
						{
							if (!ADD_OBJECT_TO_MANAGER(Graphics_window)(window,
							   command_data->graphics_window_manager))
							{
							   DESTROY(Graphics_window)(&window);
								 DESTROY(MANAGER(Interactive_tool))(&interactive_tool_manager);
 							   return_code=0;
							}
						}
						else
					   {
						  display_message(ERROR_MESSAGE,
							 "gfx_create_window.  Could not create graphics window");
						  return_code=0;
						}
#else
					   if (window=CREATE(Graphics_window)(name,buffer_mode,stereo_mode,
							minimum_colour_buffer_depth, minimum_depth_buffer_depth,
							minimum_accumulation_buffer_depth,
							command_data->graphics_buffer_package,
							&(command_data->background_colour),
							command_data->light_manager,command_data->default_light,
							command_data->light_model_manager,command_data->default_light_model,
							command_data->scene_manager,command_data->default_scene,
							command_data->texture_manager,
							command_data->interactive_tool_manager,
							command_data->user_interface))
						{
						   if (!ADD_OBJECT_TO_MANAGER(Graphics_window)(window,
							   command_data->graphics_window_manager))
							{
							   DESTROY(Graphics_window)(&window);
							   return_code=0;
							}
						}
						else
					   {
						  display_message(ERROR_MESSAGE,
							 "gfx_create_window.  Could not create graphics window");
						  return_code=0;
						}
#endif /*(WX_USER_INTERFACE)*/
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_window.  Cannot create a graphics window without a display.");
						return_code=0;
					}
				}
			}
			if (name)
			{
				DEALLOCATE(name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_window.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_window.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_window */
#endif /* defined (MOTIF) || defined (GTK_USER_INTERFACE)  || defined (WIN32_USER_INTERFACE) || defined (WX_USER_INTERFACE) */

#if defined (HAPTIC)
static int gfx_create_haptic(struct Parse_state *state,
	void *dummy__to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 12 May 1998

DESCRIPTION :
Executes a GFX CREATE HAPTIC command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	float dynamic_friction, static_friction, damping,
		spring_k;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	static struct Modifier_entry
		option_table[]=
		{
			{"scene",NULL,NULL,set_Scene},
			{"dynamic_surface_friction",NULL,NULL,set_float_0_to_1_inclusive},
			{"static_surface_friction",NULL,NULL,set_float_0_to_1_inclusive},
			{"damping_surface",NULL,NULL,set_float_0_to_1_inclusive},
			{"spring_k_surface",NULL,NULL,set_float /* SAB set_float_0_to_1_inclusive */},
			{NULL,NULL,NULL,NULL}
		};

	ENTER(gfx_create_haptic);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			return_code=1;
			/* set_defaults */
			scene = command_data->default_scene;
			dynamic_friction = 0.2;
			static_friction = 0.35;
			damping = 0.0;
			spring_k = 1.0;
			if (current_token=state->current_token)
			{
				(option_table[0]).to_be_modified= &scene;
				(option_table[0]).user_data=command_data->scene_manager;
				(option_table[1]).to_be_modified= &dynamic_friction;
				(option_table[2]).to_be_modified= &static_friction;
				(option_table[3]).to_be_modified= &damping;
				(option_table[4]).to_be_modified= &spring_k;
				if (return_code=process_multiple_options(state,option_table))
				{
					/* SAB Damping is really in Kg/1000.0*sec and ranges from
						0 to 0.005, so the parameter is scaled here */
					damping *= 0.005;
					haptic_set_surface_defaults( dynamic_friction, static_friction,
						damping, spring_k );
				}
			}
			if ( return_code )
			{
				haptic_create_scene( scene );
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_haptic.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_haptic.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_haptic */
#endif /* defined (HAPTIC) */

static int gfx_create_cmiss(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 9 October 2001

DESCRIPTION :
Executes a GFX CREATE CMISS_CONNECTION command.
==============================================================================*/
{
	char asynchronous_commands,*examples_directory,*host_name,mycm_flag,
		*parameters_file_name;
	enum Machine_type host_type;
	int connection_number,return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
#if defined (LINK_CMISS)
/*???DB.  Not sure if this is quite the right place */
	double wormhole_timeout;
#if defined (MOTIF)
#define XmNwormholeTimeoutSeconds "wormholeTimeoutSeconds"
#define XmCWormholeTimeoutSeconds "WormholeTimeoutSeconds"
	static XtResource resources[]=
	{
		{
			XmNwormholeTimeoutSeconds,
			XmCWormholeTimeoutSeconds,
			XmRInt,
			sizeof(int),
			0,
			XmRString,
			"300"
		}
	};
	int wormhole_timeout_seconds;
#endif /* defined (MOTIF) */
#endif /* defined (LINK_CMISS) */

	ENTER(gfx_create_cmiss);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			if (command_data->cm_examples_directory)
			{
				if (ALLOCATE(examples_directory,char,
					strlen(command_data->cm_examples_directory)+1))
				{
					strcpy(examples_directory,command_data->cm_examples_directory);
				}
				else
				{
					examples_directory=(char *)NULL;
				}
			}
			else
			{
				examples_directory=(char *)NULL;
			}
			if (command_data->cm_parameters_file_name)
			{
				if (ALLOCATE(parameters_file_name,char,
					strlen(command_data->cm_parameters_file_name)+1))
				{
					strcpy(parameters_file_name,command_data->cm_parameters_file_name);
				}
				else
				{
					parameters_file_name=(char *)NULL;
				}
			}
			else
			{
				parameters_file_name=(char *)NULL;
			}
			if ((!command_data->user_interface) ||
				(!User_interface_get_local_machine_name(command_data->user_interface, &host_name)))
			{
				host_name=(char *)NULL;
			}
			connection_number=0;
			host_type=MACHINE_UNKNOWN;
			mycm_flag=0;
			asynchronous_commands=0;
#if defined (LINK_CMISS)
			wormhole_timeout=300;
#if defined (MOTIF)
			if (command_data->user_interface)
			{
				XtVaGetApplicationResources(User_interface_get_application_shell(
					command_data->user_interface),&wormhole_timeout_seconds,resources,
					XtNumber(resources),NULL);
				wormhole_timeout=(double)wormhole_timeout_seconds;
			}
#endif /* defined (MOTIF) */
#endif /* defined (LINK_CMISS) */
			option_table=CREATE(Option_table)();
			Option_table_add_entry(option_table,"asynchronous_commands",
				&asynchronous_commands,(void *)NULL,set_char_flag);
			Option_table_add_entry(option_table,"connection_number",
				&connection_number,(void *)NULL,set_int_non_negative);
			Option_table_add_entry(option_table,"examples_directory",
				&examples_directory,(void *)1,set_name);
			Option_table_add_entry(option_table,"host",&host_name,(void *)1,set_name);
			Option_table_add_entry(option_table,"mycm",&mycm_flag,(void *)NULL,
				set_char_flag);
			Option_table_add_entry(option_table,"parameters",&parameters_file_name,
				(void *)1,set_name);
			Option_table_add_entry(option_table,"type",&host_type,(void *)NULL,
				set_machine_type);
			Option_table_add_entry(option_table,(char *)NULL,&host_name,(void *)NULL,
				set_name);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors, not asking for help */
			if (return_code)
			{
#if defined (LINK_CMISS)
				if (!CMISS)
				{
					if (MACHINE_UNKNOWN==host_type)
					{
						/* have a stab at identifying it */
						if (fuzzy_string_compare(host_name,"esu"))
						{
							host_type=MACHINE_UNIX;
						}
						else
						{
							if (fuzzy_string_compare(host_name,"esv"))
							{
								host_type=MACHINE_VMS;
							}
						}
					}
					if (CMISS=CREATE(CMISS_connection)(host_name, host_type,
						connection_number, wormhole_timeout, mycm_flag,
						asynchronous_commands, command_data->root_region,
						command_data->data_root_region,
						&(command_data->prompt_window),
						parameters_file_name,examples_directory,
						command_data->user_interface))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_cmiss.  Could not create CMISS connection");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_cmiss.  CMISS connection already exists");
					return_code=0;
				}
#else /* defined (LINK_CMISS) */
			display_message(ERROR_MESSAGE,"gfx_create_cmiss.  Define LINK_CMISS");
			return_code=0;
#endif /* defined (LINK_CMISS) */
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			DEALLOCATE(examples_directory);
			DEALLOCATE(host_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_create_cmiss.  Missing command data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_cmiss.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_cmiss */

#if defined (SELECT_DESCRIPTORS)
static int execute_command_attach(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 11 May 2001

DESCRIPTION :
Executes an ATTACH command.
==============================================================================*/
{
	char *current_token, end_detection, *perl_action, start_detection;
	int return_code;
	struct Io_device *device;
	static struct Option_table *option_table;
	struct Cmiss_command_data *command_data;
	
	ENTER(execute_command_attach);
	USE_PARAMETER(prompt_void);
	/* check argument */
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		device = (struct Io_device *)NULL;
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (command_data->device_list)
				{
					if (device=FIND_BY_IDENTIFIER_IN_LIST(Io_device, name)
						(current_token,command_data->device_list))
					{
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						if (device = CREATE(Io_device)(current_token))
						{
							if (ADD_OBJECT_TO_LIST(Io_device)(device, command_data->device_list))
							{
								return_code=shift_Parse_state(state,1);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"execute_command_attach.  Unable to create device struture.");
							return_code=0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_attach.  Missing device list");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" DEVICE_NAME");
				return_code = 1;
				/* By not shifting the parse state the rest of the help should come out */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_attach.  Missing device name");
			return_code=0;
		}
		if (return_code)
		{
			end_detection = 0;
			perl_action = (char *)NULL;
			start_detection = 0;
			
			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table,"end_detection", &end_detection, 
				NULL, set_char_flag);
			Option_table_add_entry(option_table,"perl_action", &perl_action, (void *)1,
				set_name);
			Option_table_add_entry(option_table,"start_detection", &start_detection,
				NULL, set_char_flag);
			return_code = Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				if (start_detection && end_detection)
				{
					display_message(ERROR_MESSAGE,"execute_command_attach.  "
						"Specify only one of start_detection and end_detection.");
					return_code=0;
				}
			}
			if (return_code)
			{
				if (start_detection)
				{
					Io_device_start_detection(device, command_data->user_interface);
				}
				if (end_detection)
				{
					Io_device_end_detection(device);
				}
				if (perl_action)
				{
					Io_device_set_perl_action(device, command_data->interpreter,
						perl_action);
				}
			}
			if (perl_action)
			{
				DEALLOCATE(perl_action);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_attach.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_attach */

static int execute_command_detach(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 11 May 2001

DESCRIPTION :
Executes a DETACH command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Io_device *device;
	struct Cmiss_command_data *command_data;
	
	ENTER(execute_command_detach);
	USE_PARAMETER(prompt_void);
	/* check argument */
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		device = (struct Io_device *)NULL;
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (command_data->device_list)
				{
					if (device=FIND_BY_IDENTIFIER_IN_LIST(Io_device, name)
						(current_token,command_data->device_list))
					{
						if (REMOVE_OBJECT_FROM_LIST(Io_device)(device,
							command_data->device_list))
						{
							if (DESTROY(Io_device)(&device))
							{
								return_code = 1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"execute_command_detach.  Unable to destroy device %s.",
									current_token);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"execute_command_detach.  Unable to remove device %s.",
								current_token);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"execute_command_detach.  Io_device %s not found.", current_token);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_detach.  Missing device list");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" DEVICE_NAME");
				return_code = 1;
				/* By not shifting the parse state the rest of the help should come out */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_detach.  Missing device name");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_detach.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_detach */
#endif /* defined (SELECT_DESCRIPTORS) */

static int gfx_convert_elements(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 April 2006

DESCRIPTION :
Executes a GFX CONVERT ELEMENETS command.
==============================================================================*/
{
	char *destination_region_path, *source_region_path;
	enum Convert_finite_elements_mode conversion_mode;
	int i, number_of_fields, previous_state_index, return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct Computed_field **fields;
	struct FE_region *destination_fe_region, *source_fe_region;
	struct Set_Computed_field_conditional_data set_field_data;
	struct Set_Computed_field_array_data set_field_array_data;
	struct Option_table *option_table;

	ENTER(gfx_convert_elements);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			Cmiss_region_get_root_region_path(&source_region_path);
			Cmiss_region_get_root_region_path(&destination_region_path);
			fields = (struct Computed_field **)NULL;
			number_of_fields = 1;
			conversion_mode = CONVERT_TO_FINITE_ELEMENTS_HERMITE_2D_PRODUCT;
			
			if (strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
			{
				/* Skip this preprocessing if we are just getting the help */
				number_of_fields = 1;
				previous_state_index = state->current_index;
				
				option_table = CREATE(Option_table)();
				/* number_of_fields */
				Option_table_add_entry(option_table, "number_of_fields",
					&number_of_fields, NULL, set_int_positive);
				/* absorb everything else */
				Option_table_ignore_all_unmatched_entries(option_table);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
				/* Return back to where we were */
				shift_Parse_state(state, previous_state_index - state->current_index);
			}
			
			if (number_of_fields)
			{
				ALLOCATE(fields, struct Computed_field *, number_of_fields);
				for (i = 0; i < number_of_fields; i++)
				{
					fields[i] = (struct Computed_field *)NULL;
				}
			}
			else
			{
				fields = (struct Computed_field **)NULL;
			}
		
			option_table=CREATE(Option_table)();
			/* destination_region */
			Option_table_add_entry(option_table, "destination_region", &destination_region_path,
				command_data->root_region, set_Cmiss_region_path);
			/* fields */
			set_field_data.conditional_function =
				Computed_field_has_numerical_components;
			set_field_data.conditional_function_user_data = (void *)NULL;
			set_field_data.computed_field_manager =
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_field_array_data.number_of_fields = number_of_fields;
			set_field_array_data.conditional_data = &set_field_data;
			Option_table_add_entry(option_table, "fields", fields,
				&set_field_array_data, set_Computed_field_array);
			/* number_of_fields */
			Option_table_add_entry(option_table, "number_of_fields",
				&number_of_fields, NULL, set_int_positive);
			/* conversion_mode */
			OPTION_TABLE_ADD_ENUMERATOR(Convert_finite_elements_mode)(option_table,
				&conversion_mode);
			/* source_region */
			Option_table_add_entry(option_table, "source_region", &source_region_path,
				command_data->root_region, set_Cmiss_region_path);
			
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);

			if (return_code)
			{
				destination_fe_region = (struct FE_region *)NULL;
				if (!(Cmiss_region_get_region_from_path(command_data->root_region,
					destination_region_path, &region) &&
					(destination_fe_region = Cmiss_region_get_FE_region(region))))
				{
					display_message(ERROR_MESSAGE,
						"gfx_convert.  Invalid destination_region");
					return_code = 0;
				}
				source_fe_region = (struct FE_region *)NULL;
				if (!(Cmiss_region_get_region_from_path(command_data->root_region,
					source_region_path, &region) &&
					(source_fe_region = Cmiss_region_get_FE_region(region))))
				{
					display_message(ERROR_MESSAGE,
						"gfx_convert.  Invalid destination_region");
					return_code = 0;
				}
			}

			if (return_code)
			{
				return_code = finite_element_conversion(source_fe_region,
					destination_fe_region, conversion_mode,
					number_of_fields, fields);
			}
			if (fields)
			{				
				for (i = 0; i < number_of_fields; i++)
				{
					if (fields[i])
					{
						DEACCESS(Computed_field)(&fields[i]);
					}
				}
				DEALLOCATE(fields);
			}
			if (destination_region_path)
			{
				DEALLOCATE(destination_region_path);
			}
			if (source_region_path)
			{
				DEALLOCATE(source_region_path);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_convert_elements.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_convert_elements.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_convert_elements */

static int gfx_convert_graphics(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 April 2006

DESCRIPTION :
Executes a GFX CREATE command.
==============================================================================*/
{
	char *region_path;
	enum Render_to_finite_elements_mode render_mode;
	int return_code;
	struct Computed_field *coordinate_field;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct FE_region *fe_region;
	struct Scene *scene;
	struct Set_Computed_field_conditional_data set_coordinate_field_data;
	struct Option_table *option_table;

	ENTER(gfx_convert_graphics);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			Cmiss_region_get_root_region_path(&region_path);
			coordinate_field=(struct Computed_field *)NULL;
			scene = (struct Scene *)NULL;
			render_mode = RENDER_TO_FINITE_ELEMENTS_LINEAR_PRODUCT;

			option_table=CREATE(Option_table)();
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					command_data->computed_field_package);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				&set_coordinate_field_data,set_Computed_field_conditional);
			/* render_to_finite_elements_mode */
			OPTION_TABLE_ADD_ENUMERATOR(Render_to_finite_elements_mode)(option_table,
				&render_mode);
			/* region */
			Option_table_add_entry(option_table, "region", &region_path,
				command_data->root_region, set_Cmiss_region_path);
			/* scene */
			Option_table_add_entry(option_table, "scene", &scene,
				command_data->scene_manager, set_Scene_including_sub_objects);
			
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);

			if (return_code)
			{
				if (!scene)
				{
					display_message(ERROR_MESSAGE,
						"gfx_convert.  Must specify a scene.");
					return_code = 0;
				}
				if (!coordinate_field)
				{
					display_message(ERROR_MESSAGE,
						"gfx_convert_graphics.  "
						"Must specify a coordinate field to define on the new nodes and elements.");
					return_code = 0;
				}
				fe_region = (struct FE_region *)NULL;
				if (!(Cmiss_region_get_region_from_path(command_data->root_region,
					region_path, &region) &&
					(fe_region = Cmiss_region_get_FE_region(region))))
				{
					display_message(ERROR_MESSAGE,
						"gfx_convert.  Invalid region");
					return_code = 0;
				}
			}

			if (return_code)
			{
				 return_code = render_to_finite_elements(scene, fe_region,
					 render_mode, coordinate_field);
			}
			if (scene)
			{
				DEACCESS(Scene)(&scene);
			}
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (region_path)
			{
				DEALLOCATE(region_path);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_convert_graphics.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_convert_graphics.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_convert_graphics */

static int gfx_convert(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 April 2006

DESCRIPTION :
Executes a GFX CONVERT command.
==============================================================================*/
{
	int return_code;
	struct Option_table *option_table;

	ENTER(gfx_convert);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && command_data_void)
	{
		option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table,"elements",NULL,
			command_data_void, gfx_convert_elements);
		Option_table_add_entry(option_table,"graphics",NULL,
			command_data_void, gfx_convert_graphics);
		return_code = Option_table_parse(option_table,state);
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_convert.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_convert */

static int execute_command_gfx_create(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Executes a GFX CREATE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Create_emoter_slider_data create_emoter_slider_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_create);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				option_table=CREATE(Option_table)();
				Option_table_add_entry(option_table,"annotation",NULL,
					command_data_void,gfx_create_annotation);
				Option_table_add_entry(option_table,"axes",NULL,
					command_data_void,gfx_create_axes);
#if defined (MOTIF)
				Option_table_add_entry(option_table,"cmiss_connection",NULL,
					command_data_void,gfx_create_cmiss);
#endif /* defined (MOTIF) */
				Option_table_add_entry(option_table,"colour_bar",NULL,
					command_data_void,gfx_create_colour_bar);
#if defined (MOTIF)
				Option_table_add_entry(option_table,"curve_editor",NULL,
					command_data_void,gfx_create_curve_editor);
#endif /* defined (MOTIF) */
				Option_table_add_entry(option_table,"cylinders",NULL,
					command_data_void,gfx_create_cylinders);
#if defined (MOTIF) || (WX_USER_INTERFACE)
				Option_table_add_entry(option_table,"data_viewer",NULL,
					command_data_void,gfx_create_data_viewer);
#endif /* defined (MOTIF) || defined (WX_USER_INTERFACE) */
				Option_table_add_entry(option_table,"data_points",/*use_data*/(void *)1,
					command_data_void,gfx_create_node_points);
				Option_table_add_entry(option_table, "dgroup", /*use_nodes*/(void *)1,
					(void *)command_data->data_root_region, gfx_create_group);
				Option_table_add_entry(option_table, "egroup", /*use_nodes*/(void *)0,
					(void *)command_data->root_region, gfx_create_group);
#if defined (MOTIF) || defined (WX_USER_INTERFACE)
				Option_table_add_entry(option_table,"element_creator",NULL,
					command_data_void,gfx_create_element_creator);
#endif /* defined (MOTIF) || defined (WX_USER_INTERFACE) */
#if defined (MOTIF)  || defined (WX_USER_INTERFACE)
				Option_table_add_entry(option_table,"element_point_viewer",NULL,
					command_data_void,gfx_create_element_point_viewer);
#endif /* defined (MOTIF)  || defined (WX_USER_INTERFACE) */
				Option_table_add_entry(option_table,"element_points",NULL,
					command_data_void,gfx_create_element_points);
				Option_table_add_entry(option_table,"element_selection_callback",NULL,
					command_data_void,gfx_create_element_selection_callback);
				create_emoter_slider_data.execute_command=command_data->execute_command;
				create_emoter_slider_data.root_region=
					command_data->root_region;
				create_emoter_slider_data.basis_manager=
					command_data->basis_manager;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
				create_emoter_slider_data.graphics_window_manager=
					command_data->graphics_window_manager;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
				create_emoter_slider_data.graphics_buffer_package=
					command_data->graphics_buffer_package;
				create_emoter_slider_data.curve_manager=
					command_data->curve_manager;
				create_emoter_slider_data.scene_manager=command_data->scene_manager;
				create_emoter_slider_data.io_stream_package =
					command_data->io_stream_package;
				create_emoter_slider_data.viewer_scene=command_data->default_scene;
				create_emoter_slider_data.viewer_background_colour=
					command_data->background_colour;
				create_emoter_slider_data.viewer_light=command_data->default_light;
				create_emoter_slider_data.viewer_light_model=
					command_data->default_light_model;
				create_emoter_slider_data.emoter_dialog_address=
					&(command_data->emoter_slider_dialog);
#if defined (MOTIF)
				if (command_data->user_interface)
				{
					create_emoter_slider_data.parent=
						User_interface_get_application_shell(command_data->user_interface);
				}
				else
				{
					create_emoter_slider_data.parent=(Widget)NULL;
				}
				create_emoter_slider_data.curve_editor_dialog_address=
					&(command_data->curve_editor_dialog);
#endif /* defined (MOTIF) */
				create_emoter_slider_data.user_interface=
					command_data->user_interface;
				Option_table_add_entry(option_table,"emoter",NULL,
					(void *)&create_emoter_slider_data,gfx_create_emoter);
#if defined (MOTIF)
				Option_table_add_entry(option_table,"environment_map",NULL,
					command_data_void,gfx_create_environment_map);
#endif /* defined (MOTIF) */
				Option_table_add_entry(option_table, "flow_particles",
					/*create_more*/(void *)0, command_data_void, gfx_create_flow_particles);
#if defined (MOTIF)
				Option_table_add_entry(option_table,"graphical_material_editor",NULL,
					command_data_void,gfx_create_graphical_material_editor);
				Option_table_add_entry(option_table,"grid_field_calculator",NULL,
					command_data_void,gfx_create_grid_field_calculator);
#if defined (HAPTIC)
				Option_table_add_entry(option_table,"haptic",NULL,
					command_data_void,gfx_create_haptic);
#endif /* defined (HAPTIC) */
				Option_table_add_entry(option_table,"im_control",NULL,
					command_data_void,gfx_create_input_module_control);
#endif /* defined (MOTIF) */
				Option_table_add_entry(option_table,"iso_surfaces",NULL,
					command_data_void,gfx_create_iso_surfaces);
				Option_table_add_entry(option_table,"light",NULL,
					command_data_void,gfx_create_light);
				Option_table_add_entry(option_table,"lmodel",NULL,
					command_data_void,gfx_create_light_model);
				Option_table_add_entry(option_table,"lines",NULL,
					command_data_void,gfx_create_lines);
				Option_table_add_entry(option_table,"material",NULL,
					(void *)command_data->material_package,gfx_create_material);
				Option_table_add_entry(option_table, "more_flow_particles",
					/*create_more*/(void *)1, command_data_void, gfx_create_flow_particles);
				Option_table_add_entry(option_table,"morph",NULL,
					command_data_void,gfx_create_morph);
				Option_table_add_entry(option_table, "ngroup", /*use_nodes*/(void *)1,
					(void *)command_data->root_region, gfx_create_group);
				Option_table_add_entry(option_table,"node_points",/*use_data*/(void *)0,
					command_data_void,gfx_create_node_points);
				Option_table_add_entry(option_table,"node_selection_callback",NULL,
					command_data_void,gfx_create_node_selection_callback);
#if defined (MOTIF) || defined (WX_USER_INTERFACE)
				Option_table_add_entry(option_table,"node_viewer",NULL,
					command_data_void,gfx_create_node_viewer);
#endif /* defined (MOTIF) || defined (WX_USER_INTERFACE) */
				Option_table_add_entry(option_table, "region", NULL,
					(void *)command_data->root_region, gfx_create_region);
				Option_table_add_entry(option_table,"scene",NULL,
					command_data_void,gfx_create_scene);
				Option_table_add_entry(option_table, "snake", NULL,
					command_data_void, gfx_create_snake);
				Option_table_add_entry(option_table,"spectrum",NULL,
					command_data_void,gfx_create_spectrum);
				Option_table_add_entry(option_table,"streamlines",NULL,
					command_data_void,gfx_create_streamlines);
				Option_table_add_entry(option_table,"surfaces",NULL,
					command_data_void,gfx_create_surfaces);
				Option_table_add_entry(option_table,"texture",NULL, 
					command_data_void,gfx_create_texture); 
#if defined (MOTIF)
				Option_table_add_entry(option_table,"time_editor",NULL,
					command_data_void,gfx_create_time_editor);
#endif /* defined (MOTIF) */
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
				Option_table_add_entry(option_table,"window",NULL,
					command_data_void,gfx_create_window);
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
				return_code=Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("gfx create",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_create.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_create.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_create */

static int gfx_define_faces(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 April 2003

DESCRIPTION :
Executes a GFX DEFINE FACES command.
==============================================================================*/
{
	char *region_path;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct FE_element_list_CM_element_type_data element_list_type_data;
	struct FE_region *fe_region;
	struct LIST(FE_element) *element_list;
	struct Option_table *option_table;

	ENTER(gfx_define_faces);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		Cmiss_region_get_root_region_path(&region_path);

		option_table = CREATE(Option_table)();
		/* egroup */
		Option_table_add_entry(option_table, "egroup", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		if (return_code = Option_table_multi_parse(option_table, state))
		{
			if (Cmiss_region_get_region_from_path(command_data->root_region,
				region_path, &region) && region &&
				(fe_region = Cmiss_region_get_FE_region(region)))
			{
				if (element_list = CREATE(LIST(FE_element))())
				{
					element_list_type_data.cm_element_type = CM_ELEMENT;
					element_list_type_data.element_list = element_list;
					if (FE_region_for_each_FE_element(fe_region,
						add_FE_element_of_CM_element_type_to_list,
						(void *)&element_list_type_data))
					{
						FE_region_begin_change(fe_region);
						FE_region_begin_define_faces(fe_region);
						return_code = FOR_EACH_OBJECT_IN_LIST(FE_element)(
							FE_region_merge_FE_element_and_faces_and_nodes_iterator,
							(void *)fe_region, element_list);
						FE_region_end_define_faces(fe_region);
						FE_region_end_change(fe_region);
					}
					else
					{
						return_code = 0;
					}
					DESTROY(LIST(FE_element))(&element_list);
				}
				else
				{
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx define faces:  Missing or invalid region");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		DEALLOCATE(region_path);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_define_faces.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_define_faces */

static int gfx_define_font(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
Executes a GFX DEFINE FONT command.
==============================================================================*/
{
	char *current_token, *font_name;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_define_font);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				font_name = current_token;
				if (shift_Parse_state(state,1)&&
					(current_token=state->current_token))
				{
					return_code = Graphics_font_package_define_font(
						command_data->graphics_font_package,
						font_name, current_token);
				}
				else
				{
					display_message(WARNING_MESSAGE,"Missing font string.");
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" FONT_NAME FONT_STRING");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing font name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_define_faces.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_define_font */

static int execute_command_gfx_define(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Executes a GFX DEFINE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Curve_command_data curve_command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_define);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* curve */
				curve_command_data.curve_manager = 
					command_data->curve_manager;
				curve_command_data.io_stream_package = 
					command_data->io_stream_package;
				Option_table_add_entry(option_table, "curve", NULL,
					&curve_command_data, gfx_define_Curve);
				/* faces */
				Option_table_add_entry(option_table, "faces", NULL,
					command_data_void, gfx_define_faces);
				/* field */
				Option_table_add_entry(option_table, "field", NULL,
					command_data->computed_field_package, define_Computed_field);
				/* font */
				Option_table_add_entry(option_table, "font", NULL,
					command_data_void, gfx_define_font);
				return_code = Option_table_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("gfx define",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_define.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_define.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_define */

#if defined (MOTIF)
static int gfx_destroy_cmiss(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX DESTROY CMISS_CONNECTION command.
==============================================================================*/
{
#if defined (LINK_CMISS)
	char *current_token;
#endif /* defined (LINK_CMISS) */
	int return_code;

	ENTER(gfx_destroy_cmiss);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
#if defined (LINK_CMISS)
		if (CMISS)
		{
			return_code=DESTROY(CMISS_connection)(&CMISS);
		}
		else
		{
			if (!(current_token=state->current_token)||
				(strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
			{
				display_message(ERROR_MESSAGE,
					"gfx_destroy_cmiss.  No CMISS connection");
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
#else /* defined (LINK_CMISS) */
			display_message(ERROR_MESSAGE,"gfx_destroy_cmiss.  Define LINK_CMISS");
#endif /* defined (LINK_CMISS) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_cmiss.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_cmiss */
#endif /* defined (MOTIF) */

static int gfx_remove_region(struct Parse_state *state,
	void *dummy_to_be_modified, void *root_region_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
Executes a GFX REMOVE REGION command.
==============================================================================*/
{
	char *current_token, *region_path;
	int return_code;
	struct Cmiss_region *last_region, *parent_region, *region;

	ENTER(gfx_remove_region);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (parent_region = (struct Cmiss_region *)root_region_void))
	{
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				/* get region to be removed and the parent_region to remove it from */
				region_path = current_token;
				last_region = parent_region;
				while (region_path &&
					(return_code = Cmiss_region_get_child_region_from_path(
						last_region, region_path, &region, &region_path)) &&
					(region != last_region))
				{
					parent_region = last_region;
					last_region = region;
				}
				if (return_code)
				{
					if (region == parent_region)
					{
						display_message(ERROR_MESSAGE,
							"gfx remove region:  The root region may not be removed");
						display_parse_state_location(state);
						return_code = 0;
					}
					else
					{
						return_code =
							Cmiss_region_remove_child_region(parent_region, region);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "Unknown region: %s", current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " REGION_PATH");
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing region path");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_remove_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_remove_region */

static int gfx_destroy_elements(struct Parse_state *state,
	void *cm_element_type_void, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 March 2003

DESCRIPTION :
Executes a GFX DESTROY ELEMENTS command.
==============================================================================*/
{
	char all_flag, *region_path, selected_flag;
	enum CM_element_type cm_element_type;
	FE_value time;
	int number_not_destroyed, return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct Computed_field *conditional_field;
	struct FE_region *fe_region, *master_fe_region;
	struct LIST(FE_element) *destroy_element_list;
	struct Multi_range *element_ranges;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_conditional_field_data;

	ENTER(gfx_destroy_elements);
	cm_element_type = (enum CM_element_type)cm_element_type_void;
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		/* region_path defaults to NULL so that we have to either specify "all"
			 or group "/" to destroy all elements */
		all_flag = 0;
		region_path = (char *)NULL;
		conditional_field=(struct Computed_field *)NULL;
		selected_flag = 0;
		element_ranges = CREATE(Multi_range)();
		if (command_data->default_time_keeper)
		{
			time = Time_keeper_get_time(command_data->default_time_keeper);
		}
		else
		{
			time = 0.0;
		}

		option_table = CREATE(Option_table)();
		/* all */
		Option_table_add_entry(option_table, "all", &all_flag, NULL, set_char_flag);
		/* conditional_field */
		set_conditional_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
			command_data->computed_field_package);
		set_conditional_field_data.conditional_function=
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_conditional_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"conditional_field",
			&conditional_field,&set_conditional_field_data,
			set_Computed_field_conditional);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* selected */
		Option_table_add_entry(option_table, "selected", &selected_flag, NULL,
			set_char_flag);
		/* default option: element number ranges */
		Option_table_add_entry(option_table, (char *)NULL, (void *)element_ranges,
			NULL, set_Multi_range);
		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (((region_path &&
				Cmiss_region_get_region_from_path(command_data->root_region,
					region_path, &region)) ||
				((all_flag || selected_flag ||
					(0 < Multi_range_get_number_of_ranges(element_ranges))) &&
					(region = command_data->root_region))) &&
				(fe_region = Cmiss_region_get_FE_region(region)) &&
				FE_region_get_ultimate_master_FE_region(fe_region, &master_fe_region))
			{
				if (destroy_element_list =
					FE_element_list_from_fe_region_selection_ranges_condition(
						fe_region, cm_element_type,
						command_data->element_selection, selected_flag,
						element_ranges, conditional_field, time))
				{
					if (0 < NUMBER_IN_LIST(FE_element)(destroy_element_list))
					{
						FE_region_begin_change(master_fe_region);
						FE_region_remove_FE_element_list(master_fe_region,
							destroy_element_list);
						if (0 < (number_not_destroyed =
							NUMBER_IN_LIST(FE_element)(destroy_element_list)))
						{
							display_message(WARNING_MESSAGE,
								"%d of the selected element(s) could not be destroyed",
								number_not_destroyed);
						}
						FE_region_end_change(master_fe_region);
					}
					else
					{
						switch (cm_element_type)
						{
							case CM_ELEMENT:
							{
								display_message(INFORMATION_MESSAGE,
									"gfx destroy elements:  No elements specified\n");
							} break;
							case CM_FACE:
							{
								display_message(INFORMATION_MESSAGE,
									"gfx destroy faces:  No faces specified\n");
							} break;
							case CM_LINE:
							{
								display_message(INFORMATION_MESSAGE,
									"gfx destroy lines:  No lines specified\n");
							} break;
						}
						return_code = 0;
					}
					DESTROY(LIST(FE_element))(&destroy_element_list);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_destroy_elements.  Could not make destroy_element_list");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "gfx_destroy_elements.  Invalid region");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (conditional_field)
		{
			DEACCESS(Computed_field)(&conditional_field);
		}
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
		DESTROY(Multi_range)(&element_ranges);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_elements.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_elements */

static int gfx_destroy_Computed_field(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Executes a GFX DESTROY FIELD command.
==============================================================================*/
{
	char *current_token;
	struct Computed_field *computed_field;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct FE_field *fe_field;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(gfx_destroy_Computed_field);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void)&&
		(computed_field_manager=Computed_field_package_get_computed_field_manager(
			command_data->computed_field_package)))
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (computed_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
					current_token,computed_field_manager))
				{
					if (MANAGED_OBJECT_NOT_IN_USE(Computed_field)(computed_field,
						computed_field_manager))
					{
						/* also want to destroy wrapped FE_field */
						fe_field = (struct FE_field *)NULL;
						if (Computed_field_is_type_finite_element(computed_field))
						{
							Computed_field_get_type_finite_element(computed_field,
								&fe_field);
						}
						if (fe_field)
						{
							return_code = FE_region_remove_FE_field(
								FE_field_get_FE_region(fe_field), fe_field);
						}
						else
						{
							return_code = REMOVE_OBJECT_FROM_MANAGER(Computed_field)(
								computed_field, computed_field_manager);
						}
						if (!return_code)
						{
							display_message(ERROR_MESSAGE,
								"gfx_destroy_Computed_field.  Could not destroy field");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cannot destroy field in use : %s",current_token);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Field does not exist: %s",
						current_token);
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," FIELD_NAME");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing field name");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_Computed_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_Computed_field */

static int Scene_remove_graphics_object_iterator(struct Scene *scene,
	void *graphics_object_void)
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Removes all instances of the <graphics_object> from <scene>.
???RC Move to scene.c?
==============================================================================*/
{
	int return_code;
	struct GT_object *graphics_object;

	ENTER(Scene_remove_graphics_object_iterator);
	if (scene&&(graphics_object=(struct GT_object *)graphics_object_void))
	{
		while (Scene_has_graphics_object(scene,graphics_object))
		{
			Scene_remove_graphics_object(scene,graphics_object);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_remove_graphics_object_iterator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_remove_graphics_object_iterator */

static int gfx_destroy_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Executes a GFX DESTROY GRAPHICS_OBJECT command.
==============================================================================*/
{
	char *current_token;
	gtObject *graphics_object;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(gfx_destroy_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
						current_token,command_data->graphics_object_list))
					{
						/* remove all instances of the graphics object from all scenes */
						FOR_EACH_OBJECT_IN_MANAGER(Scene)(
							Scene_remove_graphics_object_iterator,(void *)graphics_object,
							command_data->scene_manager);
						/* remove graphics object from the global list. Object is destroyed
							 when deaccessed by list */
						REMOVE_OBJECT_FROM_LIST(GT_object)(graphics_object,
							command_data->graphics_object_list);
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"Graphics object does not exist: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," GRAPHICS_OBJECT_NAME");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing graphics object name");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_graphics_object.  Missing graphics_object_list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_graphics_object.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_graphics_object */

static int gfx_destroy_material(struct Parse_state *state,
	void *dummy_to_be_modified, void *graphical_material_manager_void)
/*******************************************************************************
LAST MODIFIED : 8 May 2002

DESCRIPTION :
Executes a GFX DESTROY MATERIAL command.
==============================================================================*/
{
	char *current_token;
	struct Graphical_material *graphical_material;
	int return_code;
	struct MANAGER(Graphical_material) *graphical_material_manager;

	ENTER(gfx_destroy_material);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (graphical_material_manager =
		(struct MANAGER(Graphical_material) *)graphical_material_manager_void))
	{
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (graphical_material =
					FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material, name)(
						current_token, graphical_material_manager))
				{
					if (REMOVE_OBJECT_FROM_MANAGER(Graphical_material)(graphical_material,
						graphical_material_manager))
					{
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Could not remove material %s from manager", current_token);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Unknown material: %s", current_token);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " MATERIAL_NAME");
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing material name");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_material.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_material */

static int gfx_destroy_nodes(struct Parse_state *state,
	void *use_data,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 March 2003

DESCRIPTION :
Executes a GFX DESTROY NODES/DATA command.
If <used_data_flag> is set, use data_manager and data_selection, otherwise
use node_manager and node_selection.
==============================================================================*/
{
	char all_flag, *region_path, selected_flag;
	FE_value time;
	int number_not_destroyed, return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region, *root_region;
	struct Computed_field *conditional_field;
	struct FE_node_selection *node_selection;
	struct FE_region *fe_region, *master_fe_region;
	struct LIST(FE_node) *destroy_node_list;
	struct Multi_range *node_ranges;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_conditional_field_data;

	ENTER(gfx_destroy_nodes);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (use_data)
		{
			root_region = command_data->data_root_region;
			node_selection = command_data->data_selection;
		}
		else
		{
			root_region = command_data->root_region;
			node_selection = command_data->node_selection;
		}
		/* initialise defaults */
		all_flag = 0;
		region_path = (char *)NULL;
		conditional_field=(struct Computed_field *)NULL;
		selected_flag = 0;
		node_ranges = CREATE(Multi_range)();
		if (command_data->default_time_keeper)
		{
			time = Time_keeper_get_time(command_data->default_time_keeper);
		}
		else
		{
			time = 0.0;
		}

		option_table = CREATE(Option_table)();
		/* all */
		Option_table_add_entry(option_table, "all", &all_flag, NULL, set_char_flag);
		/* conditional_field */
		set_conditional_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
			command_data->computed_field_package);
		set_conditional_field_data.conditional_function=
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_conditional_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"conditional_field",
			&conditional_field,&set_conditional_field_data,
			set_Computed_field_conditional);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			root_region, set_Cmiss_region_path);
		/* selected */
		Option_table_add_entry(option_table, "selected", &selected_flag,
			NULL, set_char_flag);
		/* default option: node number ranges */
		Option_table_add_entry(option_table, (char *)NULL, (void *)node_ranges,
			NULL, set_Multi_range);
		if (return_code = Option_table_multi_parse(option_table, state))
		{
			if (((region_path &&
				Cmiss_region_get_region_from_path(root_region, region_path, &region)) ||
				((all_flag || selected_flag ||
					(0 < Multi_range_get_number_of_ranges(node_ranges))) &&
					(region = root_region))) &&
				(fe_region = Cmiss_region_get_FE_region(region)) &&
				FE_region_get_ultimate_master_FE_region(fe_region, &master_fe_region))
			{
				if (destroy_node_list =
					FE_node_list_from_fe_region_selection_ranges_condition(
						fe_region, node_selection, selected_flag, node_ranges,
						conditional_field, time))
				{
					if (0 < (number_not_destroyed =
						NUMBER_IN_LIST(FE_node)(destroy_node_list)))
					{
						FE_region_begin_change(master_fe_region);
						FE_region_remove_FE_node_list(master_fe_region, destroy_node_list);
						if (0 < (number_not_destroyed =
							NUMBER_IN_LIST(FE_node)(destroy_node_list)))
						{
							display_message(WARNING_MESSAGE,
								"%d of the selected node(s) could not be destroyed",
								number_not_destroyed);
						}
						FE_region_end_change(master_fe_region);
					}
					else
					{
						if (use_data)
						{
							display_message(WARNING_MESSAGE,
								"gfx destroy data:  No data specified");
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"gfx destroy nodes:  No nodes specified");
						}
					}
					DESTROY(LIST(FE_node))(&destroy_node_list);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_destroy_nodes.  Could not make destroy_node_list");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "gfx_destroy_elements.  Invalid region");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (conditional_field)
		{
			DEACCESS(Computed_field)(&conditional_field);
		}
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
		DESTROY(Multi_range)(&node_ranges);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_nodes.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_nodes */

static int gfx_destroy_Scene(struct Parse_state *state,
	void *dummy_to_be_modified, void *scene_manager_void)
/*******************************************************************************
LAST MODIFIED : 5 December 2001

DESCRIPTION :
Executes a GFX DESTROY SCENE command.
==============================================================================*/
{
	char *current_token;
	struct Scene *scene;
	int return_code;
	struct MANAGER(Scene) *scene_manager;

	ENTER(gfx_destroy_Scene);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (scene_manager = (struct MANAGER(Scene) *)scene_manager_void))
	{
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (scene = FIND_BY_IDENTIFIER_IN_MANAGER(Scene, name)(
					current_token, scene_manager))
				{
					if (REMOVE_OBJECT_FROM_MANAGER(Scene)(scene, scene_manager))
					{
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Could not remove scene %s from manager", current_token);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "Unknown scene: %s", current_token);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " SCENE_NAME");
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing scene name");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_destroy_Scene.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_Scene */

static int gfx_destroy_texture(struct Parse_state *state,
	void *dummy_to_be_modified, void *texture_manager_void)
/*******************************************************************************
LAST MODIFIED : 8 May 2002

DESCRIPTION :
Executes a GFX DESTROY TEXTURE command.
==============================================================================*/
{
	char *current_token;
	struct Texture *texture;
	int return_code;
	struct MANAGER(Texture) *texture_manager;

	ENTER(gfx_destroy_texture);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (texture_manager =
		(struct MANAGER(Texture) *)texture_manager_void))
	{
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (texture = FIND_BY_IDENTIFIER_IN_MANAGER(Texture, name)(
					current_token, texture_manager))
				{
					if (REMOVE_OBJECT_FROM_MANAGER(Texture)(texture, texture_manager))
					{
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Could not remove texture %s from manager", current_token);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Unknown texture: %s", current_token);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " TEXTURE_NAME");
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing texture name");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_texture.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_texture */

static int gfx_destroy_vtextures(struct Parse_state *state,
	void *dummy_to_be_modified,void *volume_texture_manager_void)
/*******************************************************************************
LAST MODIFIED : 7 October 1996

DESCRIPTION :
Executes a GFX DESTROY VTEXTURES command.
???DB.  Could merge with destroy_graphics_objects if graphics_objects used the
	new list structures.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;
	struct VT_volume_texture *volume_texture;

	ENTER(gfx_destroy_vtextures);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (volume_texture_manager=
			(struct MANAGER(VT_volume_texture) *)volume_texture_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (volume_texture=FIND_BY_IDENTIFIER_IN_MANAGER(VT_volume_texture,
						name)(current_token,volume_texture_manager))
					{
						/* remove object from list (destroys automatically) */
						return_code=REMOVE_OBJECT_FROM_MANAGER(VT_volume_texture)(
							volume_texture,volume_texture_manager);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Volume texture does not exist: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," <NAME[all]>");
					return_code=1;
				}
			}
			else
			{
				/* destroy all objects in list */
				return_code=REMOVE_ALL_OBJECTS_FROM_MANAGER(VT_volume_texture)(
					volume_texture_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_vtextures.  Missing volume texture manager");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_vtextures.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_vtextures */

#if defined (MOTIF)
static int gfx_destroy_Graphics_window(struct Parse_state *state,
	void *dummy_to_be_modified, void *graphics_window_manager_void)
/*******************************************************************************
LAST MODIFIED : 18 September 2001

DESCRIPTION :
Executes a GFX DESTROY WINDOW command.
==============================================================================*/
{
	char *current_token;
	struct Graphics_window *graphics_window;
	int return_code;
	struct MANAGER(Graphics_window) *graphics_window_manager;

	ENTER(gfx_destroy_Graphics_window);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (graphics_window_manager =
		(struct MANAGER(Graphics_window) *)graphics_window_manager_void))
	{
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (graphics_window =
					FIND_BY_IDENTIFIER_IN_MANAGER(Graphics_window, name)(
						current_token, graphics_window_manager))
				{
					if (REMOVE_OBJECT_FROM_MANAGER(Graphics_window)(graphics_window,
						graphics_window_manager))
					{
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Could not remove graphics window %s from manager",
							current_token);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Unknown graphics window: %s", current_token);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " GRAPHICS_WINDOW_NAME");
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing graphics window name");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_Graphics_window.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_Graphics_window */
#endif /* defined (MOTIF) */

static int execute_command_gfx_destroy(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Executes a GFX DESTROY command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_destroy);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data = (struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				option_table = CREATE(Option_table)();
#if defined (MOTIF)
				/* cmiss_connection */
				Option_table_add_entry(option_table, "cmiss_connection", NULL,
					command_data_void, gfx_destroy_cmiss);
#endif /* defined (MOTIF) */
				/* curve */
				Option_table_add_entry(option_table, "curve", NULL,
					command_data->curve_manager, gfx_destroy_Curve);
				/* data */
				Option_table_add_entry(option_table, "data", /*use_data*/(void *)1,
					command_data_void, gfx_destroy_nodes);
				/* dgroup */
				Option_table_add_entry(option_table, "dgroup", NULL,
					command_data->data_root_region, gfx_remove_region);
				/* egroup */
				Option_table_add_entry(option_table, "egroup", NULL,
					command_data->root_region, gfx_remove_region);
				/* elements */
				Option_table_add_entry(option_table, "elements", (void *)CM_ELEMENT,
					command_data_void, gfx_destroy_elements);
				/* faces */
				Option_table_add_entry(option_table, "faces", (void *)CM_FACE,
					command_data_void, gfx_destroy_elements);
				/* field */
				Option_table_add_entry(option_table, "field", NULL,
					command_data_void, gfx_destroy_Computed_field);
				/* graphics_object */
				Option_table_add_entry(option_table, "graphics_object", NULL,
					command_data_void, gfx_destroy_graphics_object);
				/* lines */
				Option_table_add_entry(option_table, "lines", (void *)CM_LINE,
					command_data_void, gfx_destroy_elements);
				/* material */
				Option_table_add_entry(option_table, "material", NULL,
					Material_package_get_material_manager(command_data->material_package), gfx_destroy_material);
				/* ngroup */
				Option_table_add_entry(option_table, "ngroup", NULL,
					command_data->root_region, gfx_remove_region);
				/* nodes */
				Option_table_add_entry(option_table, "nodes", /*use_data*/(void *)0,
					command_data_void, gfx_destroy_nodes);
				/* scene */
				Option_table_add_entry(option_table, "scene", NULL,
					command_data->scene_manager, gfx_destroy_Scene);
				/* spectrum */
				Option_table_add_entry(option_table, "spectrum", NULL,
					command_data->spectrum_manager, gfx_destroy_spectrum);
				/* texture */
				Option_table_add_entry(option_table, "texture", NULL,
					command_data->texture_manager, gfx_destroy_texture);
				/* vtextures */
				Option_table_add_entry(option_table, "vtextures", NULL,
					command_data->volume_texture_manager, gfx_destroy_vtextures);
#if defined (MOTIF)
				/* window */
				Option_table_add_entry(option_table, "window", NULL,
					command_data->graphics_window_manager, gfx_destroy_Graphics_window);
#endif /* defined (MOTIF) */
				return_code = Option_table_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("gfx destroy",command_data);
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_destroy.  Invalid argument(s)");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_destroy.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_destroy */

struct Scene_add_graphics_object_iterator_data
{
	int position;
	struct Scene *scene;
};

static int Scene_add_graphics_object_iterator(struct GT_object *graphics_object,
	void *data_void)
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
==============================================================================*/
{
	char *name;
	int return_code;
	struct Scene *scene;
	struct Scene_add_graphics_object_iterator_data *data;

	ENTER(Scene_add_graphics_object_iterator);
	return_code = 0;
	if (graphics_object &&
		(data = (struct Scene_add_graphics_object_iterator_data *)data_void )&&
		(scene = data->scene))
	{
		if (Scene_has_graphics_object(scene, graphics_object))
		{
			return_code = 1;
		}
		else
		{
			if (GET_NAME(GT_object)(graphics_object, &name))
			{
				return_code = Scene_add_graphics_object(scene,graphics_object,
					data->position, name, /*fast_changing*/0);
				DEALLOCATE(name);
			}
			if (0 < data->position)
			{
				data->position++;
			}
		}
		if (1 < GT_object_get_number_of_times(graphics_object))
		{
			Scene_update_time_behaviour(data->scene, graphics_object);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_add_graphics_object_iterator.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Scene_add_graphics_object_iterator */

static int execute_command_gfx_draw(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Executes a GFX DRAW command.
==============================================================================*/
{
	char *region_path, *scene_object_name, *time_object_name;
	struct GT_object *graphics_object;
	int return_code, position;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *data_region, *region;
	struct Scene *child_scene, *scene;
	struct Scene_add_graphics_object_iterator_data data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_draw);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		/* initialize defaults */
		graphics_object = (struct GT_object *)NULL;
		region_path = (char *)NULL;
		scene_object_name = (char *)NULL;
		time_object_name = (char *)NULL;
		position = 0;
		scene = ACCESS(Scene)(command_data->default_scene);
		child_scene = (struct Scene *)NULL;

		option_table=CREATE(Option_table)();
		/* as */
		Option_table_add_entry(option_table,"as",&scene_object_name,
			(void *)1,set_name);
		/* child_scene */
		Option_table_add_entry(option_table,"child_scene",&child_scene,
			command_data->scene_manager,set_Scene);
		/* graphics_object */
		Option_table_add_entry(option_table,"graphics_object",&graphics_object,
			command_data->graphics_object_list,set_Graphics_object);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* position */
		Option_table_add_entry(option_table,"position",&position,
			(void *)1,set_int);
		/* scene */
		Option_table_add_entry(option_table,"scene",&scene,
			command_data->scene_manager,set_Scene);
		/* time_object */
		Option_table_add_entry(option_table,"time_object",&time_object_name,
			(void *)1,set_name);
		/* default when token omitted (graphics_object) */
		Option_table_add_entry(option_table,(char *)NULL,&graphics_object,
			command_data->graphics_object_list,set_Graphics_object);
		return_code = Option_table_multi_parse(option_table,state);
		if ((child_scene && graphics_object) ||
			(graphics_object && region_path) ||
			(region_path && child_scene))
		{
			display_message(ERROR_MESSAGE, "execute_command_gfx_draw.  "
				"Specify only one of child_scene|graphics_object|group");
			return_code = 0;
		}
		if (child_scene && time_object_name)
		{
			display_message(ERROR_MESSAGE, "execute_command_gfx_draw.  "
				"Time objects may not be associated with a child_scene");
			return_code = 0;
		}
		/* no errors, not asking for help */
		if (return_code)
		{
			if (graphics_object)
			{
				if (scene)
				{
					return_code = Scene_add_graphics_object(scene, graphics_object,
						position, scene_object_name, /*fast_changing*/0);
					if (time_object_name)
					{
						/* SAB A new time_object is created and associated with the named
							 scene_object, the time_keeper is supplied so that the
							 default could be overridden */
						Scene_set_time_behaviour(scene,scene_object_name, time_object_name,
							command_data->default_time_keeper);
					}
					if (1<GT_object_get_number_of_times(graphics_object))
					{
						/* any scene_objects referring to this graphics_object which do
							 not already have a time_object all are associated with a
							 single common time_object */
						Scene_update_time_behaviour(scene,graphics_object);
					}
				}
			}
			else if (child_scene)
			{
				return_code = Scene_add_child_scene(scene, child_scene, position,
					scene_object_name, command_data->scene_manager);
			}
			else if (region_path)
			{
				if (Cmiss_region_get_region_from_path(command_data->root_region,
					region_path, &region) &&
					Cmiss_region_get_region_from_path(command_data->data_root_region,
						region_path, &data_region))
				{
					if (!scene_object_name)
					{
						scene_object_name = duplicate_string(region_path);
					}
					return_code = Scene_add_graphical_element_group(scene,
						region, data_region, position, scene_object_name);
				}
			}
			else
			{
				data.scene = scene;
				data.position = position;
				return_code = FOR_EACH_OBJECT_IN_LIST(GT_object)(
					Scene_add_graphics_object_iterator, (void *)&data,
					command_data->graphics_object_list);
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
		if (scene)
		{
			DEACCESS(Scene)(&scene);
		}
		if (child_scene)
		{
			DEACCESS(Scene)(&child_scene);
		}
		if (graphics_object)
		{
			DEACCESS(GT_object)(&graphics_object);
		}
		if (scene_object_name)
		{
			DEALLOCATE(scene_object_name);
		}
		if (time_object_name)
		{
			DEALLOCATE(time_object_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_draw.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_draw */

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
	struct FE_field *coordinate_field;

	ENTER(apply_transformation_to_node);
	return_code = 0;
	if (node && (data = (struct Apply_transformation_data  *)data_void))
	{
		if (coordinate_field = FE_node_get_position_cartesian(node,
			(struct FE_field *)NULL,&x,&y,&z,(FE_value *)NULL))
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

static int gfx_edit_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Executes a GFX EDIT GRAPHICS_OBJECT command.
==============================================================================*/
{
	char apply_flag, *graphics_object_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct FE_region *fe_region;
	struct Scene_object *scene_object;
	struct Scene *scene;
	struct Option_table *option_table;

	ENTER(gfx_edit_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialize defaults */
		apply_flag = 0;
		graphics_object_name = (char *)NULL;
		scene = ACCESS(Scene)(command_data->default_scene);

		option_table = CREATE(Option_table)();
		/* apply_transformation */
		Option_table_add_entry(option_table, "apply_transformation",  &apply_flag,
			NULL, set_char_flag);
		/* name */
		Option_table_add_entry(option_table, "name", &graphics_object_name,
			(void *)1, set_name);
		/* scene */
		Option_table_add_entry(option_table, "scene", &scene,
			command_data->scene_manager, set_Scene);
		/* default when token omitted (graphics_object_name) */
		Option_table_add_entry(option_table, (char *)NULL, &graphics_object_name,
			(void *)0, set_name);
		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (scene && graphics_object_name && (scene_object=
				Scene_get_Scene_object_by_name(scene, graphics_object_name)))
			{
				if (apply_flag)
				{							
					/* SAB Temporary place for this command cause I really need to use it,
						 not very general, doesn't work in prolate or rotate derivatives */
					struct Apply_transformation_data data;
					struct GT_element_group *gt_element_group;
					gtMatrix identity = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};

					if (Scene_object_has_transformation(scene_object)&&
						Scene_object_get_transformation(scene_object, &(data.transformation)))
					{
						if (Scene_object_has_graphical_element_group(scene_object, NULL) &&
							(gt_element_group = 
								Scene_object_get_graphical_element_group(scene_object)))
						{
							if ((region = GT_element_group_get_Cmiss_region(gt_element_group))
								&& (fe_region = Cmiss_region_get_FE_region(region)))
							{
								data.fe_region = fe_region;
								FE_region_begin_change(fe_region);
								FE_region_for_each_FE_node(fe_region,
									apply_transformation_to_node, (void *)&data);
								FE_region_end_change(fe_region);
							}
							else
							{
								display_message(WARNING_MESSAGE, "Invalid region");
								return_code = 0;
							}
							if ((region =
								GT_element_group_get_data_Cmiss_region(gt_element_group)) &&
								(fe_region = Cmiss_region_get_FE_region(region)))
							{
								data.fe_region = fe_region;
								FE_region_begin_change(fe_region);
								FE_region_for_each_FE_node(fe_region,
									apply_transformation_to_node, (void *)&data);
								FE_region_end_change(fe_region);
							}
							else
							{
								display_message(WARNING_MESSAGE, "Invalid data region");
								return_code = 0;
							}
							Scene_object_set_transformation(scene_object, &identity);
						}
						else
						{
							return_code = 1;
						}
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"gfx edit graphics_object:  Must specify 'apply_transformation'");
					return_code = 0;
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"Must specify name of graphics object in scene");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		DEACCESS(Scene)(&scene);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_edit_graphics_object.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_edit_graphics_object */

#if defined (MOTIF) || defined (WX_USER_INTERFACE)
static int gfx_edit_scene(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :

Executes a GFX EDIT_SCENE command.  Brings up the Scene_editor.
==============================================================================*/
{
	char close_flag;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Scene *scene;
	ENTER(gfx_edit_scene);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (command_data->scene_editor)
		{
			scene = Scene_editor_get_scene(command_data->scene_editor);
		}
		else
		{
			scene = command_data->default_scene;
		}
		ACCESS(Scene)(scene);
		close_flag = 0;

		option_table = CREATE(Option_table)();
		/* scene (to edit) */
		Option_table_add_entry(option_table, "scene",&scene,
			command_data->scene_manager,set_Scene);
		/* close (editor) */
		Option_table_add_entry(option_table, "close", &close_flag,
			NULL, set_char_flag);
		if (return_code = Option_table_multi_parse(option_table, state))
		{
			if (command_data->scene_editor)
			{
				if (close_flag)
				{
					DESTROY(Scene_editor)(&(command_data->scene_editor));
				}
				else
				{
					if (scene != Scene_editor_get_scene(command_data->scene_editor))
					{
						return_code = Scene_editor_set_scene(command_data->scene_editor,
							scene);
					}
					Scene_editor_bring_to_front(command_data->scene_editor);
				}
			}
			else if (close_flag)
			{
				display_message(ERROR_MESSAGE,
					"gfx edit scene:  There is no scene editor to close");
				return_code = 0;
			}
			else
			{
#if defined (MOTIF)
				if ((!command_data->user_interface) ||
					(!CREATE(Scene_editor)(
						&(command_data->scene_editor),
						User_interface_get_application_shell(command_data->user_interface),
						command_data->scene_manager,
						scene,
						command_data->computed_field_package,
						command_data->root_region,
						Material_package_get_material_manager(command_data->material_package),
						Material_package_get_default_material(command_data->material_package),
						command_data->default_font,
						command_data->glyph_list,
						command_data->spectrum_manager,
						command_data->default_spectrum,
						command_data->volume_texture_manager,
						command_data->user_interface)))
#elif defined (WX_USER_INTERFACE)
				if ((!command_data->user_interface) ||
					(!CREATE(Scene_editor)(	&(command_data->scene_editor),
						command_data->scene_manager,
						scene,
						command_data->computed_field_package,
						command_data->root_region,
						Material_package_get_material_manager(command_data->material_package),
						Material_package_get_default_material(command_data->material_package),
						command_data->default_font,
						command_data->glyph_list,
						command_data->spectrum_manager,
						command_data->default_spectrum,
						command_data->volume_texture_manager,
						command_data->graphics_font_package,
						command_data->user_interface)))
#endif /* defined (SWITCH_USER_INTERFACE) */
				{
					display_message(ERROR_MESSAGE, "gfx_edit_scene.  "
						"Could not create scene editor");
					return_code = 0;
				}
			}
		} /* parse error, help */
		DESTROY(Option_table)(&option_table);
		if (scene)
		{
			DEACCESS(Scene)(&scene);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_edit_scene.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_edit_scene */
#endif /* defined (MOTIF) || defined (WX_USER_INTERFACE) */

#if defined (MOTIF) || defined (WX_USER_INTERFACE)
static int gfx_edit_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
Executes a GFX EDIT SPECTRUM command.
Invokes the graphical spectrum group editor.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Spectrum *spectrum;
	struct Option_table *option_table;

	ENTER(gfx_edit_spectrum);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialize defaults */
		spectrum = (struct Spectrum *)NULL;
		option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table, (char *)NULL, &spectrum,
			command_data->spectrum_manager, set_Spectrum);
		if (return_code = Option_table_multi_parse(option_table, state))
		{
			return_code = bring_up_spectrum_editor_dialog(
				&(command_data->spectrum_editor_dialog),
#if defined (MOTIF)
				User_interface_get_application_shell(command_data->user_interface),
#endif /* defined (MOTIF) */
				command_data->spectrum_manager, spectrum,
				command_data->default_font,
				command_data->graphics_buffer_package, command_data->user_interface,
				command_data->glyph_list,
				Material_package_get_material_manager(command_data->material_package), command_data->light_manager,
				command_data->texture_manager, command_data->scene_manager);
		} /* parse error, help */
		DESTROY(Option_table)(&option_table);
		if (spectrum)
		{
			DEACCESS(Spectrum)(&spectrum);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_edit_spectrum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_edit_spectrum */
#endif /* defined (MOTIF)  || defined (WX_USER_INTERFACE) */

static int execute_command_gfx_edit(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 31 October 2001

DESCRIPTION :
Executes a GFX EDIT command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_edit);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table, "graphics_object", NULL,
				command_data_void, gfx_edit_graphics_object);
#if defined (MOTIF) || defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "scene", NULL,
				command_data_void, gfx_edit_scene);
#endif /* defined (MOTIF) || if defined (WX_USER_INTERFACE) */
#if defined (MOTIF) || defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "spectrum", NULL,
				command_data_void, gfx_edit_spectrum);
#endif /* defined (MOTIF) || defined (WX_USER_INTERFACE) */
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx edit", command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_edit.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_edit */

#if defined (MOTIF)
static int execute_command_gfx_element_creator(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Executes a GFX ELEMENT_CREATOR command.
==============================================================================*/
{
	char *region_path;
	int create_enabled, element_dimension, return_code;
	struct Cmiss_command_data *command_data;
	struct Element_creator *element_creator;
	struct FE_field *coordinate_field;
	struct Option_table *option_table;
	struct Set_FE_field_conditional_FE_region_data set_coordinate_field_data;

	ENTER(execute_command_gfx_element_creator);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialize defaults */
		region_path = (char *)NULL;
		if (element_creator=command_data->element_creator)
		{
			create_enabled = Element_creator_get_create_enabled(element_creator);
			Element_creator_get_region_path(element_creator, &region_path);
			element_dimension =
				Element_creator_get_element_dimension(element_creator);
			coordinate_field = Element_creator_get_coordinate_field(element_creator);
		}
		else
		{
			create_enabled = 0;
			Cmiss_region_get_root_region_path(&region_path);
			element_dimension = 2;
			coordinate_field = (struct FE_field *)NULL;
		}
		if (coordinate_field)
		{
			ACCESS(FE_field)(coordinate_field);
		}

		option_table=CREATE(Option_table)();
		/* coordinate_field */
		set_coordinate_field_data.fe_region =
			Cmiss_region_get_FE_region(command_data->root_region);
		set_coordinate_field_data.conditional_function=FE_field_is_coordinate_field;
		set_coordinate_field_data.user_data=(void *)NULL;
		Option_table_add_entry(option_table, "coordinate_field",
			&coordinate_field, &set_coordinate_field_data,
			set_FE_field_conditional_FE_region);
		/* create/no_create */
		Option_table_add_switch(option_table,"create","no_create",&create_enabled);
		/* dimension */
		Option_table_add_entry(option_table,"dimension",
			&element_dimension,NULL,set_int_non_negative);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (element_creator)
			{
				if (region_path)
				{
					Element_creator_set_region_path(element_creator, region_path);
					Element_creator_set_coordinate_field(element_creator,
						coordinate_field);
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"Please specify a region for the element_creator");
					return_code = 0;
				}
				Element_creator_set_create_enabled(element_creator, create_enabled);
				Element_creator_set_element_dimension(element_creator,
					element_dimension);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Must create element_creator before modifying it");
				return_code = 0;
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
		if (coordinate_field)
		{
			DEACCESS(FE_field)(&coordinate_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_element_creator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_element_creator */
#endif /* defined (MOTIF) */

#if defined (MOTIF) || (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE)
static int execute_command_gfx_element_point_tool(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Executes a GFX ELEMENT_POINT_TOOL command.
==============================================================================*/
{
	static char *(dialog_strings[2]) = {"open_dialog", "close_dialog"};
	char *dialog_string;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Computed_field *command_field;
	struct Element_point_tool *element_point_tool;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_command_field_data;

	ENTER(execute_command_gfx_element_point_tool);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialize defaults */
		if (element_point_tool=command_data->element_point_tool)
		{
			command_field = Element_point_tool_get_command_field(element_point_tool);
		}
		else
		{
			command_field = (struct Computed_field *)NULL;
		}
		if (command_field)
		{
			ACCESS(Computed_field)(command_field);
		}
		option_table = CREATE(Option_table)();
		/* open_dialog/close_dialog */
		dialog_string = (char *)NULL;
		Option_table_add_enumerator(option_table, /*number_of_valid_strings*/2,
			dialog_strings, &dialog_string);
		/* command_field */
		set_command_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_command_field_data.conditional_function =
			Computed_field_has_string_value_type;
		set_command_field_data.conditional_function_user_data = (void *)NULL;
		Option_table_add_entry(option_table, "command_field", &command_field,
			&set_command_field_data, set_Computed_field_conditional);
		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (element_point_tool)
			{
				if (dialog_string == dialog_strings[1])
				{
					Element_point_tool_pop_down_dialog(element_point_tool);
				}
				Element_point_tool_set_command_field(element_point_tool,command_field);
				if (dialog_string == dialog_strings[0])
				{
					Element_point_tool_pop_up_dialog(element_point_tool, (struct Graphics_window *)NULL);
				}
#if defined (WX_USER_INTERFACE)
			FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
				 Graphics_window_update_Interactive_tool,
				 (void *)Element_point_tool_get_interactive_tool(element_point_tool),
				 command_data->graphics_window_manager);
#endif
			Cmiss_scene_viewer_package_update_Interactive_tool(
				command_data->scene_viewer_package,
				Element_point_tool_get_interactive_tool(element_point_tool));
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_gfx_element_point_tool.  "
					"Missing element_point_tool");
				return_code = 0;
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		if (command_field)
		{
			DEACCESS(Computed_field)(&command_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_element_point_tool.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_element_point_tool */
#endif /* defined (MOTIF) || (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) */

#if defined (MOTIF) || (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE)
static int execute_command_gfx_element_tool(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Executes a GFX ELEMENT_TOOL command.
==============================================================================*/
{
	static char *(dialog_strings[2]) = {"open_dialog", "close_dialog"};
	char *dialog_string;
	int select_elements_enabled,select_faces_enabled,select_lines_enabled,
		return_code;
	struct Cmiss_command_data *command_data;
	struct Computed_field *command_field;
	struct Element_tool *element_tool;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_command_field_data;

	ENTER(execute_command_gfx_element_tool);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		/* initialize defaults */
		if (element_tool=command_data->element_tool)
		{
			select_elements_enabled=
				Element_tool_get_select_elements_enabled(element_tool);
			select_faces_enabled=Element_tool_get_select_faces_enabled(element_tool);
			select_lines_enabled=Element_tool_get_select_lines_enabled(element_tool);
			command_field=Element_tool_get_command_field(element_tool);
		}
		else
		{
			select_elements_enabled=1;
			select_faces_enabled=1;
			select_lines_enabled=1;
			command_field = (struct Computed_field *)NULL;
		}
		if (command_field)
		{
			ACCESS(Computed_field)(command_field);
		}
		option_table=CREATE(Option_table)();
		/* open_dialog/close_dialog */
		dialog_string = (char *)NULL;
		Option_table_add_enumerator(option_table, /*number_of_valid_strings*/2,
			dialog_strings, &dialog_string);
		/* select_elements/no_select_elements */
		Option_table_add_switch(option_table,"select_elements","no_select_elements",
			&select_elements_enabled);
		/* select_faces/no_select_faces */
		Option_table_add_switch(option_table,"select_faces","no_select_faces",
			&select_faces_enabled);
		/* select_lines/no_select_lines */
		Option_table_add_switch(option_table,"select_lines","no_select_lines",
			&select_lines_enabled);
		/* command_field */
		set_command_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_command_field_data.conditional_function =
			Computed_field_has_string_value_type;
		set_command_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"command_field",&command_field,
			&set_command_field_data,set_Computed_field_conditional);
		if (return_code=Option_table_multi_parse(option_table,state))
		{
			if (element_tool)
			{
				if (dialog_string == dialog_strings[1])
				{
					Element_tool_pop_down_dialog(element_tool);
				}
				Element_tool_set_select_elements_enabled(element_tool,
					select_elements_enabled);
				Element_tool_set_select_faces_enabled(element_tool,
					select_faces_enabled);
				Element_tool_set_select_lines_enabled(element_tool,
					select_lines_enabled);
				Element_tool_set_command_field(element_tool,command_field);
				if (dialog_string == dialog_strings[0])
				{
					Element_tool_pop_up_dialog(element_tool, (struct Graphics_window *)NULL);
				}
#if defined (WX_USER_INTERFACE)
			FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
				 Graphics_window_update_Interactive_tool,
				 (void *)Element_tool_get_interactive_tool(element_tool),
				 command_data->graphics_window_manager);
#endif /*(WX_USER_INTERFACE)*/
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_gfx_element_tool.  Missing element_tool");
				return_code=0;
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		if (command_field)
		{
			DEACCESS(Computed_field)(&command_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_element_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_element_tool */
#endif /* defined (MOTIF) || (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE) */ 

static int execute_command_gfx_erase(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2001

DESCRIPTION :
Executes a GFX ERASE command.
==============================================================================*/
{
	char *scene_name, *scene_object_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Scene *scene;
	struct Scene_object *scene_object;

	ENTER(execute_command_gfx_erase);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		scene_object_name = (char *)NULL;
		scene = ACCESS(Scene)(command_data->default_scene);
		option_table = CREATE(Option_table)();
		/* scene */
		Option_table_add_entry(option_table, "scene", &scene,
			command_data->scene_manager, set_Scene);
		/* default option: scene object name */
		Option_table_add_entry(option_table, (char *)NULL, &scene_object_name,
			NULL, set_name);
		if (return_code = Option_table_multi_parse(option_table, state))
		{
			if (scene && scene_object_name)
			{
				if (scene_object =
					Scene_get_Scene_object_by_name(scene, scene_object_name))
				{
					if (Scene_remove_Scene_object(scene, scene_object))
					{
						return_code = 1;
					}
					else
					{
						GET_NAME(Scene)(scene, &scene_name);
						display_message(ERROR_MESSAGE, "execute_command_gfx_erase.  "
							"Could not erase '%s' from scene '%s'",
							scene_object_name, scene_name);
						DEALLOCATE(scene_name);
						return_code = 0;
					}
				}
				else
				{
					GET_NAME(Scene)(scene, &scene_name);
					display_message(ERROR_MESSAGE,
						"gfx erase:  No object named '%s' in scene '%s'",
						scene_object_name, scene_name);
					DEALLOCATE(scene_name);
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx erase:  Must specify an object and a scene to erase it from");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (scene)
		{
			DEACCESS(Scene)(&scene);
		}
		if (scene_object_name)
		{
			DEALLOCATE(scene_object_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_erase.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_erase */

static int gfx_export_alias(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EXPORT ALIAS command.
==============================================================================*/
{
#if defined (NEW_ALIAS)
	char destroy_when_saved,*default_filename="cmgui_wire",*file_name,
		*retrieve_filename,save_now,write_sdl;
	float frame_in,frame_out,view_frame;
#endif /* defined (NEW_ALIAS) */
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	static struct Modifier_entry option_table[]=
	{
#if defined (NEW_ALIAS)
		{"dont_save_now",NULL,NULL,unset_char_flag},
		{"frame_in",NULL,NULL,set_float},
		{"frame_out",NULL,NULL,set_float},
		{"keep",NULL,NULL,unset_char_flag},
		{"retrieve",NULL,NULL,set_file_name},
#endif /* defined (NEW_ALIAS) */
		{"scene",NULL,NULL,set_Scene},
#if defined (NEW_ALIAS)
		{"sdl",NULL,NULL,set_char_flag},
		{"viewframe",NULL,NULL,set_float},
		{NULL,NULL,NULL,set_file_name}
#else /* defined (NEW_ALIAS) */
		{NULL,NULL,NULL,NULL}
#endif /* defined (NEW_ALIAS) */
	};

	ENTER(gfx_export_alias);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
#if defined (NEW_ALIAS)
			file_name=(char *)NULL;
			frame_in=0;
			frame_out=0;
			retrieve_filename=(char *)NULL;
			save_now=1;
			view_frame=0;
			write_sdl=0;
			destroy_when_saved=1;
#endif /* defined (NEW_ALIAS) */
			scene=ACCESS(Scene)(command_data->default_scene);
#if defined (NEW_ALIAS)
			(option_table[0]).to_be_modified= &save_now;
			(option_table[1]).to_be_modified= &frame_in;
			(option_table[2]).to_be_modified= &frame_out;
			(option_table[3]).to_be_modified= &destroy_when_saved;
			(option_table[4]).to_be_modified= &retrieve_filename;
			(option_table[5]).to_be_modified= &scene;
			(option_table[5]).user_data=command_data->scene_manager;
			(option_table[6]).to_be_modified= &write_sdl;
			(option_table[7]).to_be_modified= &view_frame;
			(option_table[8]).to_be_modified= &file_name;
#else /* defined (NEW_ALIAS) */
			(option_table[0]).to_be_modified= &scene;
			(option_table[0]).user_data=command_data->scene_manager;
#endif /* defined (NEW_ALIAS) */
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (scene)
				{
#if defined (NEW_ALIAS)
					if (!file_name)
					{
						file_name=default_filename;
					}
					if (write_sdl)
					{
						export_to_alias_sdl(scene,file_name,retrieve_filename,view_frame);
					}
					else
					{
						export_to_alias_frames(scene,file_name,frame_in,frame_out,save_now,
							destroy_when_saved);
					}
#else /* defined (NEW_ALIAS) */
					display_message(ERROR_MESSAGE,"gfx_export_alias.  The old gfx export alias is superseeded by gfx export wavefront");
					return_code=0;
#endif /* defined (NEW_ALIAS) */
				}
			} /* parse error,help */
			if (scene)
			{
				DEACCESS(Scene)(&scene);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_export_alias.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_alias.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_alias */

static int gfx_export_cm(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 22 March 2006

DESCRIPTION :
Executes a GFX EXPORT CM command.
==============================================================================*/
{
	char *ipbase_filename, *ipcoor_filename, *ipelem_filename, *ipnode_filename,
		*ipmap_filename, *region_path; 
	FILE *ipbase_file, *ipcoor_file, *ipelem_file, *ipmap_file, *ipnode_file;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct FE_field *coordinate_field;
	struct Option_table *option_table;
	struct Set_FE_field_conditional_FE_region_data coordinate_field_data;

	ENTER(gfx_export_cm);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		coordinate_field = (struct FE_field *)NULL;
		ipbase_filename = (char *)NULL;
		ipcoor_filename = (char *)NULL;
		ipelem_filename = (char *)NULL;
		ipmap_filename = (char *)NULL;
		ipnode_filename = (char *)NULL;
		region_path = (char *)NULL;

		option_table = CREATE(Option_table)();
		/* field */
		coordinate_field_data.conditional_function =
			(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL;
		coordinate_field_data.user_data = (void *)NULL;
		coordinate_field_data.fe_region =
			Cmiss_region_get_FE_region(command_data->root_region);
		Option_table_add_entry(option_table, "field",
			&coordinate_field,
			(void *)&coordinate_field_data,
			set_FE_field_conditional_FE_region);
		/* ipcoor_filename */
		Option_table_add_entry(option_table, "ipcoor_filename", &ipcoor_filename,
			NULL, set_name);
		/* ipbase_filename */
		Option_table_add_entry(option_table, "ipbase_filename", &ipbase_filename,
			NULL, set_name);
		/* ipmap_filename */
		Option_table_add_entry(option_table, "ipmap_filename", &ipmap_filename,
			NULL, set_name);
		/* ipnode_filename */
		Option_table_add_entry(option_table, "ipnode_filename", &ipnode_filename,
			NULL, set_name);
		/* ipelem_filename */
		Option_table_add_entry(option_table, "ipelem_filename", &ipelem_filename,
			NULL, set_name);
		/* region */
		Option_table_add_entry(option_table, "region", &region_path,
			command_data->data_root_region, set_Cmiss_region_path);

		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (!region_path)
			{
				display_message(ERROR_MESSAGE, "You must specify a region to export.");
				return_code = 0;
			}
			if (!(ipcoor_filename && ipbase_filename && ipnode_filename && ipelem_filename))
			{
				display_message(ERROR_MESSAGE,
					"You must specify all of ipcoor_filename, ipbase_filename, ipnode_filename and ipelem_filename.");
				return_code = 0;
			}
			if (!coordinate_field)
			{
				display_message(ERROR_MESSAGE,
					"You must specify an FE_field as the coordinate field to export.");
				return_code = 0;
			}
			if (return_code)
			{
				if (!(ipcoor_file = fopen(ipcoor_filename, "w")))
				{
					display_message(ERROR_MESSAGE,
						"Unable to open ipcoor_filename %s.", ipcoor_filename);
					return_code = 0;
				}
				if (!(ipbase_file = fopen(ipbase_filename, "w")))
				{
					display_message(ERROR_MESSAGE,
						"Unable to open ipbase_filename %s.", ipbase_filename);
					return_code = 0;
				}
				if (!(ipnode_file = fopen(ipnode_filename, "w")))
				{
					display_message(ERROR_MESSAGE,
						"Unable to open ipnode_filename %s.", ipnode_filename);
					return_code = 0;
				}
				if (!(ipelem_file = fopen(ipelem_filename, "w")))
				{
					display_message(ERROR_MESSAGE,
						"Unable to open ipelem_filename %s.", ipelem_filename);
					return_code = 0;
				}
				if (ipmap_filename)
				{
					if (!(ipmap_file = fopen(ipmap_filename, "w")))
					{
						display_message(ERROR_MESSAGE,
							"Unable to open ipmap_filename %s.", ipmap_filename);
						return_code = 0;
					}
				}
				else
				{
					ipmap_file = (FILE *)NULL;
				}
			}
			if (return_code)
			{
				write_cm_files(ipcoor_file, ipbase_file,
					ipnode_file, ipelem_file, ipmap_file,
					command_data->root_region, region_path,
					coordinate_field);
				fclose(ipcoor_file);
				fclose(ipbase_file);
				fclose(ipnode_file);
				fclose(ipelem_file);
				if (ipmap_file)
				{
					fclose(ipmap_file);
				}
			}
		}
		DESTROY(Option_table)(&option_table);
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
		if (ipbase_filename)
		{
			DEALLOCATE(ipbase_filename);
		}
		if (ipcoor_filename)
		{
			DEALLOCATE(ipcoor_filename);
		}
		if (ipelem_filename)
		{
			DEALLOCATE(ipelem_filename);
		}
		if (ipmap_filename)
		{
			DEALLOCATE(ipmap_filename);
		}
		if (ipnode_filename)
		{
			DEALLOCATE(ipnode_filename);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_export_cm.  Invalid argument(s)");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* gfx_export_cm */

static int gfx_export_iges(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 March 2003

DESCRIPTION :
Executes a GFX EXPORT IGES command.
==============================================================================*/
{
	char *file_name, *region_path;
	int return_code;
	struct Computed_field *coordinate_field;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct FE_region *fe_region;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data;

	ENTER(gfx_export_iges);
	USE_PARAMETER(dummy_to_be_modified);
	return_code = 0;
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		return_code=1;
		/* initialize defaults */
		Cmiss_region_get_root_region_path(&region_path);
		coordinate_field = (struct Computed_field *)NULL;
		file_name = (char *)NULL;

		option_table = CREATE(Option_table)();
		/* coordinate_field */
		set_coordinate_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_coordinate_field_data.conditional_function=
			Computed_field_has_3_components;
		set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"coordinate_field",&coordinate_field,
			&set_coordinate_field_data,set_Computed_field_conditional);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* default option: file name */
		Option_table_add_entry(option_table,(char *)NULL,&file_name,NULL,
			set_name);
		/* no errors, not asking for help */
		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (Cmiss_region_get_region_from_path(command_data->root_region,
				region_path, &region) &&
				(fe_region = Cmiss_region_get_FE_region(region)))
			{
				if (!coordinate_field)
				{
					display_message(WARNING_MESSAGE, "Must specify a coordinate field");
					return_code = 0;
				}
				if (return_code)
				{
					if (!file_name)
					{
						file_name = confirmation_get_write_filename(".igs",
							 command_data->user_interface
#if defined (WX_USER_INTERFACE)
							 , command_data->execute_command
#endif /* defined (WX_USER_INTERFACE)*/
);
					}
					if (file_name)
					{
						if (return_code = check_suffix(&file_name,".igs"))
						{
							return_code = export_to_iges(file_name, fe_region, region_path,
								coordinate_field);
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "gfx_export_iges.  Invalid region");
			}
		} /* parse error,help */
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		DESTROY(Option_table)(&option_table);
		DEALLOCATE(region_path);
		DEALLOCATE(file_name);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_export_iges.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_iges */

static int gfx_export_vrml(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EXPORT VRML command.
==============================================================================*/
{
	char *file_name;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"file",NULL,(void *)1,set_name},
		{"scene",NULL,NULL,set_Scene_including_sub_objects},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Scene *scene;

	ENTER(gfx_export_vrml);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			file_name=(char *)NULL;
			scene=ACCESS(Scene)(command_data->default_scene);
			(option_table[0]).to_be_modified= &file_name;
			(option_table[1]).to_be_modified= &scene;
			(option_table[1]).user_data=command_data->scene_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (!file_name)
				{
					file_name=confirmation_get_write_filename(".wrl",
						 command_data->user_interface
#if defined (WX_USER_INTERFACE)
							 , command_data->execute_command
#endif /* defined (WX_USER_INTERFACE)*/
																										);
				}
				if (file_name)
				{
					if (return_code=check_suffix(&file_name,".wrl"))
					{
						return_code=export_to_vrml(file_name,scene);
					}
				}
			} /* parse error,help */
			DEACCESS(Scene)(&scene);
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_export_vrml.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_vrml.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_vrml */

static int gfx_export_wavefront(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EXPORT WAVEFRONT command.
==============================================================================*/
{
	char *file_name,full_comments,*scene_object_name,*temp_filename;
	int frame_number, number_of_frames, return_code, version;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	struct Scene_object *scene_object;
	static struct Modifier_entry option_table[]=
	{
		{"file",NULL,(void *)1,set_name},
		{"frame_number",NULL,NULL,set_int_non_negative},
		{"full_comments",NULL,NULL,set_char_flag},
		{"graphics_object",NULL,(void *)1,set_name},
 		{"number_of_frames",NULL,NULL,set_int_positive},
		{"scene",NULL,NULL,set_Scene_including_sub_objects},
 		{"version",NULL,NULL,set_int_positive},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_export_wavefront);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			file_name=(char *)NULL;
			frame_number = 0;
			full_comments=0;
 			number_of_frames=100;
			scene=ACCESS(Scene)(command_data->default_scene);
			scene_object_name=(char *)NULL;
 			version=3;
			(option_table[0]).to_be_modified= &file_name;
			(option_table[1]).to_be_modified= &frame_number;
			(option_table[2]).to_be_modified= &full_comments;
			(option_table[3]).to_be_modified= &scene_object_name;
 			(option_table[4]).to_be_modified= &number_of_frames;
			(option_table[5]).to_be_modified= &scene;
			(option_table[5]).user_data=command_data->scene_manager;
 			(option_table[6]).to_be_modified= &version;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (scene_object_name)
				{
					if (!(scene_object=Scene_get_Scene_object_by_name(scene,
						scene_object_name)))
					{
						display_message(ERROR_MESSAGE,
							"gfx_export_wavefront.  Unable to find object '%s' in scene",
							scene_object_name);
						return_code=0;						
					}
				}
				else
				{
					scene_object=(struct Scene_object *)NULL;
				}
			}
			if (return_code)
			{
				if (!file_name)
				{
					if (scene_object_name)
					{
						if (ALLOCATE(file_name,char,strlen(scene_object_name)+5))
						{
							sprintf(file_name,"%s.obj",scene_object_name);
						}
					}
					else
					{
						if (GET_NAME(Scene)(scene,&file_name))
						{
							if (REALLOCATE(temp_filename,file_name,char,strlen(file_name)+5))
							{
								file_name=temp_filename;
								strcat(file_name,".obj");
							}
						}
					}
				}
				if (file_name)
				{
					return_code=export_to_wavefront(file_name,scene,scene_object,
						full_comments);
				}
			} /* parse error,help */
			DEACCESS(Scene)(&scene);
			if (scene_object_name)
			{
				DEALLOCATE(scene_object_name);
			}
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_export_wavefront.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_wavefront.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_wavefront */

static int execute_command_gfx_export(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 March 2003

DESCRIPTION :
Executes a GFX EXPORT command.
==============================================================================*/
{
	int return_code;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_export);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && command_data_void)
	{
		option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table,"alias",NULL,
			command_data_void, gfx_export_alias);
		Option_table_add_entry(option_table,"cm",NULL,
			command_data_void, gfx_export_cm);
		Option_table_add_entry(option_table,"iges",NULL,
			command_data_void, gfx_export_iges);
#if defined (OLD_CODE)
		Option_table_add_entry(option_table,"node",NULL,
			command_data_void, gfx_export_node);
#endif /* defined (OLD_CODE) */
		Option_table_add_entry(option_table,"vrml",NULL,
			command_data_void, gfx_export_vrml);
		Option_table_add_entry(option_table,"wavefront",NULL,
			command_data_void, gfx_export_wavefront);
		return_code = Option_table_parse(option_table,state);
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_export.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_export */

int gfx_evaluate(struct Parse_state *state, void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 March 2003

DESCRIPTION :
==============================================================================*/
{
	char *data_region_path, *element_region_path, *node_region_path,
		selected_flag;
	FE_value time;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct Computed_field *destination_field, *source_field;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct FE_element_selection *element_selection;
	struct FE_node_selection *data_selection, *node_selection;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_destination_field_data,
		set_source_field_data;

	ENTER(gfx_evaluate);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		data_region_path = (char *)NULL;
		element_region_path = (char *)NULL;
		node_region_path = (char *)NULL;
		selected_flag = 0;
		destination_field = (struct Computed_field *)NULL;
		source_field = (struct Computed_field *)NULL;
		if (command_data->default_time_keeper)
		{
			time = Time_keeper_get_time(command_data->default_time_keeper);
		}
		else
		{
			time = 0;
		}
		
		option_table = CREATE(Option_table)();
		/* destination */
		set_destination_field_data.conditional_function =
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_destination_field_data.conditional_function_user_data = (void *)NULL;
		set_destination_field_data.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		Option_table_add_entry(option_table, "destination", &destination_field,
			&set_destination_field_data, set_Computed_field_conditional);
		/* dgroup */
		Option_table_add_entry(option_table, "dgroup", &data_region_path,
			command_data->data_root_region, set_Cmiss_region_path);
		/* egroup */
		Option_table_add_entry(option_table, "egroup", &element_region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* ngroup */
		Option_table_add_entry(option_table, "ngroup", &node_region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* selected */
		Option_table_add_entry(option_table, "selected", &selected_flag,
			NULL, set_char_flag);
		/* source */
		set_source_field_data.conditional_function =
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		set_source_field_data.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		Option_table_add_entry(option_table, "source", &source_field,
			&set_source_field_data, set_Computed_field_conditional);

		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (destination_field && source_field)
			{
				if (selected_flag)
				{
					data_selection = command_data->data_selection;
					element_point_ranges_selection =
						command_data->element_point_ranges_selection;
					element_selection = command_data->element_selection;
					node_selection = command_data->node_selection;
				}
				else
				{
					data_selection = (struct FE_node_selection *)NULL;
					element_point_ranges_selection =
						(struct Element_point_ranges_selection *)NULL;
					element_selection = (struct FE_element_selection *)NULL;
					node_selection = (struct FE_node_selection *)NULL;
				}

				if (data_region_path && (!element_region_path) && (!node_region_path))
				{
					if (Cmiss_region_get_region_from_path(command_data->data_root_region,
						data_region_path, &region))
					{
						Computed_field_update_nodal_values_from_source(
							destination_field, source_field, region, data_selection, time);
					}
				}
				else if (element_region_path && (!data_region_path) &&
					(!node_region_path))
				{
					if (Cmiss_region_get_region_from_path(command_data->root_region,
						element_region_path, &region))
					{
						Computed_field_update_element_values_from_source(
							destination_field, source_field, region,
							element_point_ranges_selection, element_selection, time);
					}
				}
				else if (node_region_path && (!data_region_path) &&
					(!element_region_path))
				{
					if (Cmiss_region_get_region_from_path(command_data->root_region,
						node_region_path, &region))
					{
						Computed_field_update_nodal_values_from_source(
							destination_field, source_field, region, node_selection, time);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_evaluate.  Must specify one of dgroup/egroup/ngroup");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_evaluate.  Must specify destination and source fields");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (data_region_path)
		{
			DEALLOCATE(data_region_path);
		}
		if (element_region_path)
		{
			DEALLOCATE(element_region_path);
		}
		if (node_region_path)
		{
			DEALLOCATE(node_region_path);
		}
		if (source_field)
		{
			DEACCESS(Computed_field)(&source_field);
		}
		if (destination_field)
		{
			DEACCESS(Computed_field)(&destination_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_evaluate.  Invalid argument(s)");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* gfx_evaluate */

#if defined (OLD_CODE)
static int execute_command_gfx_filter(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 26 June 1997

DESCRIPTION :
Executes a GFX FILTER command.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"node",NULL,NULL,gfx_filter_node},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_gfx_filter);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			(option_table[0]).user_data=(void *)(command_data->node_group_manager);
			return_code=process_option(state,option_table);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_filter.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_filter.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_filter */
#endif /* defined (OLD_CODE) */

static int gfx_list_all_commands(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Executes a GFX LIST WINDOW.
==============================================================================*/
{
	 int return_code;
	 static char	*command_prefix, *current_token;
	 struct Cmiss_command_data *command_data;
	 struct MANAGER(Graphical_material) *graphical_material_manager;
	 struct MANAGER(Computed_field) *computed_field_manager;
	 struct LIST(Computed_field) *list_of_fields;
	 struct List_Computed_field_commands_data list_commands_data;
	 ENTER(gfx_list_all_commands);
	 USE_PARAMETER(dummy_to_be_modified);
	 if (state)
	 { 
			if (current_token=state->current_token)
			{
				 if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				 {
				 }
				 else
				 {
						display_message(INFORMATION_MESSAGE," [ALL]");
						return_code=1;
				 }
			}
			else
			{
				 if	(command_data = (struct Cmiss_command_data *)command_data_void)
				 {

						/* Command of computed_field */
						if (command_data->computed_field_package && (computed_field_manager=
									Computed_field_package_get_computed_field_manager(
										 command_data->computed_field_package)))
						{
							 if (list_of_fields = CREATE(LIST(Computed_field))())
							 {
									command_prefix="gfx define field ";
									list_commands_data.command_prefix = command_prefix;
									list_commands_data.listed_fields = 0;
									list_commands_data.computed_field_list = list_of_fields;
									list_commands_data.computed_field_manager =
										 computed_field_manager;
									while (FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
														list_Computed_field_commands_if_managed_source_fields_in_list,
														(void *)&list_commands_data, computed_field_manager) &&
										 (0 != list_commands_data.listed_fields))
									{
										 list_commands_data.listed_fields = 0;
									}			 
									DESTROY(LIST(Computed_field))(&list_of_fields);
									return_code=1;					
							 } 
							 else
							 {
									return_code=0;
							 }
							 if (!return_code)
							 {
									display_message(ERROR_MESSAGE,
										 "gfx_list_Computed_field.  Could not list field commands");
							 }
						}
						/* Commands of spectrum */
						if (command_data->spectrum_manager)
						{
							 FOR_EACH_OBJECT_IN_MANAGER(Spectrum)(
									for_each_spectrum_list_or_write_commands, (void *)"false", command_data->spectrum_manager);			
						}
						/* Command of graphical_material */
						if (graphical_material_manager =
							 Material_package_get_material_manager(command_data->material_package))
						{
							 command_prefix="gfx create material ";
							 return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphical_material)(
									list_Graphical_material_commands,(void *)command_prefix, 
									graphical_material_manager);
						}	
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Scene)(
							 for_each_graphics_object_in_scene_get_command_list, (void*) "false",
							 command_data->scene_manager);
						/* Command of graphics window */
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
							 list_Graphics_window_commands,(void *)NULL,
							 command_data->graphics_window_manager);
#endif /*defined (USE_CMGUI_GRAPHICS_WINDOW)*/
				 }
				 
			}
	 }
	 LEAVE;
	 
	 return (return_code);
}/* gfx_list_all_commands */
	 
static int gfx_list_environment_map(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 June 1996

DESCRIPTION :
Executes a GFX LIST ENVIRONMENT_MAP.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Environment_map *environment_map;

	ENTER(gfx_list_environment_map);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (command_data=(struct Cmiss_command_data *)command_data_void)
				{
					if (environment_map=FIND_BY_IDENTIFIER_IN_MANAGER(Environment_map,
						name)(current_token,command_data->environment_map_manager))
					{
						return_code=list_Environment_map(environment_map);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown environment map: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_list_environment_map.  Missing command_data");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," ENVIRONMENT_MAP_NAME");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing environment map name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_environment_map.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_environment_map */

static int gfx_list_Computed_field(struct Parse_state *state,
	void *dummy_to_be_modified,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 14 December 2001

DESCRIPTION :
Executes a GFX LIST FIELD.
???RC Could be moved to computed_field.c.
==============================================================================*/
{
	static char	*command_prefix="gfx define field ";
	char commands_flag;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,NULL,set_Computed_field_conditional},
		{NULL,NULL,NULL,set_Computed_field_conditional}
	};
	struct Computed_field *computed_field;
	struct Computed_field_package *computed_field_package;
	struct List_Computed_field_commands_data list_commands_data;
	struct LIST(Computed_field) *list_of_fields;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(gfx_list_Computed_field);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((computed_field_package=
			(struct Computed_field_package *)computed_field_package_void)&&
			(computed_field_manager=Computed_field_package_get_computed_field_manager(
				computed_field_package)))
		{
			commands_flag=0;
			/* if no computed_field specified, list all computed_fields */
			computed_field=(struct Computed_field *)NULL;
			set_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_field_data.conditional_function_user_data=(void *)NULL;
			set_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
					computed_field_package);
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &computed_field;
			(option_table[1]).user_data= &set_field_data;
			(option_table[2]).to_be_modified= &computed_field;
			(option_table[2]).user_data= &set_field_data;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (commands_flag)
				{
					if (computed_field)
					{
						return_code=list_Computed_field_commands(computed_field,
							(void *)command_prefix);
					}
					else
					{
						if (list_of_fields = CREATE(LIST(Computed_field))())
						{
							list_commands_data.command_prefix = command_prefix;
							list_commands_data.listed_fields = 0;
							list_commands_data.computed_field_list = list_of_fields;
							list_commands_data.computed_field_manager =
								computed_field_manager;
							while (FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
								list_Computed_field_commands_if_managed_source_fields_in_list,
								(void *)&list_commands_data, computed_field_manager) &&
								(0 != list_commands_data.listed_fields))
							{
								list_commands_data.listed_fields = 0;
							}
							DESTROY(LIST(Computed_field))(&list_of_fields);
						}
						else
						{
							return_code=0;
						}
						if (!return_code)
						{
							display_message(ERROR_MESSAGE,
								"gfx_list_Computed_field.  Could not list field commands");
						}
					}
				}
				else
				{
					if (computed_field)
					{
						return_code=list_Computed_field(computed_field,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
							list_Computed_field_name,(void *)NULL,computed_field_manager);
					}
				}
			}
			/* must deaccess computed_field since accessed by set_Computed_field */
			if (computed_field)
			{
				DEACCESS(Computed_field)(&computed_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_list_Computed_field.  "
				"Missing computed_field_package_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_Computed_field.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_Computed_field */

static int gfx_list_FE_element(struct Parse_state *state,
	void *cm_element_type_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 March 2003

DESCRIPTION :
Executes a GFX LIST ELEMENT.
==============================================================================*/
{
	char all_flag, *region_path, selected_flag, verbose_flag;
	enum CM_element_type cm_element_type;
	int return_code, start, stop;
	struct CM_element_type_Multi_range_data element_type_ranges_data;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct FE_region *fe_region;
	struct LIST(FE_element) *element_list;
	struct Multi_range *element_ranges;
	struct Option_table *option_table;

	ENTER(gfx_list_FE_element);
	cm_element_type = (enum CM_element_type)cm_element_type_void;
	if (state && ((CM_ELEMENT == cm_element_type) ||
		(CM_FACE == cm_element_type) || (CM_LINE == cm_element_type)) &&
		(command_data = (struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		/* region_path defaults to NULL so that we have to either specify "all"
			 or group "/" to destroy all elements */
		all_flag = 0;
		Cmiss_region_get_root_region_path(&region_path);
		selected_flag = 0;
		verbose_flag = 0;
		element_ranges = CREATE(Multi_range)();

		option_table=CREATE(Option_table)();
		/* all */
		Option_table_add_entry(option_table, "all", &all_flag, NULL, set_char_flag);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* selected */
		Option_table_add_entry(option_table, "selected", &selected_flag,
			NULL, set_char_flag);
		/* verbose */
		Option_table_add_entry(option_table, "verbose", &verbose_flag,
			NULL, set_char_flag);
		/* default option: element number ranges */
		Option_table_add_entry(option_table, (char *)NULL, (void *)element_ranges,
			NULL, set_Multi_range);
		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (Cmiss_region_get_region_from_path(command_data->root_region,
				region_path, &region) &&
				(fe_region = Cmiss_region_get_FE_region(region)))
			{
				if (element_list =
					FE_element_list_from_fe_region_selection_ranges_condition(
						fe_region, cm_element_type, command_data->element_selection,
						selected_flag, element_ranges,
						/* conditional_field */(struct Computed_field *)NULL, /*time*/0))
				{
					if (0 < NUMBER_IN_LIST(FE_element)(element_list))
					{
						/* always write verbose details if single element asked for and
							 neither all_flag nor selected_flag nor element_group set */
						if (verbose_flag || ((!all_flag) && (!selected_flag) &&
							(1 == Multi_range_get_number_of_ranges(element_ranges)) &&
							(Multi_range_get_range(element_ranges, /*range_number*/0, 
								&start, &stop)) && (start == stop)))
						{
							return_code = FOR_EACH_OBJECT_IN_LIST(FE_element)(list_FE_element,
								(void *)NULL, element_list);
						}
						else
						{
							/* write comma separated list of ranges - clear and use existing
								 element_ranges structure */
							switch (cm_element_type)
							{
								case CM_ELEMENT:
								{
									display_message(INFORMATION_MESSAGE,"Elements:\n");
								} break;
								case CM_FACE:
								{
									display_message(INFORMATION_MESSAGE,"Faces:\n");
								} break;
								case CM_LINE:
								{
									display_message(INFORMATION_MESSAGE,"Lines:\n");
								} break;
							}
							Multi_range_clear(element_ranges);
							element_type_ranges_data.cm_element_type = cm_element_type;
							element_type_ranges_data.multi_range = element_ranges;
							if (FOR_EACH_OBJECT_IN_LIST(FE_element)(
								FE_element_of_CM_element_type_add_number_to_Multi_range,
								(void *)&element_type_ranges_data, element_list))
							{
								return_code = Multi_range_display_ranges(element_ranges);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_list_FE_element.  Could not get element ranges");
								return_code = 0;
							}
							display_message(INFORMATION_MESSAGE,"Total number = %d\n",
								NUMBER_IN_LIST(FE_element)(element_list));
						}
					}
					else
					{
						switch (cm_element_type)
						{
							case CM_ELEMENT:
							{
								display_message(INFORMATION_MESSAGE,
									"gfx list elements:  No elements specified\n");
							} break;
							case CM_FACE:
							{
								display_message(INFORMATION_MESSAGE,
									"gfx list faces:  No faces specified\n");
							} break;
							case CM_LINE:
							{
								display_message(INFORMATION_MESSAGE,
									"gfx list lines:  No lines specified\n");
							} break;
						}
					}
					DESTROY(LIST(FE_element))(&element_list);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_list_FE_element.  Could not make element_list");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "gfx_list_FE_element.  Invalid region");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		DEALLOCATE(region_path);
		DESTROY(Multi_range)(&element_ranges);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_list_FE_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_FE_element */

static int gfx_list_FE_node(struct Parse_state *state,
	void *use_data,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 May 2003

DESCRIPTION :
Executes a GFX LIST NODES.
If <use_data> is set, use data_manager and data_selection, otherwise
use node_manager and node_selection.
==============================================================================*/
{
	char all_flag, *region_path, selected_flag, verbose_flag;
	int return_code, start, stop;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region, *root_region;
	struct FE_node_selection *node_selection;
	struct FE_region *fe_region;
	struct LIST(FE_node) *node_list;
	struct Multi_range *node_ranges;
	struct Option_table *option_table;

	ENTER(gfx_list_FE_node);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (use_data)
		{
			root_region = command_data->data_root_region;
			node_selection = command_data->data_selection;
		}
		else
		{
			root_region = command_data->root_region;
			node_selection = command_data->node_selection;
		}
		/* initialise defaults */
		all_flag = 0;
		Cmiss_region_get_root_region_path(&region_path);
		selected_flag = 0;
		verbose_flag = 0;
		node_ranges = CREATE(Multi_range)();

		option_table = CREATE(Option_table)();
		/* all */
		Option_table_add_entry(option_table, "all", &all_flag, NULL, set_char_flag);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			root_region, set_Cmiss_region_path);
		/* selected */
		Option_table_add_entry(option_table, "selected", &selected_flag,
			NULL, set_char_flag);
		/* verbose */
		Option_table_add_entry(option_table, "verbose", &verbose_flag,
			NULL, set_char_flag);
		/* default option: node number ranges */
		Option_table_add_entry(option_table, (char *)NULL, (void *)node_ranges,
			NULL, set_Multi_range);
		if (return_code = Option_table_multi_parse(option_table, state))
		{
			if (Cmiss_region_get_region_from_path(root_region, region_path, &region)
				&& (fe_region = Cmiss_region_get_FE_region(region)))
			{
				if (node_list = FE_node_list_from_fe_region_selection_ranges_condition(
					fe_region, node_selection, selected_flag, node_ranges,
					/* conditional_field */(struct Computed_field *)NULL, /*time*/0))
				{
					if (0 < NUMBER_IN_LIST(FE_node)(node_list))
					{
						/* always write verbose details if single node asked for and
							 neither all_flag nor selected_flag nor region set */
						if (verbose_flag || ((!all_flag) && (!selected_flag) &&
							(1 == Multi_range_get_number_of_ranges(node_ranges)) &&
							(Multi_range_get_range(node_ranges, /*range_number*/0, 
								&start, &stop)) && (start == stop)))
						{
							return_code = FOR_EACH_OBJECT_IN_LIST(FE_node)(list_FE_node,
								(void *)1, node_list);
						}
						else
						{
							if (use_data)
							{
								display_message(INFORMATION_MESSAGE,"Data:\n");
							}
							else
							{
								display_message(INFORMATION_MESSAGE,"Nodes:\n");
							}
							/* write comma separated list of ranges - use existing node
								 ranges structure */
							Multi_range_clear(node_ranges);
							if (FOR_EACH_OBJECT_IN_LIST(FE_node)(
								add_FE_node_number_to_Multi_range,(void *)node_ranges,
								node_list))
							{
								return_code=Multi_range_display_ranges(node_ranges);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_list_FE_node.  Could not get node ranges");
								return_code=0;
							}
							display_message(INFORMATION_MESSAGE,"Total number = %d\n",
								NUMBER_IN_LIST(FE_node)(node_list));
						}
					}
					else
					{
						if (use_data)
						{
							display_message(INFORMATION_MESSAGE,
								"gfx list data:  No data specified\n");
						}
						else
						{
							display_message(INFORMATION_MESSAGE,
								"gfx list nodes:  No nodes specified\n");
						}
					}
					DESTROY(LIST(FE_node))(&node_list);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_list_FE_node.  Could not make node_list");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "gfx_list_FE_node.  Invalid region");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		DEALLOCATE(region_path);
		DESTROY(Multi_range)(&node_ranges);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_FE_node.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_FE_node */

static int gfx_list_graphical_material(struct Parse_state *state,
	void *dummy_to_be_modified,void *graphical_material_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 September 1998

DESCRIPTION :
Executes a GFX LIST MATERIAL.
???RC Could be moved to material.c.
==============================================================================*/
{
	static char	*command_prefix="gfx create material ";
	char commands_flag;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,NULL,set_Graphical_material},
		{NULL,NULL,NULL,set_Graphical_material}
	};
	struct Graphical_material *material;
	struct MANAGER(Graphical_material) *graphical_material_manager;

	ENTER(gfx_list_graphical_material);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (graphical_material_manager=
			(struct MANAGER(Graphical_material) *)graphical_material_manager_void)
		{
			commands_flag=0;
			/* if no material specified, list all materials */
			material=(struct Graphical_material *)NULL;
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &material;
			(option_table[1]).user_data= graphical_material_manager_void;
			(option_table[2]).to_be_modified= &material;
			(option_table[2]).user_data= graphical_material_manager_void;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (commands_flag)
				{
					if (material)
					{
						return_code=list_Graphical_material_commands(material,
							(void *)command_prefix);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphical_material)(
							list_Graphical_material_commands,(void *)command_prefix,
							graphical_material_manager);
					}
				}
				else
				{
					if (material)
					{
						return_code=list_Graphical_material(material,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphical_material)(
							list_Graphical_material,(void *)NULL,
							graphical_material_manager);
					}
				}
			}
			/* must deaccess material since accessed by set_Graphical_material */
			if (material)
			{
				DEACCESS(Graphical_material)(&material);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_list_graphical_material.  "
				"Missing graphical_material_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_graphical_material.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_graphical_material */

static int gfx_list_region(struct Parse_state *state,
	void *dummy_to_be_modified, void *root_region_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
Executes a GFX LIST REGION command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_region *region, *root_region;

	ENTER(gfx_list_region);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (root_region = (struct Cmiss_region *)root_region_void))
	{
		return_code = 1;
		if ((current_token = state->current_token) &&
			((0 == strcmp(PARSER_HELP_STRING,current_token)) ||
				(0 == strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
		{
			/* help */
			display_message(INFORMATION_MESSAGE, " REGION_PATH");
		}
		else
		{
			region = (struct Cmiss_region *)NULL;
			if (current_token)
			{
				/* get region to be listed */
				if (Cmiss_region_get_region_from_path(root_region, current_token,
					&region))
				{
					display_message(INFORMATION_MESSAGE, "Region %s:\n", current_token);
				}
				else
				{
					display_message(ERROR_MESSAGE, "Unknown region: %s", current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				region = root_region;
				display_message(INFORMATION_MESSAGE,
					"Region " CMISS_REGION_PATH_SEPARATOR_STRING ":\n");
			}
			if (return_code)
			{
				return_code = Cmiss_region_list(region, 2, 2);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_list_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_region */

static int gfx_list_g_element(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 25 March 2003

DESCRIPTION :
Executes a GFX LIST G_ELEMENT.
==============================================================================*/
{
	char commands_flag, *command_prefix, *command_suffix, *region_path, *scene_name;
	int error, return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct GT_element_group *gt_element_group;
	struct Option_table *option_table;
	struct Scene *scene;

	ENTER(gfx_list_g_element);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		commands_flag = 0;
		region_path = (char *)NULL;
		scene = ACCESS(Scene)(command_data->default_scene);

		option_table = CREATE(Option_table)();
		/* commands */
		Option_table_add_entry(option_table, "commands", &commands_flag,
			NULL, set_char_flag);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* scene */
		Option_table_add_entry(option_table, "scene", &scene,
			command_data->scene_manager, set_Scene);
		/* default option: group name */
		Option_table_add_entry(option_table, (char *)NULL, &region_path,
			command_data->root_region, set_Cmiss_region_path);
		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (GET_NAME(Scene)(scene,&scene_name))
			{
				if (region_path && Cmiss_region_get_region_from_path(
					command_data->root_region, region_path, &region) &&
					(gt_element_group = Scene_get_graphical_element_group(scene, region)))
				{
					if (commands_flag)
					{
						error = 0;
						command_prefix = duplicate_string("gfx modify g_element ");
						make_valid_token(&region_path);
						append_string(&command_prefix, region_path, &error);
						append_string(&command_prefix, " ", &error);
						command_suffix = (char *)NULL;
						make_valid_token(&scene_name);
						if (scene != command_data->default_scene)
						{
							append_string(&command_suffix, " scene ", &error);
							append_string(&command_suffix, scene_name, &error);
						}
						append_string(&command_suffix, ";", &error);
						display_message(INFORMATION_MESSAGE,
							"Commands for reproducing group %s on scene %s:\n",
							region_path, scene_name);
						return_code = GT_element_group_list_commands(gt_element_group,
							command_prefix, command_suffix);
						DEALLOCATE(command_suffix);
						DEALLOCATE(command_prefix);
					}
					else
					{
						display_message(INFORMATION_MESSAGE,
							"Contents of group %s on scene %s:\n", region_path,
							scene_name);
						return_code=GT_element_group_list_contents(gt_element_group);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "gfx list g_element:  "
						"Must specify name of graphical element in scene %s\n", scene_name);
					return_code = 0;
				}
				DEALLOCATE(scene_name);
			}
		}
		DESTROY(Option_table)(&option_table);
		DEACCESS(Scene)(&scene);
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_list_g_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_g_element */

static int gfx_list_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *object_list_void)
/*******************************************************************************
LAST MODIFIED : 26 July 1998

DESCRIPTION :
Executes a GFX LIST GLYPH/GRAPHICS_OBJECT command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct GT_object *object;
	struct LIST(GT_object) *list;

	ENTER(gfx_list_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (list=(struct LIST(GT_object) *)object_list_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
						current_token,list))
					{
						return_code=GT_object_list_contents(object,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Could not find object named '%s'",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," OBJECT_NAME[ALL]");
					return_code=1;
				}
			}
			else
			{
				/* list contents of all objects in list */
				return_code=FOR_EACH_OBJECT_IN_LIST(GT_object)(
					GT_object_list_contents,(void *)NULL,list);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_graphics_object.  Missing graphics object list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_graphics_object.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_graphics_object */

static int gfx_list_grid_points(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 March 2003

DESCRIPTION :
Executes a GFX LIST GRID_POINTS.
If <used_data_flag> is set, use data_manager and data_selection, otherwise
use node_manager and node_selection.
==============================================================================*/
{
	char all_flag,ranges_flag,selected_flag;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Element_point_ranges_grid_to_multi_range_data grid_to_multi_range_data;
	struct FE_element_grid_to_multi_range_data element_grid_to_multi_range_data;
	struct FE_field *grid_field;
	struct FE_region *fe_region;
	struct Multi_range *grid_point_ranges,*multi_range;
	struct Option_table *option_table;
	struct Set_FE_field_conditional_FE_region_data set_grid_field_data;

	ENTER(gfx_list_grid_points);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		/* initialise defaults */
		all_flag=0;
		selected_flag=0;
		grid_point_ranges=CREATE(Multi_range)();
		fe_region = Cmiss_region_get_FE_region(command_data->root_region);

		if ((grid_field = FE_region_get_FE_field_from_name(fe_region,
			"grid_point_number")) &&
			FE_field_is_1_component_integer(grid_field,(void *)NULL))
		{
			ACCESS(FE_field)(grid_field);
		}
		else
		{
			grid_field=(struct FE_field *)NULL;
		}

		option_table=CREATE(Option_table)();
		/* all */
		Option_table_add_entry(option_table,"all",&all_flag,NULL,set_char_flag);
		/* grid_field */
		set_grid_field_data.conditional_function = FE_field_is_1_component_integer;
		set_grid_field_data.user_data = (void *)NULL;
		set_grid_field_data.fe_region = fe_region;
		Option_table_add_entry(option_table, "grid_field", &grid_field,
			(void *)&set_grid_field_data, set_FE_field_conditional_FE_region);
		/* selected */
		Option_table_add_entry(option_table,"selected",&selected_flag,
			NULL,set_char_flag);
		/* default option: grid point number ranges */
		Option_table_add_entry(option_table,(char *)NULL,(void *)grid_point_ranges,
			NULL,set_Multi_range);
		if (return_code=Option_table_multi_parse(option_table,state))
		{
			if (grid_field)
			{
				if (multi_range=CREATE(Multi_range)())
				{
					ranges_flag=(0<Multi_range_get_number_of_ranges(grid_point_ranges));
					if (selected_flag)
					{
						/* fill multi_range with selected grid_point_number ranges */
						grid_to_multi_range_data.grid_fe_field=grid_field;
						grid_to_multi_range_data.multi_range=multi_range;
						grid_to_multi_range_data.all_points_native=1;
						return_code=FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
							Element_point_ranges_grid_to_multi_range,
							(void *)&grid_to_multi_range_data,
							Element_point_ranges_selection_get_element_point_ranges_list(
								command_data->element_point_ranges_selection));
					}
					else if (ranges_flag||all_flag)
					{
						/* fill multi_range with all grid_point_number ranges */
						element_grid_to_multi_range_data.grid_fe_field=grid_field;
						element_grid_to_multi_range_data.multi_range=multi_range;
						return_code = FE_region_for_each_FE_element(fe_region,
							FE_element_grid_to_multi_range,
							(void *)&element_grid_to_multi_range_data);
					}
					if (return_code)
					{
						if (ranges_flag)
						{
							/* include in multi_range only values also in grid_point_ranges */
							Multi_range_intersect(multi_range,grid_point_ranges);
						}
						if (0<Multi_range_get_number_of_ranges(multi_range))
						{
							display_message(INFORMATION_MESSAGE,"Grid points:\n");
							return_code=Multi_range_display_ranges(multi_range);
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"gfx list grid_points:  No grid points specified");
						}
					}
					DESTROY(Multi_range)(&multi_range);
				}
				else
				{
					return_code=0;
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,"gfx_list_grid_points.  Failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"To list grid_points, "
					"need integer grid_field (eg. grid_point_number)");
				return_code=0;
			}
		}
		DESTROY(Option_table)(&option_table);
		DESTROY(Multi_range)(&grid_point_ranges);
		if (grid_field)
		{
			DEACCESS(FE_field)(&grid_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_grid_points.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_grid_points */

static int gfx_list_light(struct Parse_state *state,
	void *dummy_to_be_modified,void *light_manager_void)
/*******************************************************************************
LAST MODIFIED : 2 September 1996

DESCRIPTION :
Executes a GFX LIST LIGHT.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Light *light;
	struct MANAGER(Light) *light_manager;

	ENTER(gfx_list_light);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (light_manager=(struct MANAGER(Light) *)light_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (light=FIND_BY_IDENTIFIER_IN_MANAGER(Light,name)(current_token,
						light_manager))
					{
						return_code=list_Light(light,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown light: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," LIGHT_NAME");
					return_code=1;
				}
			}
			else
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Light)(list_Light,(void *)NULL,
					light_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_light.  Missing light_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_light.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_light */

static int gfx_list_light_model(struct Parse_state *state,
	void *dummy_to_be_modified,void *light_model_manager_void)
/*******************************************************************************
LAST MODIFIED : 3 September 1996

DESCRIPTION :
Executes a GFX LIST LMODEL.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Light_model *light_model;
	struct MANAGER(Light_model) *light_model_manager;

	ENTER(gfx_list_light_model);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (light_model_manager=
			(struct MANAGER(Light_model) *)light_model_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (light_model=FIND_BY_IDENTIFIER_IN_MANAGER(Light_model,name)(
						current_token,light_model_manager))
					{
						return_code=list_Light_model(light_model,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown light model: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," LIGHT_MODEL_NAME");
					return_code=1;
				}
			}
			else
			{
				FOR_EACH_OBJECT_IN_MANAGER(Light_model)(list_Light_model,(void *)NULL,
					light_model_manager);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_light_model.  Missing light_model_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_light_model.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_light_model */

static int gfx_list_scene(struct Parse_state *state,
	void *dummy_to_be_modified,void *scene_manager_void)
/*******************************************************************************
LAST MODIFIED : 21 September 1998

DESCRIPTION :
Executes a GFX LIST SCENE.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Scene *scene;
	struct MANAGER(Scene) *scene_manager;

	ENTER(gfx_list_scene);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (scene_manager=(struct MANAGER(Scene) *)scene_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (scene=FIND_BY_IDENTIFIER_IN_MANAGER(Scene,name)(current_token,
						scene_manager))
					{
						return_code=list_Scene(scene,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown scene: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," SCENE_NAME");
					return_code=1;
				}
			}
			else
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Scene)(list_Scene,(void *)NULL,
					scene_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_scene.  Missing scene_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_scene.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_scene */

static int gfx_list_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *spectrum_manager_void)
/*******************************************************************************
LAST MODIFIED : 28 November 2000

DESCRIPTION :
Executes a GFX LIST SPECTRUM.
==============================================================================*/
{
	static char	*command_prefix="gfx modify spectrum";
	char *commands_flag;
	int return_code;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct Spectrum *spectrum;
	struct Option_table *option_table;

	ENTER(gfx_list_spectrum);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (spectrum_manager =
		(struct MANAGER(Spectrum) *)spectrum_manager_void))
	{
		commands_flag = 0;
		spectrum = (struct Spectrum *)NULL;

		option_table=CREATE(Option_table)();
		/* commands */
		Option_table_add_entry(option_table, "commands", &commands_flag,
			NULL, set_char_flag);
		/* default option: spectrum name */
		Option_table_add_entry(option_table, (char *)NULL, &spectrum,
			spectrum_manager_void, set_Spectrum);
		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (commands_flag)
			{
				if (spectrum)
				{
					display_message(INFORMATION_MESSAGE,
						"Commands for reproducing spectrum:\n");
					return_code = Spectrum_list_commands(spectrum,
						command_prefix, (char *)NULL);
				}
				else
				{
					display_message(INFORMATION_MESSAGE," SPECTRUM_NAME\n");
					return_code = 1;
				}
			}
			else
			{
				if (spectrum)
				{
					return_code = Spectrum_list_contents(spectrum, (void *)NULL);
				}
				else
				{
					return_code = FOR_EACH_OBJECT_IN_MANAGER(Spectrum)(
						Spectrum_list_contents, (void *)NULL, spectrum_manager);
				}
			}
		}
		DESTROY(Option_table)(&option_table);
		if (spectrum)
		{
			DEACCESS(Spectrum)(&spectrum);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_spectrum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_spectrum */

static int gfx_list_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *texture_manager_void)
/*******************************************************************************
LAST MODIFIED : 19 May 1999

DESCRIPTION :
Executes a GFX LIST TEXTURE.
???RC Could be moved to texture.c.
==============================================================================*/
{
	static char	*command_prefix="gfx create texture ";
	char commands_flag;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,NULL,set_Texture},
		{NULL,NULL,NULL,set_Texture}
	};
	struct Texture *texture;
	struct MANAGER(Texture) *texture_manager;

	ENTER(gfx_list_texture);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (texture_manager=(struct MANAGER(Texture) *)texture_manager_void)
		{
			commands_flag=0;
			/* if no texture specified, list all textures */
			texture=(struct Texture *)NULL;
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &texture;
			(option_table[1]).user_data= texture_manager_void;
			(option_table[2]).to_be_modified= &texture;
			(option_table[2]).user_data= texture_manager_void;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (commands_flag)
				{
					if (texture)
					{
						return_code=list_Texture_commands(texture,(void *)command_prefix);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Texture)(
							list_Texture_commands,(void *)command_prefix,texture_manager);
					}
				}
				else
				{
					if (texture)
					{
						return_code=list_Texture(texture,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Texture)(
							list_Texture,(void *)NULL,texture_manager);
					}
				}
			}
			/* must deaccess texture since accessed by set_Texture */
			if (texture)
			{
				DEACCESS(Texture)(&texture);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_list_texture.  "
				"Missing texture_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_texture */

static int gfx_list_transformation(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2001

DESCRIPTION :
Executes a GFX LIST TRANSFORMATION.
==============================================================================*/
{
	char *command_prefix,commands_flag,*scene_name,*scene_object_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	struct Scene_object *scene_object;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,(void *)1,set_name},
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_list_transformation);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			commands_flag=0;
			scene_object_name=(char *)NULL;
			scene=ACCESS(Scene)(command_data->default_scene);
			/* parse the command line */
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &scene_object_name;
			(option_table[2]).to_be_modified= &scene;
			(option_table[2]).user_data=command_data->scene_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (GET_NAME(Scene)(scene, &scene_name))
				{
					if ((!scene_object_name)||(scene_object=
						Scene_get_Scene_object_by_name(scene,scene_object_name)))
					{
						if (commands_flag)
						{
							/* quote scene name if it contains special characters */
							make_valid_token(&scene_name);
							if (ALLOCATE(command_prefix, char, 40 + strlen(scene_name)))
							{
								sprintf(command_prefix, "gfx set transformation scene %s name",
									scene_name);
								if (scene_object_name)
								{
									return_code = list_Scene_object_transformation_commands(
										scene_object,(void *)command_prefix);
								}
								else
								{
									return_code = for_each_Scene_object_in_Scene(scene,
										list_Scene_object_transformation_commands,
										(void *)command_prefix);
								}
								DEALLOCATE(command_prefix);
							}
							else
							{
								return_code=0;
							}
						}
						else
						{
							if (scene_object_name)
							{
								return_code=
									list_Scene_object_transformation(scene_object,(void *)NULL);
							}
							else
							{
								return_code=for_each_Scene_object_in_Scene(scene,
									list_Scene_object_transformation,(void *)NULL);
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"No object named '%s' in scene %s",scene_object_name,scene_name);
						return_code=0;
					}
					DEALLOCATE(scene_name);
				}
				else
				{
					return_code=0;
				}
			} /* parse error, help */
			DEACCESS(Scene)(&scene);
			if (scene_object_name)
			{
				DEALLOCATE(scene_object_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_transformation.  Missing command_data_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_transformation.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_transformation */

#if defined (SGI_MOVIE_FILE)
static int gfx_list_movie_graphics(struct Parse_state *state,
	void *dummy_to_be_modified,void *movie_graphics_manager_void)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Executes a GFX LIST MOVIE.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Movie_graphics *movie;
	struct MANAGER(Movie_graphics) *movie_graphics_manager;

	ENTER(gfx_list_movie_graphics);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (movie_graphics_manager=
			(struct MANAGER(Movie_graphics) *)movie_graphics_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (movie=FIND_BY_IDENTIFIER_IN_MANAGER(Movie_graphics,name)(
						current_token,movie_graphics_manager))
					{
						return_code=list_Movie_graphics(movie,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown volume movie: %s",
							current_token);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," MOVIE_NAME");
					return_code=1;
				}
			}
			else
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Movie_graphics)(
					list_Movie_graphics,(void *)NULL,movie_graphics_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_movie_graphics.  Missing movie_graphics_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_movie_graphics.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_movie_graphics */
#endif /* defined (SGI_MOVIE_FILE) */

#if defined (USE_CMGUI_GRAPHICS_WINDOW)
static int gfx_list_graphics_window(struct Parse_state *state,
	void *dummy_to_be_modified,void *graphics_window_manager_void)
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Executes a GFX LIST WINDOW.
==============================================================================*/
{
	char commands_flag;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,set_Graphics_window}
	};
	struct Graphics_window *window;
	struct MANAGER(Graphics_window) *graphics_window_manager;

	ENTER(gfx_list_graphics_window);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (graphics_window_manager=
			(struct MANAGER(Graphics_window) *)graphics_window_manager_void)
		{
			commands_flag=0;
			/* if no window specified, list all windows */
			window=(struct Graphics_window *)NULL;
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &window;
			(option_table[1]).user_data= graphics_window_manager_void;
			(option_table[2]).to_be_modified= &window;
			(option_table[2]).user_data= graphics_window_manager_void;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (commands_flag)
				{
					if (window)
					{
						return_code=list_Graphics_window_commands(window,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
							list_Graphics_window_commands,(void *)NULL,
							graphics_window_manager);
					}
				}
				else
				{
					if (window)
					{
						return_code=list_Graphics_window(window,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
							list_Graphics_window,(void *)NULL,
							graphics_window_manager);
					}
				}
			}
			/* must deaccess window since accessed by set_Graphics_window */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_list_graphics_window.  "
				"Missing graphics_window_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_graphics_window.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_graphics_window */
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */

static int execute_command_gfx_list(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 November 2000

DESCRIPTION :
Executes a GFX LIST command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_list);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			option_table = CREATE(Option_table)();
			/* all_commands */
			Option_table_add_entry(option_table, "all_commands", NULL,
				command_data_void, gfx_list_all_commands);
			/* curve */
			Option_table_add_entry(option_table, "curve", NULL,
				command_data->curve_manager, gfx_list_Curve);
			/* data */
			Option_table_add_entry(option_table, "data", /*use_data*/(void *)1,
				command_data_void, gfx_list_FE_node);
			/* dgroup */
			Option_table_add_entry(option_table, "dgroup", NULL,
				command_data->data_root_region, gfx_list_region);
			/* egroup */
			Option_table_add_entry(option_table, "egroup", NULL,
				command_data->root_region, gfx_list_region);
			/* element */
			Option_table_add_entry(option_table, "elements", (void *)CM_ELEMENT,
				command_data_void, gfx_list_FE_element);
			/* environment_map */
			Option_table_add_entry(option_table, "environment_map", NULL,
				command_data_void, gfx_list_environment_map);
			/* faces */
			Option_table_add_entry(option_table, "faces", (void *)CM_FACE,
				command_data_void, gfx_list_FE_element);
			/* field */
			Option_table_add_entry(option_table, "field", NULL,
				command_data->computed_field_package, gfx_list_Computed_field);
			/* g_element */
			Option_table_add_entry(option_table, "g_element", NULL,
				command_data_void, gfx_list_g_element);
			/* glyph */
			Option_table_add_entry(option_table, "glyph", NULL,
				command_data->glyph_list, gfx_list_graphics_object);
			/* graphics_object */
			Option_table_add_entry(option_table, "graphics_object", NULL,
				command_data->graphics_object_list, gfx_list_graphics_object);
			/* grid_points */
			Option_table_add_entry(option_table, "grid_points", NULL,
				command_data_void, gfx_list_grid_points);
			/* light */
			Option_table_add_entry(option_table, "light", NULL,
				command_data->light_manager, gfx_list_light);
			/* lines */
			Option_table_add_entry(option_table, "lines", (void *)CM_LINE,
				command_data_void, gfx_list_FE_element);
			/* lmodel */
			Option_table_add_entry(option_table, "lmodel", NULL,
				command_data->light_model_manager, gfx_list_light_model);
			/* material */
			Option_table_add_entry(option_table, "material", NULL,
				Material_package_get_material_manager(command_data->material_package), gfx_list_graphical_material);
#if defined (SGI_MOVIE_FILE)
			/* movie */
			Option_table_add_entry(option_table, "movie", NULL,
				command_data->movie_graphics_manager, gfx_list_movie_graphics);
#endif /* defined (SGI_MOVIE_FILE) */
			/* ngroup */
			Option_table_add_entry(option_table, "ngroup", NULL,
				command_data->root_region, gfx_list_region);
			/* nodes */
			Option_table_add_entry(option_table, "nodes", /*use_data*/(void *)0,
				command_data_void, gfx_list_FE_node);
			/* scene */
			Option_table_add_entry(option_table, "scene", NULL,
				command_data->scene_manager, gfx_list_scene);
			/* spectrum */
			Option_table_add_entry(option_table, "spectrum", NULL,
				command_data->spectrum_manager, gfx_list_spectrum);
			/* texture */
			Option_table_add_entry(option_table, "texture", NULL,
				command_data->texture_manager, gfx_list_texture);
			/* transformation */
			Option_table_add_entry(option_table, "transformation", NULL,
				command_data_void, gfx_list_transformation);
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
			/* graphics window */
			Option_table_add_entry(option_table, "window", NULL,
				command_data->graphics_window_manager, gfx_list_graphics_window);
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx list", command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_list.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_list */

static int gfx_modify_element_group(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 March 2003

DESCRIPTION :
Modifies the membership of a group.  Only one of <add> or <remove> can
be specified at once.
==============================================================================*/
{
	char add_flag, all_flag, elements_flag, faces_flag, *from_region_path, lines_flag,
		*modify_region_path, remove_flag, selected_flag;
	enum CM_element_type cm_element_type;
	FE_value time;
	int number_not_removed, return_code;
	struct Cmiss_command_data *command_data;
	struct Computed_field *conditional_field;
	struct Cmiss_region *region;
	struct FE_region *fe_region, *modify_fe_region;
	struct LIST(FE_element) *element_list;
	struct Multi_range *element_ranges;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_conditional_field_data;

	ENTER(gfx_modify_element_group);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		modify_region_path = (char *)NULL;
		if (set_Cmiss_region_path(state, (void *)&modify_region_path,
			(void *)command_data->root_region))
		{
			/* initialise defaults */
			add_flag = 0;
			conditional_field=(struct Computed_field *)NULL;
			remove_flag = 0;
			all_flag = 0;
			selected_flag = 0;
			element_ranges = CREATE(Multi_range)();
			elements_flag = 0;
			faces_flag = 0;
			from_region_path = (char *)NULL;
			lines_flag = 0;
			if (command_data->default_time_keeper)
			{
				time = Time_keeper_get_time(command_data->default_time_keeper);
			}
			else
			{
				time = 0;
			}

			option_table=CREATE(Option_table)();
			/* add */
			Option_table_add_entry(option_table, "add", &add_flag,
				NULL, set_char_flag);
			/* all */
			Option_table_add_entry(option_table, "all", &all_flag,
				NULL, set_char_flag);
			/* conditional_field */
			set_conditional_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
			set_conditional_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_conditional_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"conditional_field",
				&conditional_field,&set_conditional_field_data,
				set_Computed_field_conditional);
			/* elements */
			Option_table_add_entry(option_table,"elements",&elements_flag,
				(void *)NULL,set_char_flag);
			/* faces */
			Option_table_add_entry(option_table,"faces",&faces_flag,
				(void *)NULL,set_char_flag);
			/* group */
			Option_table_add_entry(option_table, "group", &from_region_path,
				command_data->root_region, set_Cmiss_region_path);
			/* lines */
			Option_table_add_entry(option_table,"lines",&lines_flag,
				(void *)NULL,set_char_flag);
			/* remove */
			Option_table_add_entry(option_table, "remove", &remove_flag,
				NULL, set_char_flag);
			/* selected */
			Option_table_add_entry(option_table, "selected", &selected_flag,
				NULL, set_char_flag);
			/* default option: element number ranges */
			Option_table_add_entry(option_table, (char *)NULL, (void *)element_ranges,
				NULL, set_Multi_range);
			if (return_code = Option_table_multi_parse(option_table, state))
			{
				if (add_flag && remove_flag)
				{
					display_message(ERROR_MESSAGE, "gfx modify egroup:  "
						"Only specify one of add or remove at a time.");
					return_code = 0;
				}
				if ((!add_flag) && (!remove_flag))
				{
					display_message(ERROR_MESSAGE, "gfx modify egroup:  "
						"Must specify an operation, either add or remove.");				
					return_code = 0;
				}
				if (elements_flag + faces_flag + lines_flag > 1)
				{
					display_message(ERROR_MESSAGE, "gfx modify egroup:  "
						"Only specify one of elements, faces or lines at a time.");
					return_code = 0;
				}
				if (faces_flag)
				{
					cm_element_type = CM_FACE;
				}
				else if (lines_flag)
				{
					cm_element_type = CM_LINE;
				}
				else
				{
					cm_element_type = CM_ELEMENT;
				}
			}
			/* no errors, not asking for help */
			if (return_code)
			{
				if (Cmiss_region_get_region_from_path(command_data->root_region,
					modify_region_path, &region) &&
					(modify_fe_region = Cmiss_region_get_FE_region(region)))
				{
					if ((from_region_path &&
						Cmiss_region_get_region_from_path(command_data->root_region,
							from_region_path, &region) &&
						(fe_region = Cmiss_region_get_FE_region(region))) ||
						((!from_region_path) && FE_region_get_ultimate_master_FE_region(
							modify_fe_region,	&fe_region)))
					{
						if (element_list =
							FE_element_list_from_fe_region_selection_ranges_condition(
								fe_region, cm_element_type, command_data->element_selection,
								selected_flag, element_ranges, conditional_field, time))
						{
							if (0 < NUMBER_IN_LIST(FE_element)(element_list))
							{
								FE_region_begin_change(modify_fe_region);
								if (add_flag)
								{
									return_code = FOR_EACH_OBJECT_IN_LIST(FE_element)(
										FE_region_merge_FE_element_and_faces_and_nodes_iterator,
										(void *)modify_fe_region, element_list);
								}
								else /* remove_flag */
								{
									return_code = FE_region_remove_FE_element_list(
										modify_fe_region, element_list);
									if (0 < (number_not_removed =
										NUMBER_IN_LIST(FE_element)(element_list)))
									{
										display_message(WARNING_MESSAGE,
											"%d of the selected element(s) could not be removed",
											number_not_removed);
									}
								}
								FE_region_end_change(modify_fe_region);
							}
							else
							{
								display_message(WARNING_MESSAGE,
									"gfx modify egroup:  No elements specified");
							}
							DESTROY(LIST(FE_element))(&element_list);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_element_group.  Could not make element_list");
							return_code = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_modify_element_group.  Invalid source region");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_element_group.  Invalid modify region");
					return_code = 0;
				}
			}
			DESTROY(Option_table)(&option_table);
			if (from_region_path)
			{
				DEALLOCATE(from_region_path);
			}
			if (conditional_field)
			{
				DEACCESS(Computed_field)(&conditional_field);
			}
			DESTROY(Multi_range)(&element_ranges);
		}
		if (modify_region_path)
		{
			DEALLOCATE(modify_region_path);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_element_group.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_element_group */

int Cmiss_region_modify_g_element(struct Cmiss_region *region,
	struct Scene *scene, struct GT_element_settings *settings,
	int delete_flag, int position)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *same_settings;

	ENTER(iterator_modify_g_element);
	if (region && scene && settings)
	{
		if (gt_element_group = Scene_get_graphical_element_group(scene, region))
		{
			/* get settings describing same geometry in list */
			same_settings = first_settings_in_GT_element_group_that(
				gt_element_group, GT_element_settings_same_name_or_geometry,
				(void *)settings);
			if (delete_flag)
			{
				/* delete */
				if (same_settings)
				{
					return_code =
						GT_element_group_remove_settings(gt_element_group, same_settings);
				}
				else
				{
					return_code = 1;
				}
			}
			else
			{
				/* add/modify */
				if (same_settings)
				{
					ACCESS(GT_element_settings)(same_settings);
					if (-1 != position)
					{
						/* move same_settings to new position */
						GT_element_group_remove_settings(gt_element_group, same_settings);
						GT_element_group_add_settings(gt_element_group, same_settings,
							position);
					}
					/* modify same_settings to match new ones */
					return_code = GT_element_group_modify_settings(gt_element_group,
						same_settings, settings);
					DEACCESS(GT_element_settings)(&same_settings);
				}
				else
				{
					return_code = 0;
					if (same_settings = CREATE(GT_element_settings)(
						GT_element_settings_get_settings_type(settings)))
					{
						ACCESS(GT_element_settings)(same_settings);
						if (GT_element_settings_copy_without_graphics_object(
							same_settings, settings))
						{
							return_code = GT_element_group_add_settings(gt_element_group,
								same_settings, position);
						}
						DEACCESS(GT_element_settings)(&same_settings);
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"iterator_modify_g_element.  g_element not in scene");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"iterator_modify_g_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* iterator_modify_g_element */

static int gfx_modify_g_element(struct Parse_state *state,
	void *help_mode,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT command.
Parameter <help_mode> should be NULL when calling this function.
==============================================================================*/
{
	char *dummy_string, *region_path, *settings_name;
	int previous_state_index, return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct GT_element_group *gt_element_group;
	struct G_element_command_data g_element_command_data;
	struct Modify_g_element_data modify_g_element_data;
	struct Option_table *option_table;

	ENTER(gfx_modify_g_element);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		return_code = 1;
		/* initialize defaults */
		Cmiss_region_get_root_region_path(&region_path);
		if (!help_mode)
		{
			option_table = CREATE(Option_table)();
			if (!state->current_token ||
				(strcmp(PARSER_HELP_STRING, state->current_token) &&
					strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
			{
				/* default option: group name */
				Option_table_add_entry(option_table, (char *)NULL, &region_path,
					command_data->root_region, set_Cmiss_region_path);
			}
			else
			{
				/* help: call this function in help_mode */
				Option_table_add_entry(option_table, "REGION_PATH",
					/*help_mode*/(void *)1, command_data_void, gfx_modify_g_element);
			}
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		if (return_code)
		{
			if (region_path && Cmiss_region_get_region_from_path(
				command_data->root_region, region_path, &region))
			{
				/* set defaults */
				modify_g_element_data.delete_flag = 0;
				modify_g_element_data.position = -1;
				modify_g_element_data.scene =
					ACCESS(Scene)(command_data->default_scene);

				modify_g_element_data.settings = (struct GT_element_settings *)NULL;
				/* Look ahead for the "as" option and find the settings with that name
					if there is one.  Then the modify routines can keep the previous defaults */
				if (state && (previous_state_index = state->current_index))
				{
					option_table = CREATE(Option_table)();
					/* as */
					settings_name = (char *)NULL;
					Option_table_add_name_entry(option_table, "as", &settings_name);
					/* scene */
					Option_table_add_entry(option_table, "scene",
						&(modify_g_element_data.scene),
						command_data->scene_manager, set_Scene);
					/* default to absorb everything else */
					dummy_string = (char *)NULL;
					Option_table_add_name_entry(option_table, (char *)NULL, &dummy_string);
					return_code = Option_table_multi_parse(option_table, state);
					DESTROY(Option_table)(&option_table);
					if (return_code && settings_name)
					{
						if (gt_element_group = Scene_get_graphical_element_group(
							modify_g_element_data.scene, region))
						{
							if (modify_g_element_data.settings = first_settings_in_GT_element_group_that(
								gt_element_group, GT_element_settings_has_name, (void *)settings_name))
							{
								ACCESS(GT_element_settings)(modify_g_element_data.settings);
							}
						}
					}

					if (dummy_string)
					{
						DEALLOCATE(dummy_string);
					}
					if (settings_name)
					{
						DEALLOCATE(settings_name);
					}
					/* Return back to where we were */
					shift_Parse_state(state, previous_state_index - state->current_index);
				}					

				g_element_command_data.default_material =
					Material_package_get_default_material(command_data->material_package);
				g_element_command_data.graphics_font_package = 
					command_data->graphics_font_package;
				g_element_command_data.default_font = command_data->default_font;
				g_element_command_data.glyph_list = command_data->glyph_list;
				g_element_command_data.computed_field_manager =
					Computed_field_package_get_computed_field_manager(
						command_data->computed_field_package);
				g_element_command_data.region = region;
				g_element_command_data.root_region = command_data->root_region;
				g_element_command_data.graphical_material_manager =
					Material_package_get_material_manager(command_data->material_package);
				g_element_command_data.scene_manager = command_data->scene_manager;
				g_element_command_data.spectrum_manager =
					command_data->spectrum_manager;
				g_element_command_data.default_spectrum =
					command_data->default_spectrum;
				g_element_command_data.user_interface = command_data->user_interface;
				g_element_command_data.texture_manager = command_data->texture_manager;
				g_element_command_data.volume_texture_manager =
					command_data->volume_texture_manager;

				option_table = CREATE(Option_table)();
				/* cylinders */
				Option_table_add_entry(option_table, "cylinders",
					(void *)&modify_g_element_data, (void *)&g_element_command_data,
					gfx_modify_g_element_cylinders);
				/* data_points */
				Option_table_add_entry(option_table, "data_points",
					(void *)&modify_g_element_data, (void *)&g_element_command_data,
					gfx_modify_g_element_data_points);
				/* element_points */
				Option_table_add_entry(option_table, "element_points",
					(void *)&modify_g_element_data, (void *)&g_element_command_data,
					gfx_modify_g_element_element_points);
				/* general */
				Option_table_add_entry(option_table, "general",
					(void *)region, (void *)(command_data->default_scene),
					gfx_modify_g_element_general);
				/* iso_surfaces */
				Option_table_add_entry(option_table, "iso_surfaces",
					(void *)&modify_g_element_data, (void *)&g_element_command_data,
					gfx_modify_g_element_iso_surfaces);
				/* lines */
				Option_table_add_entry(option_table, "lines",
					(void *)&modify_g_element_data, (void *)&g_element_command_data,
					gfx_modify_g_element_lines);
				/* node_points */
				Option_table_add_entry(option_table, "node_points",
					(void *)&modify_g_element_data, (void *)&g_element_command_data,
					gfx_modify_g_element_node_points);
				/* streamlines */
				Option_table_add_entry(option_table, "streamlines",
					(void *)&modify_g_element_data, (void *)&g_element_command_data,
					gfx_modify_g_element_streamlines);
				/* surfaces */
				Option_table_add_entry(option_table, "surfaces",
					(void *)&modify_g_element_data, (void *)&g_element_command_data,
					gfx_modify_g_element_surfaces);

				return_code = Option_table_parse(option_table, state);
				if (return_code && (modify_g_element_data.settings))
				{
					return_code = Cmiss_region_modify_g_element(region,
						modify_g_element_data.scene, modify_g_element_data.settings,
						modify_g_element_data.delete_flag,
						modify_g_element_data.position);
				} /* parse error,help */
				DESTROY(Option_table)(&option_table);
				if (modify_g_element_data.settings)
				{
					DEACCESS(GT_element_settings)(&(modify_g_element_data.settings));
				}
				DEACCESS(Scene)(&modify_g_element_data.scene);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_g_element.  Invalid region");
				return_code = 0;
			}
		} /* parse error,help */
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_g_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_g_element */

static int gfx_modify_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Executes a GFX MODIFY GRAPHICS_OBJECT command.
==============================================================================*/
{
	char glyph_flag;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct GT_object *graphics_object;
	struct Graphical_material *material;
	struct Option_table *option_table;
	struct Spectrum *spectrum;

	ENTER(gfx_modify_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		graphics_object=(struct GT_object *)NULL;
		if (!state->current_token||
			(strcmp(PARSER_HELP_STRING,state->current_token)&&
			strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
		{
			if(graphics_object=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)
				(state->current_token,command_data->graphics_object_list))
			{
				shift_Parse_state(state,1);				
				/* initialise defaults */
				glyph_flag = 0;
				if (material = get_GT_object_default_material(graphics_object))
				{
					ACCESS(Graphical_material)(material);
				}
				if (spectrum = get_GT_object_spectrum(graphics_object))
				{
					ACCESS(Spectrum)(spectrum);
				}
				option_table = CREATE(Option_table)();
				Option_table_add_char_flag_entry(option_table, "glyph", &glyph_flag);
				Option_table_add_set_Material_entry(option_table, "material",&material,
					command_data->material_package);
				Option_table_add_entry(option_table,"spectrum",&spectrum,
					command_data->spectrum_manager,set_Spectrum);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
				if (return_code)
				{
					set_GT_object_default_material(graphics_object, material);
					set_GT_object_Spectrum(graphics_object, spectrum);
					if (glyph_flag)
					{
						if (!(IS_OBJECT_IN_LIST(GT_object)(graphics_object, 
							command_data->glyph_list)))
						{
							ADD_OBJECT_TO_LIST(GT_object)(graphics_object, 
							command_data->glyph_list);
						}
					}
				}
				if (material)
				{
					DEACCESS(Graphical_material)(&material);
				}
				if (spectrum)
				{
					DEACCESS(Spectrum)(&spectrum);
				}
			}
			else
			{
				if (state->current_token)
				{
					display_message(ERROR_MESSAGE,"Could not find object named '%s'",
						state->current_token);
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing graphics object name");
				}
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{	
			/* Help */
			display_message(INFORMATION_MESSAGE,
				"\n      GRAPHICS_OBJECT_NAME");

			glyph_flag = 0;
			spectrum = (struct Spectrum *)NULL;
			material = (struct Graphical_material *)NULL;
			option_table=CREATE(Option_table)();
			Option_table_add_char_flag_entry(option_table, "glyph", &glyph_flag);
			Option_table_add_set_Material_entry(option_table, "material",&material,
				command_data->material_package);
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			return_code=Option_table_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_graphics_object.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_graphics_object */

static int gfx_modify_node_group(struct Parse_state *state,
	void *use_data, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 March 2003

DESCRIPTION :
Modifies the membership of a group.  Only one of <add> or <remove> can
be specified at once.
If <used_data_flag> is set, use data_manager and data_selection, otherwise
use node_manager and node_selection.
==============================================================================*/
{
	char add_flag, all_flag, *from_region_path, *modify_region_path,
		remove_flag, selected_flag;
	FE_value time;
	int number_not_removed, return_code;
	struct Cmiss_command_data *command_data;
	struct Computed_field *conditional_field;
	struct Cmiss_region *region, *root_region;
	struct FE_node_selection *node_selection;
	struct FE_region *fe_region, *modify_fe_region;
	struct LIST(FE_node) *node_list;
	struct Multi_range *node_ranges;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_conditional_field_data;

	ENTER(gfx_modify_node_group);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		modify_region_path = (char *)NULL;
		if (use_data)
		{
			root_region = command_data->data_root_region;
			node_selection = command_data->data_selection;
		}
		else
		{
			root_region = command_data->root_region;
			node_selection = command_data->node_selection;
		}
		if (set_Cmiss_region_path(state, (void *)&modify_region_path,
			(void *)root_region))
		{
			/* initialise defaults */
			add_flag = 0;
			remove_flag = 0;
			all_flag = 0;
			conditional_field=(struct Computed_field *)NULL;
			selected_flag = 0;
			node_ranges = CREATE(Multi_range)();
			from_region_path = (char *)NULL;
			if (command_data->default_time_keeper)
			{
				time = Time_keeper_get_time(command_data->default_time_keeper);
			}
			else
			{
				time = 0.0;
			}

			option_table=CREATE(Option_table)();
			/* add */
			Option_table_add_entry(option_table, "add", &add_flag,
				NULL, set_char_flag);
			/* all */
			Option_table_add_entry(option_table, "all", &all_flag,
				NULL, set_char_flag);
			/* conditional_field */
			set_conditional_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
			set_conditional_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_conditional_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"conditional_field",
				&conditional_field,&set_conditional_field_data,
				set_Computed_field_conditional);
			/* group */
			Option_table_add_entry(option_table, "group", &from_region_path,
				root_region, set_Cmiss_region_path);
			/* remove */
			Option_table_add_entry(option_table, "remove", &remove_flag,
				NULL, set_char_flag);
			/* selected */
			Option_table_add_entry(option_table, "selected", &selected_flag,
				NULL, set_char_flag);
			/* default option: node number ranges */
			Option_table_add_entry(option_table, (char *)NULL, (void *)node_ranges,
				NULL, set_Multi_range);
			if (return_code = Option_table_multi_parse(option_table, state))
			{
				if (add_flag && remove_flag)
				{
					if (use_data)
					{
						display_message(ERROR_MESSAGE,"gfx modify dgroup:  "
							"Only specify one of add or remove at a time.");
					}
					else
					{
						display_message(ERROR_MESSAGE,"gfx modify ngroup:  "
							"Only specify one of add or remove at a time.");
					}
					return_code = 0;
				}
				if ((!add_flag) && (!remove_flag))
				{
					if (use_data)
					{
						display_message(ERROR_MESSAGE,"gfx modify dgroup:  "
							"Must specify an operation, either add or remove.");				
					}
					else
					{
						display_message(ERROR_MESSAGE,"gfx modify ngroup:  "
							"Must specify an operation, either add or remove.");				
					}
					return_code = 0;
				}
			}
			/* no errors, not asking for help */
			if (return_code)
			{
				if (Cmiss_region_get_region_from_path(root_region,
					modify_region_path, &region) &&
					(modify_fe_region = Cmiss_region_get_FE_region(region)))
				{
					if ((from_region_path &&
						Cmiss_region_get_region_from_path(root_region,
							from_region_path, &region) &&
						(fe_region = Cmiss_region_get_FE_region(region))) ||
						((!from_region_path) && FE_region_get_ultimate_master_FE_region(
							modify_fe_region,	&fe_region)))
					{
						if (node_list =
							FE_node_list_from_fe_region_selection_ranges_condition(
								fe_region, node_selection, selected_flag, node_ranges,
								conditional_field, time))
						{
							if (0 < NUMBER_IN_LIST(FE_node)(node_list))
							{
								FE_region_begin_change(modify_fe_region);
								if (add_flag)
								{
									return_code = FOR_EACH_OBJECT_IN_LIST(FE_node)(
										FE_region_merge_FE_node_iterator,
										(void *)modify_fe_region, node_list);
								}
								else /* remove_flag */
								{
									return_code = FE_region_remove_FE_node_list(
										modify_fe_region, node_list);
									if (0 < (number_not_removed =
										NUMBER_IN_LIST(FE_node)(node_list)))
									{
										display_message(WARNING_MESSAGE,
											"%d of the selected node(s) could not be removed",
											number_not_removed);
									}
								}
								FE_region_end_change(modify_fe_region);
							}
							else
							{
								if (use_data)
								{
									display_message(WARNING_MESSAGE,
										"gfx modify dgroup:  No data specified");
								}
								else
								{
									display_message(WARNING_MESSAGE,
										"gfx modify ngroup:  No nodes specified");
								}
							}
							DESTROY(LIST(FE_node))(&node_list);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_node_group.  Could not make node_list");
							return_code = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_modify_node_group.  Invalid source region");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_node_group.  Invalid modify region");
					return_code = 0;
				}
			}
			DESTROY(Option_table)(&option_table);
			if (from_region_path)
			{
				DEALLOCATE(from_region_path);
			}
			if (conditional_field)
			{
				DEACCESS(Computed_field)(&conditional_field);
			}
			DESTROY(Multi_range)(&node_ranges);
		}
		if (modify_region_path)
		{
			DEALLOCATE(modify_region_path);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_node_group.  Missing command_data");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_node_group */

struct FE_node_field_component_derivatives_data
{
	int *components_number_of_derivatives, number_of_components;
	enum FE_nodal_value_type **components_nodal_value_types;
};

int set_FE_node_field_component_derivatives(struct Parse_state *state,
	void *component_derivatives_data_void, void *field_address_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2001

DESCRIPTION :
Modifier function for entering the number of derivatives and their names
(d/d2, d2/ds1ds3 etc.) for each component of field to be defined at nodes.
Note requires field to be specified prior to entering this function, unless in
help mode.
==============================================================================*/
{
	char *current_token;
	enum FE_nodal_value_type *nodal_value_types;
	int i, j, number_of_components, number_of_derivatives, return_code;
	struct FE_field *field, **field_address;
	struct FE_node_field_component_derivatives_data *component_derivatives_data;

	ENTER(set_FE_node_field_component_derivatives);
	component_derivatives_data =
		(struct FE_node_field_component_derivatives_data *)NULL;
	if (state && (component_derivatives_data =
		(struct FE_node_field_component_derivatives_data *)
		component_derivatives_data_void) &&
		(0 == component_derivatives_data->number_of_components) &&
		(field_address = (struct FE_field **)field_address_void))
	{
		return_code = 1;
		if (field = *field_address)
		{
			number_of_components = get_FE_field_number_of_components(field);
		}
		else
		{
			/* following used for help */
			number_of_components = 3;
		}
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (field)
				{
					ALLOCATE(component_derivatives_data->components_number_of_derivatives,
						int, number_of_components);
					if (ALLOCATE(component_derivatives_data->components_nodal_value_types,
						enum FE_nodal_value_type *, number_of_components))
					{
						for (i = 0; i < number_of_components; i++)
						{
							component_derivatives_data->components_nodal_value_types[i] =
								(enum FE_nodal_value_type *)NULL;
						}
					}
					if (component_derivatives_data->components_number_of_derivatives &&
						component_derivatives_data->components_nodal_value_types)
					{
						/* remember number_of_components so arrays can be deallocated by
							 themselves */
						component_derivatives_data->number_of_components =
							number_of_components;
						for (i = 0; return_code && (i < number_of_components); i++)
						{
							if ((current_token = state->current_token) &&
								(1 == sscanf(current_token, " %d ", &number_of_derivatives)) &&
								(0 <= number_of_derivatives) && shift_Parse_state(state, 1))
							{
								component_derivatives_data->components_number_of_derivatives[i]
									= number_of_derivatives;
								if (0 < number_of_derivatives)
								{
									if (ALLOCATE(nodal_value_types,
										enum FE_nodal_value_type, number_of_derivatives))
									{
										component_derivatives_data->components_nodal_value_types[i]
											= nodal_value_types;
										for (j = 0; return_code && (j < number_of_derivatives); j++)
										{
											if (!((current_token = state->current_token) &&
												STRING_TO_ENUMERATOR(FE_nodal_value_type)(current_token,
													&(nodal_value_types[j])) &&
												shift_Parse_state(state, 1)))
											{
												display_message(ERROR_MESSAGE,
													"Missing or invalid nodal value type: %s",
													current_token);
												display_parse_state_location(state);
												return_code = 0;
											}
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"set_FE_node_field_component_derivatives.  "
											"Not enough memory for nodal_value_types");
										return_code = 0;
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Missing or invalid number of derivatives: %s",
									current_token);
								display_parse_state_location(state);
								return_code = 0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_FE_node_field_component_derivatives.  "
							"Not enough memory for derivatives");
						return_code = 0;
					}
					if (!return_code)
					{
						DEALLOCATE(
							component_derivatives_data->components_number_of_derivatives);
						if (component_derivatives_data->components_nodal_value_types)
						{
							for (i = 0; i < number_of_components; i++)
							{
								if (component_derivatives_data->components_nodal_value_types[i])
								{
									DEALLOCATE(component_derivatives_data->
										components_nodal_value_types[i]);
								}
							}
							DEALLOCATE(
								component_derivatives_data->components_nodal_value_types);
						}
						component_derivatives_data->number_of_components = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Must have a field before setting component derivatives");
					return_code = 0;
				}
			}
			else
			{
				for (i = 0; i < number_of_components; i++)
				{
					display_message(INFORMATION_MESSAGE," NUMBER_IN_COMPONENT_%d "
						"DERIVATIVE_NAMES(d/ds1 d2/ds1ds2 etc.) ...", i + 1);
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing component derivatives");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		if (component_derivatives_data &&
			(0 != component_derivatives_data->number_of_components))
		{
			display_message(ERROR_MESSAGE,
				"Component derivatives have already been set");
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_FE_node_field_component_derivatives.  Invalid argument(s)");
		}
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_node_field_component_derivatives */

struct FE_node_field_component_versions_data
{
	int *components_number_of_versions, number_of_components;
};

int set_FE_node_field_component_versions(struct Parse_state *state,
	void *component_versions_data_void, void *field_address_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2001

DESCRIPTION :
Modifier function for entering the number of versions for each component of
field to be defined at nodes. Note requires field to be specified prior to
entering this function, unless in help mode.
==============================================================================*/
{
	char *current_token;
	int i, number_of_components, number_of_versions, return_code;
	struct FE_field *field, **field_address;
	struct FE_node_field_component_versions_data *component_versions_data;

	ENTER(set_FE_node_field_component_versions);
	component_versions_data =
		(struct FE_node_field_component_versions_data *)NULL;
	if (state && (component_versions_data =
		(struct FE_node_field_component_versions_data *)
		component_versions_data_void) &&
		(0 == component_versions_data->number_of_components) &&
		(field_address = (struct FE_field **)field_address_void))
	{
		return_code = 1;
		if (field = *field_address)
		{
			number_of_components = get_FE_field_number_of_components(field);
		}
		else
		{
			/* following used for help */
			number_of_components = 3;
		}
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (field)
				{
					if (ALLOCATE(component_versions_data->components_number_of_versions,
						int, number_of_components))
					{
						/* remember number_of_components so arrays can be deallocated by
							 themselves */
						component_versions_data->number_of_components =
							number_of_components;
						for (i = 0; return_code && (i < number_of_components); i++)
						{
							if ((current_token = state->current_token) &&
								(1 == sscanf(current_token, " %d ", &number_of_versions)) &&
								(0 < number_of_versions) &&
								shift_Parse_state(state, 1))
							{
								component_versions_data->components_number_of_versions[i] =
									number_of_versions;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Missing or invalid number of versions: %s",current_token);
								display_parse_state_location(state);
								return_code = 0;
							}
						}
						if (!return_code)
						{
							DEALLOCATE(
								component_versions_data->components_number_of_versions);
							component_versions_data->number_of_components = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_FE_node_field_component_versions.  "
							"Not enough memory for versions");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Must have a field before setting component versions");
					return_code = 0;
				}
			}
			else
			{
				for (i = 0; i < number_of_components; i++)
				{
					display_message(INFORMATION_MESSAGE,
						" NUMBER_IN_COMPONENT_%d", i + 1);
				}
				if (!field)
				{
					display_message(INFORMATION_MESSAGE, " ...");
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing component versions");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		if (component_versions_data &&
			(0 != component_versions_data->number_of_components))
		{
			display_message(ERROR_MESSAGE,
				"Component versions have already been set");
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_FE_node_field_component_versions.  Invalid argument(s)");
		}
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_node_field_component_versions */

static int gfx_modify_nodes(struct Parse_state *state,
	void *use_data,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 April 2003

DESCRIPTION :
Executes a GFX MODIFY NODES/DATA command.
If <used_data_flag> is set, use data_manager and data_selection, otherwise
use node_manager and node_selection.
==============================================================================*/
{
	char all_flag, *region_path, selected_flag;
	int i, j, number_in_elements, number_of_components, return_code;
	FE_value time;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region, *root_region;
	struct Computed_field *conditional_field;
	struct FE_field *define_field, *undefine_field;
	struct FE_node *node;
	struct FE_node_field_component_derivatives_data component_derivatives_data;
	struct FE_node_field_component_versions_data component_versions_data;
	struct FE_node_field_creator *node_field_creator;
	struct FE_node_selection *node_selection;
	struct FE_region *fe_region;
	struct LIST(FE_node) *node_list;
	struct Multi_range *node_ranges;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_conditional_field_data;
	struct Set_FE_field_conditional_FE_region_data set_define_field_data,
		set_undefine_field_data;

	ENTER(gfx_modify_nodes);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (use_data)
		{
			root_region = command_data->data_root_region;
			node_selection = command_data->data_selection;
		}
		else
		{
			root_region = command_data->root_region;
			node_selection = command_data->node_selection;
		}
		/* initialise defaults */
		all_flag = 0;
		conditional_field=(struct Computed_field *)NULL;
		selected_flag = 0;
		node_field_creator = (struct FE_node_field_creator *)NULL;
		region_path = (char *)NULL;
		node_ranges = CREATE(Multi_range)();
		define_field = (struct FE_field *)NULL;
		component_derivatives_data.number_of_components = 0;
		component_derivatives_data.components_number_of_derivatives = (int *)NULL;
		component_derivatives_data.components_nodal_value_types =
			(enum FE_nodal_value_type **)NULL;
		component_versions_data.number_of_components = 0;
		component_versions_data.components_number_of_versions = (int *)NULL;
		undefine_field = (struct FE_field *)NULL;
		if (command_data->default_time_keeper)
		{
			time = Time_keeper_get_time(command_data->default_time_keeper);
		}
		else
		{
			time = 0.0;
		}

		option_table = CREATE(Option_table)();
		/* all */
		Option_table_add_entry(option_table, "all", &all_flag, NULL, set_char_flag);
		/* component_derivatives */
		Option_table_add_entry(option_table, "component_derivatives",
			(void *)&component_derivatives_data, (void *)&define_field,
			set_FE_node_field_component_derivatives);
		/* component_versions */
		Option_table_add_entry(option_table, "component_versions",
			(void *)&component_versions_data, (void *)&define_field,
			set_FE_node_field_component_versions);
		/* conditional_field */
		set_conditional_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
			command_data->computed_field_package);
		set_conditional_field_data.conditional_function=
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_conditional_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"conditional_field",
			&conditional_field,&set_conditional_field_data,
			set_Computed_field_conditional);
		/* define */
		set_define_field_data.conditional_function =
			(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL;
		set_define_field_data.user_data = (void *)NULL;
		set_define_field_data.fe_region =
			Cmiss_region_get_FE_region(command_data->root_region);
		Option_table_add_entry(option_table, "define", &define_field,
			(void *)&set_define_field_data, set_FE_field_conditional_FE_region);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			root_region, set_Cmiss_region_path);
		/* selected */
		Option_table_add_entry(option_table, "selected", &selected_flag,
			NULL, set_char_flag);
		/* undefine */
		set_undefine_field_data.conditional_function =
			(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL;
		set_undefine_field_data.user_data = (void *)NULL;
		set_undefine_field_data.fe_region =
			Cmiss_region_get_FE_region(command_data->root_region);
		Option_table_add_entry(option_table, "undefine", &undefine_field,
			(void *)&set_undefine_field_data, set_FE_field_conditional_FE_region);
		/* default option: node number ranges */
		Option_table_add_entry(option_table, (char *)NULL, (void *)node_ranges,
			NULL, set_Multi_range);
		if (return_code = Option_table_multi_parse(option_table, state))
		{
			if (define_field && undefine_field)
			{
				display_message(WARNING_MESSAGE,
				 	"gfx modify data:  Only specify one of define or undefine field");
				return_code = 0;
			}
			if ((!define_field) && (!undefine_field))
			{
				display_message(WARNING_MESSAGE,
					"gfx modify data:  Must specify define or undefine field");
				return_code = 0;
			}
		}
		if (return_code)
		{
			if (((region_path &&
				Cmiss_region_get_region_from_path(root_region, region_path, &region)) ||
				((all_flag || selected_flag ||
					(0 < Multi_range_get_number_of_ranges(node_ranges))) &&
					(region = root_region))) &&
				(fe_region = Cmiss_region_get_FE_region(region)))
			{
				if (node_list = FE_node_list_from_fe_region_selection_ranges_condition(
					fe_region, node_selection,  selected_flag, node_ranges,
					conditional_field, time))
				{
					if (0 < NUMBER_IN_LIST(FE_node)(node_list))
					{
						FE_region_begin_change(fe_region);
						if (define_field)
						{
							number_of_components =
								get_FE_field_number_of_components(define_field);
							if (node_field_creator =
								CREATE(FE_node_field_creator)(number_of_components))
							{
								if (component_versions_data.number_of_components)
								{
									if (number_of_components ==
										component_versions_data.number_of_components)
									{
										for (i = 0 ; i < number_of_components ; i++)
										{
											FE_node_field_creator_define_versions
												(node_field_creator, i, component_versions_data.
													components_number_of_versions[i]);
										}
									}
									else
									{
										display_message(WARNING_MESSAGE,
											"gfx modify data:  The number of specified "
											"versions must match the number of field components");
										return_code = 0;
									}
								}
								/* else default is 1 version */

								if (component_derivatives_data.number_of_components)
								{
									if (number_of_components ==
										component_derivatives_data.number_of_components)
									{
										for (i = 0 ; i < number_of_components ; i++)
										{
											for (j = 0 ; j < component_derivatives_data.
														 components_number_of_derivatives[i] ; j++)
											{
												FE_node_field_creator_define_derivative
													(node_field_creator, i, component_derivatives_data.
														components_nodal_value_types[i][j]);
											}
										}
									}
									else
									{
										display_message(WARNING_MESSAGE, "gfx modify data:  "
											"The number of specified derivative arrays must match "
											"the number of field components");
										return_code = 0;
									}
								}
								/* else default is no derivatives */
								if (return_code)
								{
									while (return_code &&
										(node = FIRST_OBJECT_IN_LIST_THAT(FE_node)(
											(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL, (void *)NULL,
											node_list)))
									{
										return_code = FE_region_define_FE_field_at_FE_node(
											fe_region, node, define_field,
											(struct FE_time_sequence *)NULL, node_field_creator) &&
											REMOVE_OBJECT_FROM_LIST(FE_node)(node, node_list);
									}
								}
								else
								{
									display_message(ERROR_MESSAGE, "gfx_modify_nodes.  "
										"Could not create derivative/version information");
								}
								DESTROY(FE_node_field_creator)(&(node_field_creator));
							}
							else
							{
								display_message(WARNING_MESSAGE,
									"gfx modify data:  Unable to make node_field_creator.");
								return_code = 0;
							}
						}
						else /* undefine_field */
						{
							if (FE_region_undefine_FE_field_in_FE_node_list(fe_region,
								undefine_field, node_list, &number_in_elements))
							{
								if (0 < number_in_elements)
								{
									display_message(WARNING_MESSAGE,
										"gfx modify nodes: Field could not be undefined in %d "
										"node(s) because in-use by elements", number_in_elements);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_modify_nodes:  Undefining field at nodes failed");
								return_code = 0;
							}
						}
						FE_region_end_change(fe_region);
					}
					else
					{
						if (use_data)
						{
							display_message(WARNING_MESSAGE,
								"gfx modify data:  No data specified");
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"gfx modify nodes:  No nodes specified");
						}
					}
					DESTROY(LIST(FE_node))(&node_list);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_nodes.  Could not make node_list");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "gfx_modify_nodes.  Invalid region");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
		DESTROY(Multi_range)(&node_ranges);
		if (define_field)
		{
			DEACCESS(FE_field)(&define_field);
		}
		if (undefine_field)
		{
			DEACCESS(FE_field)(&undefine_field);
		}
		if (component_derivatives_data.components_number_of_derivatives)
		{
			DEALLOCATE(component_derivatives_data.components_number_of_derivatives);
		}
		if (component_derivatives_data.components_nodal_value_types)
		{
			for (i = 0; i < component_derivatives_data.number_of_components; i++)
			{
				if (component_derivatives_data.components_nodal_value_types[i])
				{
					DEALLOCATE(
						component_derivatives_data.components_nodal_value_types[i]);
				}
			}
			DEALLOCATE(component_derivatives_data.components_nodal_value_types);
		}
		if (component_versions_data.components_number_of_versions)
		{
			DEALLOCATE(component_versions_data.components_number_of_versions);
		}
		if (conditional_field)
		{
			DEACCESS(Computed_field)(&conditional_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_nodes.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_nodes */

static int execute_command_gfx_modify(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 26 March 2001

DESCRIPTION :
Executes a GFX MODIFY command.
???DB.  Part of GFX EDIT ?
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Modify_environment_map_data modify_environment_map_data;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
	struct Modify_graphics_window_data modify_graphics_window_data;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
	struct Modify_light_data modify_light_data;
	struct Modify_light_model_data modify_light_model_data;
	struct Modify_scene_data modify_scene_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_modify);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				option_table=CREATE(Option_table)();
				/* data */
				Option_table_add_entry(option_table, "data", /*use_data*/(void *)1, 
					(void *)command_data, gfx_modify_nodes);
				/* dgroup */
				Option_table_add_entry(option_table,"dgroup",(void *)1/*data*/, 
					(void *)command_data, gfx_modify_node_group);
				/* egroup */
				Option_table_add_entry(option_table,"egroup",NULL, 
					(void *)command_data, gfx_modify_element_group);
				/* emoter */
				Option_table_add_entry(option_table,"emoter",NULL, 
					(void *)command_data->emoter_slider_dialog,
					gfx_modify_emoter);
				/* environment_map */
				modify_environment_map_data.graphical_material_manager=
					Material_package_get_material_manager(command_data->material_package);
				modify_environment_map_data.environment_map_manager=
					command_data->environment_map_manager;
				Option_table_add_entry(option_table,"environment_map",NULL, 
					(&modify_environment_map_data),modify_Environment_map);
				/* flow_particles */
				Option_table_add_entry(option_table,"flow_particles",NULL, 
					(void *)command_data, gfx_modify_flow_particles);
				/* g_element */
				Option_table_add_entry(option_table,"g_element",NULL, 
					(void *)command_data, gfx_modify_g_element);
				/* graphics_object */
				Option_table_add_entry(option_table,"graphics_object",NULL, 
					(void *)command_data, gfx_modify_graphics_object);
				/* light */
				modify_light_data.default_light=command_data->default_light;
				modify_light_data.light_manager=command_data->light_manager;
				Option_table_add_entry(option_table,"light",NULL, 
					(void *)(&modify_light_data), modify_Light);
				/* lmodel */
				modify_light_model_data.default_light_model=
					command_data->default_light_model;
				modify_light_model_data.light_model_manager=
					command_data->light_model_manager;
				Option_table_add_entry(option_table,"lmodel",NULL, 
					(void *)(&modify_light_model_data), modify_Light_model);
				/* material */
				Option_table_add_entry(option_table,"material",NULL, 
					(void *)command_data->material_package, modify_Graphical_material);
				/* ngroup */
				Option_table_add_entry(option_table,"ngroup",NULL, 
					(void *)command_data, gfx_modify_node_group);
				/* nodes */
				Option_table_add_entry(option_table, "nodes", /*use_data*/(void *)0, 
					(void *)command_data, gfx_modify_nodes);
				/* scene */
				modify_scene_data.light_manager=command_data->light_manager;
				modify_scene_data.scene_manager=command_data->scene_manager;
				modify_scene_data.default_scene=command_data->default_scene;
				/* following used for enabling GFEs */
				modify_scene_data.computed_field_manager=
					Computed_field_package_get_computed_field_manager(
						command_data->computed_field_package);
				modify_scene_data.root_region = command_data->root_region;
				modify_scene_data.data_root_region = command_data->data_root_region;
				modify_scene_data.element_point_ranges_selection=
					command_data->element_point_ranges_selection;
				modify_scene_data.element_selection=command_data->element_selection;
				modify_scene_data.node_selection=command_data->node_selection;
				modify_scene_data.data_selection=command_data->data_selection;
				modify_scene_data.user_interface=command_data->user_interface;
				Option_table_add_entry(option_table,"scene",NULL, 
					(void *)(&modify_scene_data), modify_Scene);
				/* spectrum */
				Option_table_add_entry(option_table,"spectrum",NULL, 
					(void *)command_data, gfx_modify_Spectrum);
				/* texture */
				Option_table_add_entry(option_table,"texture",NULL, 
					(void *)command_data, gfx_modify_Texture);
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
				/* window */
				modify_graphics_window_data.computed_field_package=
					command_data->computed_field_package;
				modify_graphics_window_data.graphics_window_manager=
					command_data->graphics_window_manager;
				modify_graphics_window_data.interactive_tool_manager=
					command_data->interactive_tool_manager;
				modify_graphics_window_data.light_manager=command_data->light_manager;
				modify_graphics_window_data.light_model_manager=
					command_data->light_model_manager;
				modify_graphics_window_data.scene_manager=command_data->scene_manager;
				modify_graphics_window_data.texture_manager=
					command_data->texture_manager;
				Option_table_add_entry(option_table,"window",NULL, 
					(void *)(&modify_graphics_window_data), modify_Graphics_window);
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */

				return_code=Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("gfx modify",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_modify.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_modify.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_modify */

#if defined (SGI_MOVIE_FILE)
#if defined (MOTIF)
int gfx_movie(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 2 February 2000

DESCRIPTION :
???RC Movie should ACCESS the graphics window so that it cannot be closed while
movie is being created.
==============================================================================*/
{
	static char *default_movie_name="default";
	char add_frame,avi,cinepak_avi,cinepak_quicktime,*create_file_name,end,
		every_frame,force_onscreen,indeo_avi,indeo_quicktime,loop,*movie_name,
		mvc1_sgi_movie3,once,*open_file_name,play,quicktime,
		rle24_sgi_movie3,skip_frames,sgi_movie3,stop;
	double speed;
	int height, width;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Movie_graphics *movie;
	struct Option_table *option_table;
	struct X3d_movie *x3d_movie;
	struct Graphics_window *graphics_window;

	ENTER(gfx_movie);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data = (struct Cmiss_command_data *)command_data_void)
		{
			/* initialise defaults */
			if (ALLOCATE(movie_name,char,strlen(default_movie_name)+1))
			{
				strcpy(movie_name,default_movie_name);
			}
			else
			{
				movie_name=(char *)NULL;
			}
			add_frame = 0;
			avi = 0;
			cinepak_avi = 0;
			cinepak_quicktime = 0;
			create_file_name=(char *)NULL;
			end = 0;
			every_frame = 0;
			force_onscreen = 0;
			height = 0;
			indeo_avi = 0;
			indeo_quicktime = 0;
			loop = 0;
			mvc1_sgi_movie3 = 0;
			once = 0;
			open_file_name=(char *)NULL;
			play = 0;
			quicktime = 0;
			rle24_sgi_movie3 = 0;
			sgi_movie3 = 0;
			skip_frames = 0;
			speed = 0;
			stop = 0;
			width = 0;
			graphics_window=(struct Graphics_window *)NULL;

			option_table=CREATE(Option_table)();
			/* add_frame */
			Option_table_add_entry(option_table,"add_frame",&add_frame,
				NULL,set_char_flag);
			/* avi */
			Option_table_add_entry(option_table,"avi",&avi,
				NULL,set_char_flag);
			/* cinepak_avi */
			Option_table_add_entry(option_table,"cinepak_avi",&cinepak_avi,
				NULL,set_char_flag);
			/* cinepak_quicktime */
			Option_table_add_entry(option_table,"cinepak_quicktime",
				&cinepak_quicktime,NULL,set_char_flag);
			/* create */
			Option_table_add_entry(option_table,"create",&create_file_name,
				(void *)1,set_name);
			/* end */
			Option_table_add_entry(option_table,"end",&end,
				NULL,set_char_flag);
			/* every_frame */
			Option_table_add_entry(option_table,"every_frame",&every_frame,
				NULL,set_char_flag);
			/* force_onscreen */
			Option_table_add_entry(option_table,"force_onscreen",&force_onscreen,
				NULL,set_char_flag);
			/* height */
			Option_table_add_entry(option_table,"height",&height,
				NULL,set_int_non_negative);
			/* indeo_avi */
			Option_table_add_entry(option_table,"indeo_avi",&indeo_avi,
				NULL,set_char_flag);
			/* indeo_quicktime */
			Option_table_add_entry(option_table,"indeo_quicktime",&indeo_quicktime,
				NULL,set_char_flag);
			/* loop */
			Option_table_add_entry(option_table,"loop",&loop,
				NULL,set_char_flag);
			/* mvc1_sgi_movie3 */
			Option_table_add_entry(option_table,"mvc1_sgi_movie3",&mvc1_sgi_movie3,
				NULL,set_char_flag);
			/* name */
			Option_table_add_entry(option_table,"name",&movie_name,
				(void *)1,set_name);
			/* once */
			Option_table_add_entry(option_table,"once",&once,
				NULL,set_char_flag);
			/* open */
			Option_table_add_entry(option_table,"open",&open_file_name,
				(void *)1,set_name);
			/* play */
			Option_table_add_entry(option_table,"play",&play,
				NULL,set_char_flag);
			/* quicktime */
			Option_table_add_entry(option_table,"quicktime",&quicktime,
				NULL,set_char_flag);
			/* rle24_sgi_movie3 */
			Option_table_add_entry(option_table,"rle24_sgi_movie3",&rle24_sgi_movie3,
				NULL,set_char_flag);
			/* sgi_movie3 */
			Option_table_add_entry(option_table,"sgi_movie3",&sgi_movie3,
				NULL,set_char_flag);
			/* skip_frames */
			Option_table_add_entry(option_table,"skip_frames",&skip_frames,
				NULL,set_char_flag);
			/* speed */
			Option_table_add_entry(option_table,"speed",&speed,
				NULL,set_double);
			/* stop */
			Option_table_add_entry(option_table,"stop",&stop,
				NULL,set_char_flag);
			/* width */
			Option_table_add_entry(option_table,"width",&width,
				NULL,set_int_non_negative);
			/* window */
			Option_table_add_entry(option_table,"window",&graphics_window,
				command_data->graphics_window_manager,set_Graphics_window);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				movie=(struct Movie_graphics *)NULL;
				if (movie_name)
				{
					movie=FIND_BY_IDENTIFIER_IN_MANAGER(Movie_graphics,name)(
						movie_name,command_data->movie_graphics_manager);
				}
				if ((avi + cinepak_avi + cinepak_quicktime + indeo_quicktime + indeo_avi + 
					mvc1_sgi_movie3 + quicktime + rle24_sgi_movie3 + sgi_movie3) > 1)
				{
					display_message(ERROR_MESSAGE,
						"gfx_movie.  Can only specify one movie format, avi,  cinepak_quicktime, "
						"indeo_quicktime, quicktime, rle24_sgi_movie3 or sgi_movie3");
					return_code = 0;
				}
				if (open_file_name && (avi + cinepak_avi + cinepak_quicktime + indeo_avi + 
					indeo_quicktime + mvc1_sgi_movie3 + quicktime + rle24_sgi_movie3 + sgi_movie3))
				{
					display_message(ERROR_MESSAGE,
						"gfx_movie.  Cannot specify a format"
						" (avi, cinepak_quicktime, indeo_quicktime, quicktime or sgi_movie3) "
						"when opening an existing movie (open)");
					return_code = 0;
				}
			}
			if (return_code)
			{
				if (movie)
				{
					if (open_file_name || create_file_name)
					{
						display_message(ERROR_MESSAGE,
							"gfx_movie.  Movie %s is already open and must be ended before "
							"another opened or create",movie_name);
					}
				}
				else
				{
					if (movie_name)
					{
						if (open_file_name)
						{
							if (create_file_name)
							{
								display_message(ERROR_MESSAGE,
									"gfx_movie.  Specify only one of open and create");
							}
							else
							{
								if (!(movie=CREATE(Movie_graphics)(movie_name,open_file_name,
									X3D_MOVIE_OPEN_FILE)))
								{
									display_message(ERROR_MESSAGE,
										"gfx_movie.  Could not create movie.");
								}
							}
						}
						else
						{
							if (create_file_name)
							{
								if(avi)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_UNCOMPRESSED_AVI);
								}
								else if(cinepak_avi)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_CINEPAK_AVI);
								}
								else if(cinepak_quicktime)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_CINEPAK_QUICKTIME);
								}
								else if(indeo_avi)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_INDEO_AVI);
								}
								else if(indeo_quicktime)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_INDEO_QUICKTIME);
								}
								else if(quicktime)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_APPLE_ANIMATION_QUICKTIME);
								}
								else if(rle24_sgi_movie3)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_RLE24_SGI_MOVIE3);
								}
								else if(mvc1_sgi_movie3)
								{
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_MVC1_SGI_MOVIE3);
								}
								else
								{
									/* Default to this if no format is given */
									movie = CREATE(Movie_graphics)(movie_name,create_file_name,
										X3D_MOVIE_CREATE_FILE_UNCOMPRESSED_SGI_MOVIE3);
								}
								if (!movie)
								{
									display_message(ERROR_MESSAGE,
										"gfx_movie.  Could not create movie.");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_movie.  Need to specify 'open' or 'create' FILENAME");
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_movie.  No name given for new movie");
					}
					if (movie)
					{
						if (!ADD_OBJECT_TO_MANAGER(Movie_graphics)(movie,
							command_data->movie_graphics_manager))
						{
							display_message(ERROR_MESSAGE,
								"gfx_movie.  Could not add movie to manager");
							DESTROY(Movie_graphics)(&movie);
							movie = (struct Movie_graphics *)NULL;
						}
					}
					if (movie)
					{
						/* attach the time object of the new movie to the default time
							keeper */
						Time_object_set_time_keeper(
							X3d_movie_get_time_object(Movie_graphics_get_X3d_movie(movie)),
							command_data->default_time_keeper);
					}
				}
				if (movie && (x3d_movie = Movie_graphics_get_X3d_movie(movie)))
				{
					if (graphics_window)
					{
						Movie_graphics_set_Graphics_window(movie, graphics_window);
					}
					if ( add_frame )
					{
						Movie_graphics_add_frame_to_movie(movie, width, height, force_onscreen);
					}
					if ( every_frame )
					{
						X3d_movie_set_play_every_frame(x3d_movie, 1);
					}
					if ( loop )
					{
						X3d_movie_set_play_loop(x3d_movie, 1);
					}
					if ( once )
					{
						X3d_movie_set_play_loop(x3d_movie, 0);
					}
					if ( play )
					{
						X3d_movie_play(x3d_movie);
					}
					if ( skip_frames )
					{
						X3d_movie_set_play_every_frame(x3d_movie, 0);
					}
					if ( speed != 0.0 )
					{
						X3d_movie_set_play_speed(x3d_movie, speed);
					}
					if ( stop )
					{
						X3d_movie_stop(x3d_movie);
					}
					if ( end )
					{
						return_code=REMOVE_OBJECT_FROM_MANAGER(Movie_graphics)(movie,
							command_data->movie_graphics_manager);
						movie=(struct Movie_graphics *)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_movie.  Invalid movie object");
					return_code=0;
				}
			}
			DESTROY(Option_table)(&option_table);
			if (graphics_window)
			{
				DEACCESS(Graphics_window)(&graphics_window);
			}
			if (create_file_name)
			{
				DEALLOCATE(create_file_name);
			}
			if (open_file_name)
			{
				DEALLOCATE(open_file_name);
			}
			if (movie_name)
			{
				DEALLOCATE(movie_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_movie.  Missing command data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_movie.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* gfx_movie */
#endif /* defined (MOTIF) */
#endif /* defined (SGI_MOVIE_FILE) */

#if defined (MOTIF) || (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE)
static int execute_command_gfx_node_tool(struct Parse_state *state,
	void *data_tool_flag, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Executes a GFX NODE_TOOL or GFX_DATA_TOOL command. If <data_tool_flag> is set,
then the <data_tool> is being modified, otherwise the <node_tool>.
Which tool that is being modified is passed in <node_tool_void>.
==============================================================================*/
{
	static char *(dialog_strings[2]) = {"open_dialog", "close_dialog"};
	char *dialog_string, *region_path;
	int create_enabled,define_enabled,edit_enabled,motion_update_enabled,
		return_code,select_enabled, streaming_create_enabled,
		constrain_to_surface;
#if defined (WX_USER_INTERFACE)
	int element_dimension, element_create_enabled;
#endif /*(WX_USER_INTERFACE)*/
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *root_region;
	struct Computed_field *coordinate_field, *command_field;
	struct Node_tool *node_tool;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_command_field_data;

	ENTER(execute_command_gfx_node_tool);
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (data_tool_flag)
		{
			node_tool = command_data->data_tool;
			root_region = command_data->data_root_region;
		}
		else
		{
			node_tool=command_data->node_tool;
			root_region = command_data->root_region;
		}
		/* initialize defaults */
		coordinate_field=(struct Computed_field *)NULL;
		create_enabled=0;
		define_enabled=0;
		edit_enabled=0;
		motion_update_enabled=0;
		select_enabled=1;
		streaming_create_enabled = 0;
		constrain_to_surface = 0;
		command_field=(struct Computed_field *)NULL;
		region_path = (char *)NULL;
#if defined (WX_USER_INTERFACE)
		if (!data_tool_flag)
		{
		element_create_enabled = 0;
		element_dimension = 2;
		}
#endif /*(WX_USER_INTERFACE)*/
		if (node_tool)
		{
			coordinate_field=Node_tool_get_coordinate_field(node_tool);
			create_enabled=Node_tool_get_create_enabled(node_tool);
			define_enabled=Node_tool_get_define_enabled(node_tool);
			edit_enabled=Node_tool_get_edit_enabled(node_tool);
			motion_update_enabled=Node_tool_get_motion_update_enabled(node_tool);
			select_enabled=Node_tool_get_select_enabled(node_tool);
			streaming_create_enabled =
				 Node_tool_get_streaming_create_enabled(node_tool);
			constrain_to_surface =
				 Node_tool_get_constrain_to_surface(node_tool);
			command_field=Node_tool_get_command_field(node_tool);
			Node_tool_get_region_path(node_tool, &region_path);
#if defined (WX_USER_INTERFACE)
		if (!data_tool_flag)
		{
			 element_create_enabled = Node_tool_get_element_create_enabled(node_tool);
			 element_dimension =
					Node_tool_get_element_dimension(node_tool);
		}
#endif /*(WX_USER_INTERFACE)*/
		}
		if (coordinate_field)
		{
			ACCESS(Computed_field)(coordinate_field);
		}
		if (command_field)
		{
			ACCESS(Computed_field)(command_field);
		}

		option_table=CREATE(Option_table)();
		/* coordinate_field */
		set_coordinate_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_coordinate_field_data.conditional_function=
			Computed_field_has_up_to_3_numerical_components;
		set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"coordinate_field",&coordinate_field,
			&set_coordinate_field_data,set_Computed_field_conditional);
		/* constrain_to_surfaces/no_constrain_to_surfaces */
		Option_table_add_switch(option_table,"constrain_to_surfaces","no_constrain_to_surfaces",
			&constrain_to_surface);
		/* create/no_create */
		Option_table_add_switch(option_table,"create","no_create",&create_enabled);
		/* define/no_define */
		Option_table_add_switch(option_table,"define","no_define",&define_enabled);
#if defined (WX_USER_INTERFACE)
		if (!data_tool_flag)
		{
			 /* create/no_create */
			 Option_table_add_switch(option_table,"element_create","no_element_create",&element_create_enabled);
			 /* element_dimension*/
			 Option_table_add_entry(option_table,"element_dimension",
					&element_dimension,NULL,set_int_non_negative);
		}
#endif /*(WX_USER_INTERFACE)*/
		/* edit/no_edit */
		Option_table_add_switch(option_table,"edit","no_edit",&edit_enabled);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			root_region, set_Cmiss_region_path);
		/* motion_update/no_motion_update */
		Option_table_add_switch(option_table,"motion_update","no_motion_update",
			&motion_update_enabled);
		/* open_dialog/close_dialog */
		dialog_string = (char *)NULL;
		Option_table_add_enumerator(option_table, /*number_of_valid_strings*/2,
			dialog_strings, &dialog_string);
		/* select/no_select */
		Option_table_add_switch(option_table,"select","no_select",&select_enabled);
		/* streaming_create/no_streaming_create */
		Option_table_add_switch(option_table, "streaming_create",
			"no_streaming_create", &streaming_create_enabled);
		/* command_field */
		set_command_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_command_field_data.conditional_function =
			Computed_field_has_string_value_type;
		set_command_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"command_field",&command_field,
			&set_command_field_data,set_Computed_field_conditional);
		if (return_code = Option_table_multi_parse(option_table,state))
		{
			if (node_tool)
			{
				if (dialog_string == dialog_strings[1])
				{
					Node_tool_pop_down_dialog(node_tool);
				}
				Node_tool_set_coordinate_field(node_tool,coordinate_field);
				Node_tool_set_region_path(node_tool,region_path);
				Node_tool_set_streaming_create_enabled(node_tool,
					streaming_create_enabled);
				Node_tool_set_command_field(node_tool,command_field);

				/* Set the state after setting the parameters as some of them
				   states rely on these parameters */
				Node_tool_set_edit_enabled(node_tool,edit_enabled);
				Node_tool_set_select_enabled(node_tool,select_enabled);
				Node_tool_set_define_enabled(node_tool,define_enabled);
				Node_tool_set_create_enabled(node_tool,create_enabled);
				Node_tool_set_constrain_to_surface(node_tool,constrain_to_surface);
				Node_tool_set_motion_update_enabled(node_tool,motion_update_enabled);
#if defined (WX_USER_INTERFACE)
		if (!data_tool_flag)
		{
			 Node_tool_set_element_dimension(node_tool,element_dimension);
			 Node_tool_set_element_create_enabled(node_tool,element_create_enabled);
		}
#endif /*(WX_USER_INTERFACE)*/ 
				if (dialog_string == dialog_strings[0])
				{
#if ! defined (WX_USER_INTERFACE)
					 Node_tool_pop_up_dialog(node_tool, (struct Graphics_window *)NULL);
#else /* defined (WX_USER_INTERFACE) */
					 display_message(WARNING_MESSAGE,
							"This command changes the node tool settings for each window to the global settings. To change node tool settings for individual window, please see the command [gfx modify window <name> nodes ?]. \n");
#endif /* defined (WX_USER_INTERFACE) */
				}
#if defined (WX_USER_INTERFACE)
				FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
					 Graphics_window_update_Interactive_tool,
					 (void *)Node_tool_get_interactive_tool(node_tool),
					 command_data->graphics_window_manager);
				display_message(WARNING_MESSAGE,
					 "This command changes the node tool settings for each window to the global settings. To change node tool settings for individual window, please see the command [gfx modify window <name> nodes ?]. \n");
#endif /*(WX_USER_INTERFACE)*/
				Cmiss_scene_viewer_package_update_Interactive_tool(
					command_data->scene_viewer_package,
					Node_tool_get_interactive_tool(node_tool));
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_gfx_node_tool.  Missing node/data tool");
				return_code=0;
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
		if (command_field)
		{
			DEACCESS(Computed_field)(&command_field);
		}
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_node_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_node_tool */
#endif /* defined (MOTIF) || (GTK_USER_INTERFACE) || defined
			  (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined(WX_USER_INTERFACE */

#if defined (MOTIF) || (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (WX_USER_INTERFACE)
static int execute_command_gfx_print(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 23 April 2002

DESCRIPTION :
Executes a GFX PRINT command.
==============================================================================*/
{
	char *file_name, force_onscreen_flag, *image_file_format_string,
		**valid_strings;
	enum Image_file_format image_file_format;
	enum Texture_storage_type storage;
	int antialias, height, number_of_valid_strings, return_code, 
		transparency_layers, width;
	struct Cmgui_image *cmgui_image;
	struct Cmgui_image_information *cmgui_image_information;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_print);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		/* initialize defaults */
		antialias = 0;
		file_name = (char *)NULL;
		height = 0;
		force_onscreen_flag = 0;
		storage = TEXTURE_RGBA;
		transparency_layers = 0;
		width = 0;
		/* default file format is to obtain it from the filename extension */
		image_file_format = UNKNOWN_IMAGE_FILE_FORMAT;
		/* must have at least one graphics_window to print */
		if (window = FIRST_OBJECT_IN_MANAGER_THAT(
			Graphics_window)((MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL, command_data->graphics_window_manager))
		{
			ACCESS(Graphics_window)(window);
		}

		option_table = CREATE(Option_table)();
		/* antialias */
		Option_table_add_entry(option_table, "antialias",
			&antialias, NULL, set_int_positive);
		/* image file format */
		image_file_format_string =
			ENUMERATOR_STRING(Image_file_format)(image_file_format);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Image_file_format)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Image_file_format) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
			valid_strings, &image_file_format_string);
		DEALLOCATE(valid_strings);
		/* file */
		Option_table_add_entry(option_table, "file", &file_name,
			(void *)1, set_name);
		/* force_onscreen */
		Option_table_add_entry(option_table, "force_onscreen",
			&force_onscreen_flag, NULL, set_char_flag);
		/* format */
		Option_table_add_entry(option_table, "format", &storage,
			NULL, set_Texture_storage);
		/* height */
		Option_table_add_entry(option_table, "height",
			&height, NULL, set_int_non_negative);
		/* transparency_layers */
		Option_table_add_entry(option_table, "transparency_layers",
			&transparency_layers, NULL, set_int_positive);
		/* width */
		Option_table_add_entry(option_table, "width",
			&width, NULL, set_int_non_negative);
		/* window */
		Option_table_add_entry(option_table, "window",
			&window, command_data->graphics_window_manager, set_Graphics_window);

		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (!file_name)
			{
				if (!(file_name = confirmation_get_write_filename(NULL,
								 command_data->user_interface
#if defined (WX_USER_INTERFACE)
							 , command_data->execute_command
#endif /* defined (WX_USER_INTERFACE)*/
)))
				{
					display_message(ERROR_MESSAGE, "gfx print:  No file name specified");
					return_code = 0;
				}					
			}
			if (!window)
			{
				display_message(ERROR_MESSAGE,
					"gfx print:  No graphics windows to print");
				return_code = 0;
			}
		}
		if (return_code)
		{
			cmgui_image_information = CREATE(Cmgui_image_information)();
			if (image_file_format_string)
			{
				STRING_TO_ENUMERATOR(Image_file_format)(
					image_file_format_string, &image_file_format);
			}
			else
			{
				image_file_format = UNKNOWN_IMAGE_FILE_FORMAT;
			}
			Cmgui_image_information_set_image_file_format(
				cmgui_image_information, image_file_format);
			Cmgui_image_information_add_file_name(cmgui_image_information,
				file_name);
			Cmgui_image_information_set_io_stream_package(cmgui_image_information,
				command_data->io_stream_package);
			if (cmgui_image = Graphics_window_get_image(window,
				force_onscreen_flag, width, height, antialias,
				transparency_layers, storage))
			{
				if (!Cmgui_image_write(cmgui_image, cmgui_image_information))
				{
					display_message(ERROR_MESSAGE,
						"gfx print:  Error writing image %s", file_name);
					return_code = 0;
				}
				DESTROY(Cmgui_image)(&cmgui_image);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_gfx_print.  Could not get image from window");
				return_code = 0;
			}
			DESTROY(Cmgui_image_information)(&cmgui_image_information);
		}
		if (window)
		{
			DEACCESS(Graphics_window)(&window);
		}
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_print.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_print */
#endif /* defined (MOTIF) */

static int gfx_read_Curve(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 January 2002

DESCRIPTION :
Reads a curve from 3 files: ~.curve.com, ~.curve.exnode, ~.curve.exelem, where
~ is the name of the curve/file specified here.
Works by executing the .curve.com file, which should have a gfx define curve
instruction to read in the mesh.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(gfx_read_Curve);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data = (struct Cmiss_command_data *)command_data_void)
		{
			file_name = (char *)NULL;
			option_table = CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table, CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
				&file_name, &(command_data->example_directory),
				set_file_name);
			/* default */
			Option_table_add_entry(option_table, NULL, &file_name,
				NULL, set_file_name);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".curve.com",
									 command_data->user_interface
#if defined (WX_USER_INTERFACE)
									 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE)*/
																													 )))
					{
						return_code = 0;
					}
				}
#if defined (WX_USER_INTERFACE) &&  (__WIN32__)
			if (file_name)
			{
				 file_name = CMISS_set_directory_and_filename_WIN32(file_name, 
						command_data); 
			}
#endif /* defined (__WIN32__)*/
				if (return_code)
				{
					/* open the file */
					if (return_code=check_suffix(&file_name,".curve.com"))
					{
						return_code=execute_comfile(file_name, command_data->io_stream_package,
							 command_data->execute_command);
					}
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_read_Curve.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_Curve.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_Curve */

static int gfx_read_elements(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
If an element file is not specified a file selection box is presented to the
user, otherwise the elements file is read.
==============================================================================*/
{
	char *file_name, generate_faces_and_lines_flag, *region_path, 
		element_flag, face_flag, line_flag, node_flag;
	int element_offset, face_offset, line_offset, node_offset,
		return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region, *top_region;
	struct FE_region *fe_region;
	struct IO_stream *input_file;
	struct Option_table *option_table;

	ENTER(gfx_read_elements);
	USE_PARAMETER(dummy_to_be_modified);
	input_file = NULL;
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		element_flag = 0;
		element_offset = 0;
		face_flag = 0;
		face_offset = 0;
		generate_faces_and_lines_flag = 0;
		line_flag = 0;
		line_offset = 0;
		node_flag = 0;
		node_offset = 0;
		file_name = (char *)NULL;
		region_path = (char *)NULL;
		option_table = CREATE(Option_table)();
		/* element_offset */
		Option_table_add_entry(option_table, "element_offset", &element_offset,
			&element_flag, set_int_and_char_flag);
		/* example */
		Option_table_add_entry(option_table,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
			&file_name, &(command_data->example_directory), set_file_name);
		/* face_offset */
		Option_table_add_entry(option_table, "face_offset", &face_offset,
			&face_flag, set_int_and_char_flag);
		/* generate_faces_and_lines */
		Option_table_add_char_flag_entry(option_table, "generate_faces_and_lines",
			&generate_faces_and_lines_flag);
		/* line_offset */
		Option_table_add_entry(option_table, "line_offset", &line_offset,
			&line_flag, set_int_and_char_flag);
		/* node_offset */
		Option_table_add_entry(option_table, "node_offset", &node_offset,
			&node_flag, set_int_and_char_flag);
		/* region */
		Option_table_add_entry(option_table,"region",
			&region_path, (void *)1, set_name);
		/* default */
		Option_table_add_entry(option_table,NULL,&file_name,
			NULL,set_file_name);
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code)
		{
			if (!file_name)
			{
				if (!(file_name = confirmation_get_read_filename(".exelem",
								 command_data->user_interface
#if defined(WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */				
		 )))	 
				{
					return_code = 0;
				}
			}
#if defined (WX_USER_INTERFACE) &&  (__WIN32__)
			if (file_name)
			{
				 file_name = CMISS_set_directory_and_filename_WIN32(file_name,
						command_data); 
			}
#endif /* defined (__WIN32__)*/

			if (return_code)
			{
				if (!check_suffix(&file_name,".exelem"))
				{
					return_code = 0;
				}
			}
			if (region_path)
			{
				if (!(Cmiss_region_get_region_from_path(command_data->root_region,
					region_path, &top_region) && top_region))
				{
					if (top_region = CREATE(Cmiss_region)())
					{
						if (fe_region=CREATE(FE_region)((struct FE_region *)NULL,
							(struct MANAGER(FE_basis) *)NULL, (struct LIST(FE_element_shape) *)NULL))
						{
							if (Cmiss_region_attach_FE_region(top_region,fe_region))
							{
								Cmiss_region_add_child_region(command_data->root_region, top_region, region_path,
									/*child_position*/-1);
							}
							else
							{
								display_message(ERROR_MESSAGE, "gfx_read_nodes.  "
									"Unable to attach new region.");
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE, "gfx_read_nodes.  "
								"Unable to make new finite element region.");
							return_code = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE, "gfx_read_nodes.  "
							"Unable to make new region, basis_manager or element_shape_list.");
						return_code = 0;
					}
				}
			}
			else
			{
				top_region = command_data->root_region;
			}
			if (return_code)
			{
				/* open the file */
				if ((input_file = CREATE(IO_stream)(command_data->io_stream_package))
					&& (IO_stream_open_for_read(input_file, file_name)))
				{
					if (region = read_exregion_file(input_file,
						command_data->basis_manager, command_data->element_shape_list,
						(struct FE_import_time_index *)NULL))
					{
						if (element_flag || face_flag || line_flag || node_flag)
						{
							/* Offset these nodes and elements before merging */
							if (fe_region = Cmiss_region_get_FE_region(region))
							{
								FE_region_begin_change(fe_region);
								if (element_flag)
								{
									if (!FE_region_change_element_identifiers(fe_region,
											CM_ELEMENT,	element_offset, 
											(struct Computed_field *)NULL, /*time*/0))
									{
										return_code = 0;
									}
								}
								if (face_flag)
								{
									if (!FE_region_change_element_identifiers(fe_region,
											CM_FACE,	face_offset,
											(struct Computed_field *)NULL, /*time*/0))
									{
										return_code = 0;
									}
								}
								if (line_flag)
								{
									if (!FE_region_change_element_identifiers(fe_region,
											CM_LINE,	line_offset,
											(struct Computed_field *)NULL, /*time*/0))
									{
										return_code = 0;
									}
								}
								if (node_flag)
								{
									if (!FE_region_change_node_identifiers(fe_region,
											node_offset,
											(struct Computed_field *)NULL, /*time*/0))
									{
										return_code = 0;
									}
								}
								FE_region_end_change(fe_region);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Unable to get fe_region to offset nodes or elements in file %s.",
									file_name);
								return_code = 0;
							}
						}
						if (return_code)
						{
							ACCESS(Cmiss_region)(region);
							if (generate_faces_and_lines_flag)
							{
								fe_region = Cmiss_region_get_FE_region(top_region);
								FE_region_begin_define_faces(fe_region);
							}
							if (Cmiss_regions_FE_regions_can_be_merged(
									 top_region, region))
							{
								if (!Cmiss_regions_merge_FE_regions(
										 top_region, region))
								{
									display_message(ERROR_MESSAGE,
										"Error merging elements from file: %s", file_name);
									return_code = 0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Contents of file %s not compatible with global objects",
									file_name);
								return_code = 0;
							}
							if (generate_faces_and_lines_flag)
							{
								FE_region_end_define_faces(fe_region);
							}
							DEACCESS(Cmiss_region)(&region);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Error reading element file: %s", file_name);
						return_code = 0;
					}
					IO_stream_close(input_file);
					DESTROY(IO_stream)(&input_file);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Could not open element file: %s", file_name);
					return_code = 0;
				}
			}
		}
		DESTROY(Option_table)(&option_table);
#if defined (WX_USER_INTERFACE)
		if (input_file)
			 DESTROY(IO_stream)(&input_file); 
#endif /*defined (WX_USER_INTERFACE)*/
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_read_elements.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_elements */

static int gfx_read_nodes(struct Parse_state *state,
	void *use_data, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
If a nodes file is not specified a file selection box is presented to the user,
otherwise the nodes file is read.
If the <use_data> flag is set, then read data, otherwise nodes.
==============================================================================*/
{
	char *file_name, node_offset_flag, *region_path, time_set_flag;
	double maximum, minimum;
	float time;
	int node_offset, return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region, *top_region;
	struct FE_import_time_index *node_time_index, node_time_index_data;
	struct FE_region *fe_region;
	struct IO_stream *input_file;
	struct Option_table *option_table;

	ENTER(gfx_read_nodes);
	input_file=NULL;
	if (state)
	{
		if (command_data = (struct Cmiss_command_data *)command_data_void)
		{
			file_name = (char *)NULL;
			node_offset_flag = 0;
			node_offset = 0;
			region_path = (char *)NULL;
			time = 0;
			time_set_flag = 0;
			node_time_index = (struct FE_import_time_index *)NULL;
			option_table=CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
				&file_name, &(command_data->example_directory), set_file_name);
			if (!use_data)
			{
				/* node_offset */
				Option_table_add_entry(option_table, "node_offset", &node_offset,
					&node_offset_flag, set_int_and_char_flag);
			}
			else
			{
				/* data_offset */
				Option_table_add_entry(option_table, "data_offset", &node_offset,
					&node_offset_flag, set_int_and_char_flag);
			}
			if (!use_data)
			{
				/* region */
				Option_table_add_entry(option_table,"region",
					&region_path, (void *)1, set_name);
			}
			/* time */
			Option_table_add_entry(option_table,"time",
				&time, &time_set_flag, set_float_and_char_flag);
			/* default */
			Option_table_add_entry(option_table, NULL, &file_name,
				NULL, set_file_name);
			return_code = Option_table_multi_parse(option_table,state);
			if (return_code)
			{
				if (!file_name)
				{
					if (use_data)
					{
						if (!(file_name = confirmation_get_read_filename(".exdata",
										 command_data->user_interface
#if defined(WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																														 )))
						{
							return_code = 0;
						}
					}
					else
					{
						if (!(file_name = confirmation_get_read_filename(".exnode",
							command_data->user_interface
#if defined(WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																														 )))
						{
							return_code = 0;
						}
					}
				}
				if (time_set_flag)
				{
					node_time_index_data.time = time;
					node_time_index = &node_time_index_data;
				}
				if (return_code)
				{
#if defined (WX_USER_INTERFACE) &&  (__WIN32__)
			if (file_name)
			{
				 file_name = CMISS_set_directory_and_filename_WIN32(file_name,
						command_data); 
			}
#endif /* defined (__WIN32__)*/
					/* open the file */
					if (use_data)
					{
						return_code = check_suffix(&file_name,".exdata");
					}
					else
					{
						return_code = check_suffix(&file_name,".exnode");
					}
					if (region_path)
					{
						if (!(Cmiss_region_get_region_from_path(command_data->root_region,
							region_path, &top_region) && top_region))
						{
							if (top_region = CREATE(Cmiss_region)())
							{
								if (fe_region=CREATE(FE_region)((struct FE_region *)NULL,
									(struct MANAGER(FE_basis) *)NULL, (struct LIST(FE_element_shape) *)NULL))
								{
									if (Cmiss_region_attach_FE_region(top_region,fe_region))
									{
										Cmiss_region_add_child_region(command_data->root_region, top_region, region_path,
											/*child_position*/-1);
									}
									else
									{
										display_message(ERROR_MESSAGE, "gfx_read_nodes.  "
											"Unable to attach new region.");
										return_code = 0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE, "gfx_read_nodes.  "
										"Unable to make new finite element region.");
									return_code = 0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE, "gfx_read_nodes.  "
									"Unable to make new region, basis_manager or element_shape_list.");
								return_code = 0;
							}
						}
					}
					else
					{
						if (use_data)
						{
							top_region = command_data->data_root_region;
						}
						else
						{
							top_region = command_data->root_region;
						}
					}
					if (return_code)
					{
						if ((input_file = CREATE(IO_stream)(command_data->io_stream_package))
							&& (IO_stream_open_for_read(input_file, file_name)))
						{
							if (region = read_exregion_file(input_file,
								command_data->basis_manager, command_data->element_shape_list,
								node_time_index))
							{
								if (node_offset_flag)
								{
									/* Offset these nodes before merging */
									if (fe_region = Cmiss_region_get_FE_region(region))
									{
										FE_region_begin_change(fe_region);
										if (!FE_region_change_node_identifiers(fe_region,
												node_offset,
												(struct Computed_field *)NULL, time))
										{
											return_code = 0;
										}
								
										FE_region_end_change(fe_region);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"Unable to get fe_region to offset nodes in file %s.",
											file_name);
										return_code = 0;
									}
								}
								ACCESS(Cmiss_region)(region);
								if (Cmiss_regions_FE_regions_can_be_merged(
									top_region, region))
								{
									if (!Cmiss_regions_merge_FE_regions(
										top_region, region))
									{
										if (use_data)
										{
											display_message(ERROR_MESSAGE,
												"Error merging data from file: %s", file_name);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"Error merging nodes from file: %s", file_name);
										}
										return_code = 0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Contents of file %s not compatible with global objects",
										file_name);
									return_code = 0;
								}
								DEACCESS(Cmiss_region)(&region);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Error reading node file: %s", file_name);
								return_code = 0;
							}
							IO_stream_close(input_file);
							DESTROY(IO_stream)(&input_file);
							input_file =NULL;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Could not open node file: %s", file_name);
							return_code = 0;
						}
					}
					if (return_code && time_set_flag)
					{
						/* Increase the range of the default time keepeer and set the
						   minimum and maximum if we set anything */
						maximum = Time_keeper_get_maximum(
							command_data->default_time_keeper);
						minimum = Time_keeper_get_minimum(
							command_data->default_time_keeper);
						if (time < minimum)
						{
							Time_keeper_set_minimum(
								command_data->default_time_keeper, time);
							Time_keeper_set_maximum(
								command_data->default_time_keeper, maximum);
						}
						if (time > maximum)
						{
							Time_keeper_set_minimum(
								command_data->default_time_keeper, minimum);
							Time_keeper_set_maximum(
								command_data->default_time_keeper, time);
						}
					}
				}
			}
			DESTROY(Option_table)(&option_table);
#if defined (WX_USER_INTERFACE)
			if (input_file)
				 DESTROY(IO_stream)(&input_file);
			input_file =NULL;
#endif /*defined (WX_USER_INTERFACE)*/
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
			if (region_path)
			{
				DEALLOCATE(region_path);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_read_nodes.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_nodes.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_nodes */

static int gfx_read_objects(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 January 2003

DESCRIPTION :
If a file is not specified a file selection box is presented to the user,
otherwise the file of graphics objects is read.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(gfx_read_objects);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data = (struct Cmiss_command_data *)command_data_void)
		{
			file_name = (char *)NULL;
			option_table=CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
				&file_name, &(command_data->example_directory), set_file_name);
			/* default */
			Option_table_add_entry(option_table,NULL,&file_name,
				NULL,set_file_name);
			return_code = Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".exgobj",
						command_data->user_interface
#if defined(WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																													 )))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* open the file */
					if (return_code=check_suffix(&file_name,".exgobj"))
					{
						return_code=file_read_graphics_objects(file_name, command_data->io_stream_package,
							Material_package_get_material_manager(command_data->material_package),
							command_data->graphics_object_list);
					}
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_read_objects.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_objects.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_objects */

#if defined (HAVE_XML2)
static int gfx_read_region(struct Parse_state *state,
	void *dummy, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2003

DESCRIPTION :
If a nodes file is not specified a file selection box is presented to the user,
otherwise the nodes file is written.
Can now specify individual node groups to read with the <group> option.
If <use_data> is set, writing data, otherwise writing nodes.
==============================================================================*/
{
	char file_ext[] = ".fml", *file_name, *region_path;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct FE_field_order_info *field_order_info;
	struct Option_table *option_table;

	ENTER(gfx_read_region);
	USE_PARAMETER(dummy);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		return_code = 1;
		region_path = (char *)NULL;
		field_order_info = (struct FE_field_order_info *)NULL;
		file_name = (char *)NULL;

		option_table = CREATE(Option_table)();
		/* fields */
		Option_table_add_entry(option_table, "fields", &field_order_info,
			Cmiss_region_get_FE_region(command_data->root_region),
			set_FE_fields_FE_region);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* default option: file name */
		Option_table_add_entry(option_table, (char *)NULL, &file_name,
			NULL, set_name);

		if (return_code = Option_table_multi_parse(option_table, state))
		{
			if (!file_name)
			{
				if (!(file_name = confirmation_get_read_filename(file_ext,
					command_data->user_interface
#if defined(WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																												 )))
				{
					return_code = 0;
				}
			}
			if (region_path)
			{
				display_message(WARNING_MESSAGE,
					"gfx read region: 'group' option is unused");
			}
			if (return_code)
			{
				if (region = parse_fieldml_file(file_name, command_data->basis_manager,
					command_data->element_shape_list))
				{
					ACCESS(Cmiss_region)(region);
					if (Cmiss_regions_FE_regions_can_be_merged(
						command_data->root_region, region))
					{
						if (!Cmiss_regions_merge_FE_regions(
							command_data->root_region, region))
						{
							display_message(ERROR_MESSAGE,
								"Error merging elements from region file: %s", file_name);
							return_code = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Contents of region file %s not compatible with global objects",
							file_name);
						return_code = 0;
					}
					DEACCESS(Cmiss_region)(&region);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Error reading region file: %s", file_name);
					return_code = 0;
				}
			}
		}
		DESTROY(Option_table)(&option_table);
		if (field_order_info)
		{
			DESTROY(FE_field_order_info)(&field_order_info);
		}
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_read_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_region */
#endif /* defined (HAVE_XML2) */

static int gfx_read_wavefront_obj(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
If a file is not specified a file selection box is presented to the user,
otherwise the wavefront obj file is read.
==============================================================================*/
{
	char *file_name, *graphics_object_name, *render_type_string, 
		*specified_graphics_object_name, **valid_strings;
	enum Render_type render_type;
	float time;
	int number_of_valid_strings, return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(gfx_read_wavefront_obj);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct Cmiss_command_data *)command_data_void))
		{
			specified_graphics_object_name=(char *)NULL;
			graphics_object_name=(char *)NULL;
			time = 0;
			file_name=(char *)NULL;

			option_table=CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
			  &file_name, &(command_data->example_directory), set_file_name);
			/* as */
			Option_table_add_entry(option_table,"as",&specified_graphics_object_name,
				(void *)1,set_name);
			/* render_type */
			render_type = RENDER_TYPE_SHADED;
			render_type_string = ENUMERATOR_STRING(Render_type)(render_type);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Render_type)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Render_type) *)NULL, (void *)NULL);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&render_type_string);
			DEALLOCATE(valid_strings);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* default */
			Option_table_add_entry(option_table,NULL,&file_name,
				NULL,set_file_name);

			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				STRING_TO_ENUMERATOR(Render_type)(render_type_string, &render_type);
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".obj",
						command_data->user_interface
#if defined(WX_USER_INTERFACE)
									 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																													 )))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					if(specified_graphics_object_name)
					{
						graphics_object_name = specified_graphics_object_name;
					}
					else
					{
						graphics_object_name = file_name;
					}
					/* open the file */
					if (return_code=check_suffix(&file_name,".obj"))
					{
						return_code=file_read_voltex_graphics_object_from_obj(file_name,
							command_data->io_stream_package,
							graphics_object_name, render_type, time, 
							Material_package_get_material_manager(command_data->material_package),
							Material_package_get_default_material(command_data->material_package),
							command_data->graphics_object_list);
					}
				}
			}
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
			if (specified_graphics_object_name)
			{
				DEALLOCATE(specified_graphics_object_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_read_wavefront_obj.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_wavefront_obj.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_wavefront_obj */

static int execute_command_gfx_read(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 January 2003

DESCRIPTION :
Executes a GFX READ command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_read);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			option_table = CREATE(Option_table)();
			/* curve */
			Option_table_add_entry(option_table, "curve",
				NULL, command_data_void, gfx_read_Curve);
			/* data */
			Option_table_add_entry(option_table, "data",
				/*use_data*/(void *)1, command_data_void, gfx_read_nodes);
			/* elements */
			Option_table_add_entry(option_table, "elements",
				NULL, command_data_void, gfx_read_elements);
			/* nodes */
			Option_table_add_entry(option_table, "nodes",
				/*use_data*/(void *)0, command_data_void, gfx_read_nodes);
			/* objects */
			Option_table_add_entry(option_table, "objects",
				NULL, command_data_void, gfx_read_objects);
#if defined (HAVE_XML2)
			/* region */
			Option_table_add_entry(option_table, "region",
				NULL, command_data_void, gfx_read_region);
#endif /* defined (HAVE_XML2) */
			/* wavefront_obj */
			Option_table_add_entry(option_table, "wavefront_obj",
				NULL, command_data_void, gfx_read_wavefront_obj);
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx read", command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_read.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_read */

static int execute_command_gfx_select(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Executes a GFX SELECT command.
==============================================================================*/
{
	char all_flag,data_flag,elements_flag,faces_flag,grid_points_flag,
		lines_flag,nodes_flag, *region_path, selected_flag, verbose_flag;
	FE_value time;
	int return_code;
	struct Computed_field *conditional_field;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *data_region, *region;
	struct Element_point_ranges *element_point_ranges;
	struct FE_element_grid_to_Element_point_ranges_list_data grid_to_list_data;
	struct FE_field *grid_field;
	struct FE_region *data_fe_region, *fe_region;
	struct LIST(FE_element) *element_list;
	struct LIST(FE_node) *node_list;
	struct Multi_range *multi_range;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_conditional_field_data;
	struct Set_FE_field_conditional_FE_region_data set_grid_field_data;

	ENTER(execute_command_gfx_select);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			Cmiss_region_get_root_region_path(&region_path);
			fe_region = Cmiss_region_get_FE_region(command_data->root_region);
			all_flag = 0;
			conditional_field=(struct Computed_field *)NULL;
			data_flag = 0;
			element_point_ranges=(struct Element_point_ranges *)NULL;
			data_flag = 0;
			elements_flag = 0;
			faces_flag = 0;
			grid_points_flag = 0;
			lines_flag = 0;
			nodes_flag = 0;
			/* With the current method the selection is always additive
				and so to set the selected flag makes the command useless */
			selected_flag = 0;
			multi_range=CREATE(Multi_range)();
			if ((grid_field = FE_region_get_FE_field_from_name(fe_region,
				"grid_point_number")) &&
				FE_field_is_1_component_integer(grid_field, (void *)NULL))
			{
				ACCESS(FE_field)(grid_field);
			}
			else
			{
				grid_field = (struct FE_field *)NULL;
			}
			if (command_data->default_time_keeper)
			{
				time = Time_keeper_get_time(command_data->default_time_keeper);
			}
			else
			{
				time = 0.0;
			}
			verbose_flag = 0;

			option_table=CREATE(Option_table)();
			/* all */
			Option_table_add_entry(option_table,"all", &all_flag,
				(void *)NULL,set_char_flag);
			/* conditional_field */
			set_conditional_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
			set_conditional_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_conditional_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"conditional_field",
				&conditional_field,&set_conditional_field_data,
				set_Computed_field_conditional);
			/* data */
			Option_table_add_entry(option_table,"data", &data_flag,
				(void *)NULL,set_char_flag);
			/* elements */
			Option_table_add_entry(option_table,"elements",&elements_flag,
				(void *)NULL,set_char_flag);
			/* faces */
			Option_table_add_entry(option_table,"faces",&faces_flag,
				(void *)NULL,set_char_flag);
			/* grid_field */
			set_grid_field_data.conditional_function =
				FE_field_is_1_component_integer;
			set_grid_field_data.user_data = (void *)NULL;
			set_grid_field_data.fe_region = fe_region;
			Option_table_add_entry(option_table, "grid_field", &grid_field,
				(void *)&set_grid_field_data, set_FE_field_conditional_FE_region);
			/* grid_points */
			Option_table_add_entry(option_table,"grid_points",&grid_points_flag,
				(void *)NULL,set_char_flag);
			/* group */
			Option_table_add_entry(option_table, "group", &region_path,
				command_data->root_region, set_Cmiss_region_path);
			/* lines */
			Option_table_add_entry(option_table,"lines",&lines_flag,
				(void *)NULL,set_char_flag);
			/* nodes */
			Option_table_add_entry(option_table,"nodes",&nodes_flag,
				(void *)NULL,set_char_flag);
			/* points */
			Option_table_add_entry(option_table,"points",&element_point_ranges,
				(void *)fe_region, set_Element_point_ranges);
			/* verbose */
			Option_table_add_char_flag_entry(option_table,"verbose",
				&verbose_flag);
			/* default option: multi range */
			Option_table_add_entry(option_table, (char *)NULL, (void *)multi_range,
				NULL, set_Multi_range);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				if ((data_flag + elements_flag + faces_flag + grid_points_flag
					+ lines_flag + nodes_flag) != 1)
				{
					display_message(ERROR_MESSAGE,"gfx select:  "
						"You must specify one and only one of "
						"data/elements/faces/lines/grid_points/nodes.");
					return_code = 0;
				}
				if (!(region_path &&
					Cmiss_region_get_region_from_path(command_data->root_region,
						region_path, &region) &&
					(fe_region = Cmiss_region_get_FE_region(region)) &&
					Cmiss_region_get_region_from_path(command_data->data_root_region,
						region_path, &data_region) &&
					(data_fe_region = Cmiss_region_get_FE_region(data_region))))
				{
					display_message(ERROR_MESSAGE, "gfx select:  Invalid region");
					return_code = 0;
				}
			}
			if (return_code)
			{
				/* data */
				if (data_flag)
				{
					if (node_list =
						FE_node_list_from_fe_region_selection_ranges_condition(
							data_fe_region, command_data->data_selection, selected_flag,
							multi_range, conditional_field, time))
					{
						FE_node_selection_begin_cache(command_data->data_selection);
						if (return_code = FOR_EACH_OBJECT_IN_LIST(FE_node)(
							FE_node_select_in_FE_node_selection,
							(void *)command_data->data_selection, node_list))
						{
							if (verbose_flag)
							{
								display_message(INFORMATION_MESSAGE,
									"Selected %d data points.\n",
									NUMBER_IN_LIST(FE_node)(node_list));
							}
						}
						else
						{
							display_message(ERROR_MESSAGE, 
								"execute_command_gfx_select.  Problem selecting nodes.");
						}
						FE_node_selection_end_cache(command_data->data_selection);
						DESTROY(LIST(FE_node))(&node_list);
					}
				}
				/* element_points */
				if (element_point_ranges)
				{
					Element_point_ranges_selection_select_element_point_ranges(
						command_data->element_point_ranges_selection,element_point_ranges);
				}
				/* elements */
				if (elements_flag)
				{
					if (element_list =
						FE_element_list_from_fe_region_selection_ranges_condition(
							fe_region, CM_ELEMENT, command_data->element_selection,
							selected_flag, multi_range, conditional_field, time))
					{
						FE_element_selection_begin_cache(command_data->element_selection);
						if (return_code = FOR_EACH_OBJECT_IN_LIST(FE_element)(
							FE_element_select_in_FE_element_selection,
							(void *)command_data->element_selection, element_list))
						{
							if (verbose_flag)
							{
								display_message(INFORMATION_MESSAGE,
									"Selected %d elements.\n",
									NUMBER_IN_LIST(FE_element)(element_list));
							}
						}
						else
						{
							display_message(ERROR_MESSAGE, 
								"execute_command_gfx_select.  Problem selecting nodes.");
						}
						FE_element_selection_end_cache(command_data->element_selection);
						DESTROY(LIST(FE_element))(&element_list);
					}
				}
				/* faces */
				if (faces_flag)
				{
					if (element_list =
						FE_element_list_from_fe_region_selection_ranges_condition(
							fe_region, CM_FACE, command_data->element_selection,
							selected_flag, multi_range, conditional_field, time))
					{
						FE_element_selection_begin_cache(command_data->element_selection);
						if (return_code = FOR_EACH_OBJECT_IN_LIST(FE_element)(
							FE_element_select_in_FE_element_selection,
							(void *)command_data->element_selection, element_list))
						{
							if (verbose_flag)
							{
								display_message(INFORMATION_MESSAGE,
									"Selected %d faces.\n",
									NUMBER_IN_LIST(FE_element)(element_list));
							}
						}
						else
						{
							display_message(ERROR_MESSAGE, 
								"execute_command_gfx_select.  Problem selecting nodes.");
						}
						FE_element_selection_end_cache(command_data->element_selection);
						DESTROY(LIST(FE_element))(&element_list);
					}
				}
				/* grid_points */
				if (grid_points_flag)
				{
					if (0<Multi_range_get_total_number_in_ranges(multi_range))
					{
						if (grid_field)
						{
							if (grid_to_list_data.element_point_ranges_list=
								CREATE(LIST(Element_point_ranges))())
							{
								grid_to_list_data.grid_fe_field=grid_field;
								grid_to_list_data.grid_value_ranges=multi_range;
								/* inefficient: go through every element in FE_region */
								FE_region_for_each_FE_element(fe_region,
									FE_element_grid_to_Element_point_ranges_list,
									(void *)&grid_to_list_data);
								if (0 < NUMBER_IN_LIST(Element_point_ranges)(
									grid_to_list_data.element_point_ranges_list))
								{
									Element_point_ranges_selection_begin_cache(
										command_data->element_point_ranges_selection);
									FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
										Element_point_ranges_select,
										(void *)command_data->element_point_ranges_selection,
										grid_to_list_data.element_point_ranges_list);
									Element_point_ranges_selection_end_cache(
										command_data->element_point_ranges_selection);
								}
								DESTROY(LIST(Element_point_ranges))(
									&(grid_to_list_data.element_point_ranges_list));
							}
							else
							{
								display_message(ERROR_MESSAGE,"execute_command_gfx_select.  "
									"Could not create grid_point list");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"To select grid_points, "
								"need integer grid_field (eg. grid_point_number)");
						}
					}
				}
				/* lines */
				if (lines_flag)
				{
					if (element_list =
						FE_element_list_from_fe_region_selection_ranges_condition(
							fe_region, CM_LINE, command_data->element_selection,
							selected_flag, multi_range, conditional_field, time))
					{
						FE_element_selection_begin_cache(command_data->element_selection);
						if (return_code = FOR_EACH_OBJECT_IN_LIST(FE_element)(
							FE_element_select_in_FE_element_selection,
							(void *)command_data->element_selection, element_list))
						{
							if (verbose_flag)
							{
								display_message(INFORMATION_MESSAGE,
									"Selected %d lines.\n",
									NUMBER_IN_LIST(FE_element)(element_list));
							}
						}
						else
						{
							display_message(ERROR_MESSAGE, 
								"execute_command_gfx_select.  Problem selecting nodes.");
						}
						FE_element_selection_end_cache(command_data->element_selection);
						DESTROY(LIST(FE_element))(&element_list);
					}
				}
				/* nodes */
				if (nodes_flag)
				{
					if (node_list =
						FE_node_list_from_fe_region_selection_ranges_condition(
							fe_region, command_data->node_selection, selected_flag,
							multi_range, conditional_field, time))
					{
						FE_node_selection_begin_cache(command_data->node_selection);
						if (return_code = FOR_EACH_OBJECT_IN_LIST(FE_node)(
							FE_node_select_in_FE_node_selection,
							(void *)command_data->node_selection, node_list))
						{
							if (verbose_flag)
							{
								display_message(INFORMATION_MESSAGE,
									"Selected %d nodes.\n",
									NUMBER_IN_LIST(FE_node)(node_list));
							}
						}
						else
						{
							display_message(ERROR_MESSAGE, 
								"execute_command_gfx_select.  Problem selecting nodes.");
						}
						FE_node_selection_end_cache(command_data->node_selection);
						DESTROY(LIST(FE_node))(&node_list);
					}
				}
			}
			DESTROY(Option_table)(&option_table);
			DEALLOCATE(region_path);
			if (conditional_field)
			{
				DEACCESS(Computed_field)(&conditional_field);
			}
			if (grid_field)
			{
				DEACCESS(FE_field)(&grid_field);
			}
			DESTROY(Multi_range)(&multi_range);
			if (element_point_ranges)
			{
				DESTROY(Element_point_ranges)(&element_point_ranges);
			}
		}
		else
		{
			set_command_prompt("gfx select",command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_select.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_select */

static int execute_command_gfx_unselect(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Executes a GFX UNSELECT command.
==============================================================================*/
{
	char all_flag,data_flag,elements_flag,faces_flag,grid_points_flag,
		lines_flag,nodes_flag, *region_path,selected_flag,verbose_flag;
	FE_value time;
	int return_code;
	struct Computed_field *conditional_field;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *data_region, *region;
	struct Element_point_ranges *element_point_ranges;
	struct FE_element_grid_to_Element_point_ranges_list_data grid_to_list_data;
	struct FE_field *grid_field;
	struct FE_region *data_fe_region, *fe_region;
	struct LIST(FE_element) *element_list;
	struct LIST(FE_node) *node_list;
	struct Multi_range *multi_range;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_conditional_field_data;
	struct Set_FE_field_conditional_FE_region_data set_grid_field_data;

	ENTER(execute_command_gfx_unselect);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			Cmiss_region_get_root_region_path(&region_path);
			fe_region = Cmiss_region_get_FE_region(command_data->root_region);
			all_flag = 0;
			conditional_field=(struct Computed_field *)NULL;
			data_flag = 0;
			element_point_ranges=(struct Element_point_ranges *)NULL;
			data_flag = 0;
			elements_flag = 0;
			faces_flag = 0;
			grid_points_flag = 0;
			lines_flag = 0;
			nodes_flag = 0;
			/* We only want to unselected from selected objects */
			selected_flag = 1;
			multi_range=CREATE(Multi_range)();
			if ((grid_field = FE_region_get_FE_field_from_name(fe_region,
				"grid_point_number")) &&
				FE_field_is_1_component_integer(grid_field, (void *)NULL))
			{
				ACCESS(FE_field)(grid_field);
			}
			else
			{
				grid_field = (struct FE_field *)NULL;
			}
			if (command_data->default_time_keeper)
			{
				time = Time_keeper_get_time(command_data->default_time_keeper);
			}
			else
			{
				time = 0.0;
			}
			verbose_flag = 0;

			option_table=CREATE(Option_table)();
			/* all */
			Option_table_add_entry(option_table,"all", &all_flag,
				(void *)NULL,set_char_flag);
			/* conditional_field */
			set_conditional_field_data.computed_field_manager=
				Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
			set_conditional_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_conditional_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"conditional_field",
				&conditional_field,&set_conditional_field_data,
				set_Computed_field_conditional);
			/* data */
			Option_table_add_entry(option_table,"data", &data_flag,
				(void *)NULL,set_char_flag);
			/* elements */
			Option_table_add_entry(option_table,"elements",&elements_flag,
				(void *)NULL,set_char_flag);
			/* faces */
			Option_table_add_entry(option_table,"faces",&faces_flag,
				(void *)NULL,set_char_flag);
			/* grid_field */
			set_grid_field_data.conditional_function =
				FE_field_is_1_component_integer;
			set_grid_field_data.user_data = (void *)NULL;
			set_grid_field_data.fe_region = fe_region;
			Option_table_add_entry(option_table, "grid_field", &grid_field,
				(void *)&set_grid_field_data, set_FE_field_conditional_FE_region);
			/* grid_points */
			Option_table_add_entry(option_table,"grid_points",&grid_points_flag,
				(void *)NULL,set_char_flag);
			/* group */
			Option_table_add_entry(option_table, "group", &region_path,
				command_data->root_region, set_Cmiss_region_path);
			/* lines */
			Option_table_add_entry(option_table,"lines",&lines_flag,
				(void *)NULL,set_char_flag);
			/* nodes */
			Option_table_add_entry(option_table,"nodes",&nodes_flag,
				(void *)NULL,set_char_flag);
			/* points */
			Option_table_add_entry(option_table,"points",&element_point_ranges,
				(void *)fe_region, set_Element_point_ranges);
			/* verbose */
			Option_table_add_char_flag_entry(option_table,"verbose",
				&verbose_flag);
			/* default option: multi range */
			Option_table_add_entry(option_table, (char *)NULL, (void *)multi_range,
				NULL, set_Multi_range);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				if ((data_flag + elements_flag + faces_flag + grid_points_flag
					+ lines_flag + nodes_flag) != 1)
				{
					display_message(ERROR_MESSAGE,"gfx unselect:  "
						"You must specify one and only one of "
						"data/elements/faces/lines/grid_points/nodes.");
					return_code = 0;
				}
				if (!(region_path &&
					Cmiss_region_get_region_from_path(command_data->root_region,
						region_path, &region) &&
					(fe_region = Cmiss_region_get_FE_region(region)) &&
					Cmiss_region_get_region_from_path(command_data->data_root_region,
						region_path, &data_region) &&
					(data_fe_region = Cmiss_region_get_FE_region(data_region))))
				{
					display_message(ERROR_MESSAGE, "gfx select:  Invalid region");
					return_code = 0;
				}
			}
			if (return_code)
			{
				/* data */
				if (data_flag)
				{
					if (node_list =
						FE_node_list_from_fe_region_selection_ranges_condition(
							data_fe_region, command_data->data_selection, selected_flag,
							multi_range, conditional_field, time))
					{
						FE_node_selection_begin_cache(command_data->data_selection);
						if (return_code = FOR_EACH_OBJECT_IN_LIST(FE_node)(
							FE_node_unselect_in_FE_node_selection,
							(void *)command_data->data_selection, node_list))
						{
							if (verbose_flag)
							{
								display_message(INFORMATION_MESSAGE,
									"Unselected %d data points.\n",
									NUMBER_IN_LIST(FE_node)(node_list));
							}
						}
						else
						{
							display_message(ERROR_MESSAGE, 
								"execute_command_gfx_unselect.  Problem unselecting nodes.");
						}
						FE_node_selection_end_cache(command_data->data_selection);
						DESTROY(LIST(FE_node))(&node_list);
					}
				}
				/* element_points */
				if (element_point_ranges)
				{
					Element_point_ranges_selection_unselect_element_point_ranges(
						command_data->element_point_ranges_selection,element_point_ranges);
				}
				/* elements */
				if (elements_flag)
				{
					if (element_list =
						FE_element_list_from_fe_region_selection_ranges_condition(
							fe_region, CM_ELEMENT, command_data->element_selection,
							selected_flag, multi_range, conditional_field, time))
					{
						FE_element_selection_begin_cache(command_data->element_selection);
						if (return_code = FOR_EACH_OBJECT_IN_LIST(FE_element)(
							FE_element_unselect_in_FE_element_selection,
							(void *)command_data->element_selection, element_list))
						{
							if (verbose_flag)
							{
								display_message(INFORMATION_MESSAGE,
									"Unselected %d elements.\n",
									NUMBER_IN_LIST(FE_element)(element_list));
							}
						}
						else
						{
							display_message(ERROR_MESSAGE, 
								"execute_command_gfx_unselect.  Problem unselecting elements.");
						}
						FE_element_selection_end_cache(command_data->element_selection);
						DESTROY(LIST(FE_element))(&element_list);
					}
				}
				/* faces */
				if (faces_flag)
				{
					if (element_list =
						FE_element_list_from_fe_region_selection_ranges_condition(
							fe_region, CM_FACE, command_data->element_selection,
							selected_flag, multi_range, conditional_field, time))
					{
						FE_element_selection_begin_cache(command_data->element_selection);
						if (return_code = FOR_EACH_OBJECT_IN_LIST(FE_element)(
							FE_element_unselect_in_FE_element_selection,
							(void *)command_data->element_selection, element_list))
						{
							if (verbose_flag)
							{
								display_message(INFORMATION_MESSAGE,
									"Unselected %d faces.\n",
									NUMBER_IN_LIST(FE_element)(element_list));
							}
						}
						else
						{
							display_message(ERROR_MESSAGE, 
								"execute_command_gfx_unselect.  Problem unselecting faces.");
						}
						FE_element_selection_end_cache(command_data->element_selection);
						DESTROY(LIST(FE_element))(&element_list);
					}
				}
				/* grid_points */
				if (grid_points_flag)
				{
					if (0<Multi_range_get_total_number_in_ranges(multi_range))
					{
						if (grid_field)
						{
							if (grid_to_list_data.element_point_ranges_list=
								CREATE(LIST(Element_point_ranges))())
							{
								grid_to_list_data.grid_fe_field=grid_field;
								grid_to_list_data.grid_value_ranges=multi_range;
								/* inefficient: go through every element in FE_region */
								FE_region_for_each_FE_element(fe_region,
									FE_element_grid_to_Element_point_ranges_list,
									(void *)&grid_to_list_data);
								if (0<NUMBER_IN_LIST(Element_point_ranges)(
									grid_to_list_data.element_point_ranges_list))
								{
									Element_point_ranges_selection_begin_cache(
										command_data->element_point_ranges_selection);
									FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
										Element_point_ranges_unselect,
										(void *)command_data->element_point_ranges_selection,
										grid_to_list_data.element_point_ranges_list);
									Element_point_ranges_selection_end_cache(
										command_data->element_point_ranges_selection);
								}
								DESTROY(LIST(Element_point_ranges))(
									&(grid_to_list_data.element_point_ranges_list));
							}
							else
							{
								display_message(ERROR_MESSAGE,"execute_command_gfx_unselect.  "
									"Could not create grid_point list");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"To unselect grid_points, "
								"need integer grid_field (eg. grid_point_number)");
						}
					}
				}
				/* lines */
				if (lines_flag)
				{
					if (element_list =
						FE_element_list_from_fe_region_selection_ranges_condition(
							fe_region, CM_LINE, command_data->element_selection,
							selected_flag, multi_range, conditional_field, time))
					{
						FE_element_selection_begin_cache(command_data->element_selection);
						if (return_code = FOR_EACH_OBJECT_IN_LIST(FE_element)(
							FE_element_unselect_in_FE_element_selection,
							(void *)command_data->element_selection, element_list))
						{
							if (verbose_flag)
							{
								display_message(INFORMATION_MESSAGE,
									"Unselected %d lines.\n",
									NUMBER_IN_LIST(FE_element)(element_list));
							}
						}
						else
						{
							display_message(ERROR_MESSAGE, 
								"execute_command_gfx_unselect.  Problem unselecting lines.");
						}
						FE_element_selection_end_cache(command_data->element_selection);
						DESTROY(LIST(FE_element))(&element_list);
					}
				}
				/* nodes */
				if (nodes_flag)
				{
					if (node_list =
						FE_node_list_from_fe_region_selection_ranges_condition(
							fe_region, command_data->node_selection, selected_flag,
							multi_range, conditional_field, time))
					{
						FE_node_selection_begin_cache(command_data->node_selection);
						if (return_code = FOR_EACH_OBJECT_IN_LIST(FE_node)(
							FE_node_unselect_in_FE_node_selection,
							(void *)command_data->node_selection, node_list))
						{
							if (verbose_flag)
							{
								display_message(INFORMATION_MESSAGE,
									"Unselected %d nodes.\n",
									NUMBER_IN_LIST(FE_node)(node_list));
							}
						}
						else
						{
							display_message(ERROR_MESSAGE, 
								"execute_command_gfx_unselect.  Problem unselecting nodes.");
						}
						FE_node_selection_end_cache(command_data->node_selection);
						DESTROY(LIST(FE_node))(&node_list);
					}
				}
			}
			DESTROY(Option_table)(&option_table);
			DEALLOCATE(region_path);
			if (conditional_field)
			{
				DEACCESS(Computed_field)(&conditional_field);
			}
			if (grid_field)
			{
				DEACCESS(FE_field)(&grid_field);
			}
			DESTROY(Multi_range)(&multi_range);
			if (element_point_ranges)
			{
				DESTROY(Element_point_ranges)(&element_point_ranges);
			}
		}
		else
		{
			set_command_prompt("gfx unselect",command_data);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_unselect.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_unselect */

int gfx_set_FE_nodal_value(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Sets nodal field values from a command.
???DB.  Should it be here ?
==============================================================================*/
{
	char *current_token;
	enum FE_nodal_value_type fe_nodal_d_ds1,fe_nodal_d_ds2,fe_nodal_d_ds3,
		fe_nodal_d2_ds1ds2,fe_nodal_d2_ds1ds3,fe_nodal_d2_ds2ds3,
		fe_nodal_d3_ds1ds2ds3,fe_nodal_value,value_type;
	FE_value value;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"value",NULL,NULL,set_enum},
		{"d/ds1",NULL,NULL,set_enum},
		{"d/ds2",NULL,NULL,set_enum},
		{"d/ds3",NULL,NULL,set_enum},
		{"d2/ds1ds2",NULL,NULL,set_enum},
		{"d2/ds1ds3",NULL,NULL,set_enum},
		{"d2/ds2ds3",NULL,NULL,set_enum},
		{"d3/ds1ds2ds3",NULL,NULL,set_enum},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct FE_field_component component;
	struct FE_node *node;
	struct FE_region *fe_region;
	struct LIST(FE_field) *fe_field_list;

	ENTER(gfx_set_FE_nodal_value);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		fe_region = Cmiss_region_get_FE_region(command_data->root_region);
		node = (struct FE_node *)NULL;
		if (return_code = set_FE_node_FE_region(state,(void *)&node,
			(void *)fe_region))
		{
			component.field = (struct FE_field *)NULL;
			component.number = 0;
			if (return_code = set_FE_field_component_FE_region(state,
				(void *)&component, (void *)fe_region))
			{
				value_type=FE_NODAL_UNKNOWN;
				option_table[0].to_be_modified= &value_type;
				fe_nodal_value=FE_NODAL_VALUE;
				option_table[0].user_data= &fe_nodal_value;
				option_table[1].to_be_modified= &value_type;
				fe_nodal_d_ds1=FE_NODAL_D_DS1;
				option_table[1].user_data= &fe_nodal_d_ds1;
				option_table[2].to_be_modified= &value_type;
				fe_nodal_d_ds2=FE_NODAL_D_DS2;
				option_table[2].user_data= &fe_nodal_d_ds2;
				option_table[3].to_be_modified= &value_type;
				fe_nodal_d_ds3=FE_NODAL_D_DS3;
				option_table[3].user_data= &fe_nodal_d_ds3;
				option_table[4].to_be_modified= &value_type;
				fe_nodal_d2_ds1ds2=FE_NODAL_D2_DS1DS2;
				option_table[4].user_data= &fe_nodal_d2_ds1ds2;
				option_table[5].to_be_modified= &value_type;
				fe_nodal_d2_ds1ds3=FE_NODAL_D2_DS1DS3;
				option_table[5].user_data= &fe_nodal_d2_ds1ds3;
				option_table[6].to_be_modified= &value_type;
				fe_nodal_d2_ds2ds3=FE_NODAL_D2_DS2DS3;
				option_table[6].user_data= &fe_nodal_d2_ds2ds3;
				option_table[7].to_be_modified= &value_type;
				fe_nodal_d3_ds1ds2ds3=FE_NODAL_D3_DS1DS2DS3;
				option_table[7].user_data= &fe_nodal_d3_ds1ds2ds3;
				if (return_code=process_option(state,option_table))
				{
					if (current_token=state->current_token)
					{
						if (1 == sscanf(current_token, FE_VALUE_INPUT_STRING, &value))
						{
							if (!(set_FE_nodal_FE_value_value(node,
								component.field, component.number, 
								/*version*/0, value_type, /*time*/0,value)))
							{
								return_code = 0;
							}
							if (!return_code)
							{
								display_message(ERROR_MESSAGE,
									"gfx_set_FE_nodal_value.  Failed");
								return_code = 0;
							}
							DESTROY(LIST(FE_field))(&fe_field_list);
						}
						else
						{
							display_message(ERROR_MESSAGE,"Invalid nodal value %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"Missing value for node");
						display_parse_state_location(state);
						return_code=1;
					}
				}
				else
				{
					if ((current_token=state->current_token)&&
						!(strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
					{
						display_message(INFORMATION_MESSAGE," #\n");
						return_code=1;
					}
				}
			}
		}
		if (node)
		{
			DEACCESS(FE_node)(&node);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_FE_nodal_value.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_FE_nodal_value */

static int gfx_set_scene_order(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Sets the ordering of graphics objects on scene(s) from the command line.
==============================================================================*/
{
	char *name;
	int return_code,position;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	struct Scene_object *scene_object;
	static struct Modifier_entry option_table[]=
	{
		{"object",NULL,(void *)1,set_name},
		{"position",NULL,NULL,set_int},
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,set_name}
	};

	ENTER(gfx_set_scene_order);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			name=(char *)NULL;
			position=0;
			scene=ACCESS(Scene)(command_data->default_scene);
			(option_table[0]).to_be_modified= &name;
			(option_table[1]).to_be_modified= &position;
			(option_table[2]).to_be_modified= &scene;
			(option_table[2]).user_data=command_data->scene_manager;
			(option_table[3]).to_be_modified= &name;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (name)
				{
					if (scene_object=Scene_get_Scene_object_by_name(scene,name))
					{
						return_code=Scene_set_scene_object_position(scene,scene_object,
							position);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"No graphics object named '%s' in scene",name);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing graphics object name");
					return_code=0;
				}
			} /* parse error,help */
			if (name)
			{
				DEALLOCATE(name);
			}
			if (scene)
			{
				DEACCESS(Scene)(&scene);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_scene_order.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_set_scene_order.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_scene_order */

#if defined (MOTIF)
static int gfx_set_time(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Sets the time from the command line.
==============================================================================*/
{
	char *timekeeper_name;
	float time;
	int return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"timekeeper",NULL,NULL,set_name},
		{NULL,NULL,NULL,set_float}
	};

	ENTER(gfx_set_time);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			ALLOCATE(timekeeper_name,char,10);
			/* there is only a default timekeeper at the moment but I am making the
				commands with a timekeeper manager in mind */
			strcpy(timekeeper_name,"default");
			if (command_data->default_time_keeper)
			{
				time=Time_keeper_get_time(command_data->default_time_keeper);
			}
			else
			{
				/*This option is used so that help comes out*/
				time = 0;
			}
			(option_table[0]).to_be_modified= &timekeeper_name;
			(option_table[1]).to_be_modified= &time;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				/* the old routine only use to call this if the time wasn't the
					same as the default time, but the timekeeper might not be the
					default */
				Time_keeper_request_new_time(command_data->default_time_keeper,time);
			} /* parse error, help */
			DEALLOCATE(timekeeper_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_time.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_time.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_time */
#endif /* defined (MOTIF) */

static int set_transformation_matrix(struct Parse_state *state,
	void *transformation_matrix_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Sets a transformation matrix from the command line.
==============================================================================*/
{
	char *current_token;
	gtMatrix *transformation_matrix;
	int i,j,return_code;

	ENTER(set_transformation_matrix);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if ((current_token=state->current_token)&&(
			!strcmp(PARSER_HELP_STRING,current_token)||
			!strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
		{
			display_message(INFORMATION_MESSAGE,"# # # # # # # # # # # # # # # #");
			if (transformation_matrix=(gtMatrix *)transformation_matrix_void)
			{
				display_message(INFORMATION_MESSAGE,
					" [%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g]",
					(*transformation_matrix)[0][0],(*transformation_matrix)[0][1],
					(*transformation_matrix)[0][2],(*transformation_matrix)[0][3],
					(*transformation_matrix)[1][0],(*transformation_matrix)[1][1],
					(*transformation_matrix)[1][2],(*transformation_matrix)[1][3],
					(*transformation_matrix)[2][0],(*transformation_matrix)[2][1],
					(*transformation_matrix)[2][2],(*transformation_matrix)[2][3],
					(*transformation_matrix)[3][0],(*transformation_matrix)[3][1],
					(*transformation_matrix)[3][2],(*transformation_matrix)[3][3]);
			}
			return_code=1;
		}
		else
		{
			if (transformation_matrix=(gtMatrix *)transformation_matrix_void)
			{
				return_code=1;
				i=0;
				while ((i<4)&&return_code&&current_token)
				{
					j=0;
					while ((j<4)&&return_code&&current_token)
					{
						if (1==sscanf(current_token,"%f",&((*transformation_matrix)[i][j])))
						{
							shift_Parse_state(state,1);
							current_token=state->current_token;
						}
						else
						{
							return_code=0;
						}
						j++;
					}
					i++;
				}
				if (!return_code||(i<4)||(j<4))
				{
					if (current_token)
					{
						display_message(ERROR_MESSAGE,
							"Error reading transformation matrix: %s",current_token);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Error reading transformation matrix");
					}
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_transformation_matrix.  Missing transformation_matrix");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_transformation_matrix.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_transformation_matrix */

static int gfx_set_transformation(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Sets the transformation for a graphics object from the command line.
==============================================================================*/
{
	char *scene_object_name;
	gtMatrix transformation_matrix;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	struct Scene_object *scene_object;
	static struct Modifier_entry option_table[]=
	{
		{"name",NULL,(void *)1,set_name},
		{"scene",NULL,NULL,set_Scene},
		{"field",NULL,NULL,set_Computed_field_conditional},
		{NULL,NULL,NULL,set_transformation_matrix}
	};
	struct Computed_field *computed_field;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(gfx_set_transformation);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			 /* initialise defaults */
			computed_field=(struct Computed_field *)NULL;
			set_field_data.conditional_function=
				 (MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_field_data.conditional_function_user_data=(void *)NULL;
			set_field_data.computed_field_manager=
				 Computed_field_package_get_computed_field_manager(
						command_data->computed_field_package);
			scene_object_name=(char *)NULL;
			scene=ACCESS(Scene)(command_data->default_scene);
			transformation_matrix[0][0]=1;
			transformation_matrix[0][1]=0;
			transformation_matrix[0][2]=0;
			transformation_matrix[0][3]=0;
			transformation_matrix[1][0]=0;
			transformation_matrix[1][1]=1;
			transformation_matrix[1][2]=0;
			transformation_matrix[1][3]=0;
			transformation_matrix[2][0]=0;
			transformation_matrix[2][1]=0;
			transformation_matrix[2][2]=1;
			transformation_matrix[2][3]=0;
			transformation_matrix[3][0]=0;
			transformation_matrix[3][1]=0;
			transformation_matrix[3][2]=0;
			transformation_matrix[3][3]=1;
			/* parse the command line */
			(option_table[0]).to_be_modified= &scene_object_name;
			(option_table[1]).to_be_modified= &scene;
			(option_table[1]).user_data=command_data->scene_manager;
			(option_table[2]).to_be_modified= &computed_field;
			(option_table[2]).user_data= &set_field_data;
			(option_table[3]).to_be_modified= &transformation_matrix;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (scene_object_name)
				{
					if (scene_object=Scene_get_Scene_object_by_name(scene,
						scene_object_name))
					{
						 if (computed_field)
						 {
								if (!command_data->scene_object_transformation_data)
								{
									 ALLOCATE(command_data->scene_object_transformation_data,
											struct Scene_object_transformation_data,1);
								}
								if (command_data->scene_object_transformation_data)
								{
									 command_data->scene_object_transformation_data->scene_object = 
											scene_object;
									 command_data->scene_object_transformation_data->computed_field = 
											computed_field;
									 Time_keeper_add_callback(command_data->default_time_keeper,
											Scene_object_set_transformation_with_time_callback,
											(void *)command_data->scene_object_transformation_data,
											(enum Time_keeper_event) (TIME_KEEPER_NEW_TIME | 
												 TIME_KEEPER_NEW_MINIMUM | TIME_KEEPER_NEW_MAXIMUM ));
								}
						 }
						 else
						 {
								if (command_data->scene_object_transformation_data)
								{
									 Time_keeper_remove_callback(command_data->default_time_keeper,
											Scene_object_set_transformation_with_time_callback, 
											(void *)command_data->scene_object_transformation_data);
									 DEALLOCATE(command_data->scene_object_transformation_data);
									 command_data->scene_object_transformation_data = 
											(struct Scene_object_transformation_data*)NULL;
								}
								Scene_object_set_transformation(scene_object,
									 &transformation_matrix);
						 }
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"No object named '%s' in scene",scene_object_name);return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing graphics object name");
					return_code=0;
				}
			} /* parse error, help */
			DEACCESS(Scene)(&scene);
			if (scene_object_name)
			{
				DEALLOCATE(scene_object_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_transformation.  Missing command_data_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_transformation.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_transformation */

static int gfx_set_visibility(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Toggles the visibility of graphics objects on scenes from the command line.
==============================================================================*/
{
	char *name,off_flag,on_flag;
	enum GT_visibility_type current_visibility,new_visibility;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Scene *scene;
	struct Scene_object *scene_object;
	static struct Modifier_entry
		off_on_option_table[]=
		{
			{"off",NULL,NULL,set_char_flag},
			{"on",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"name",NULL,(void *)1,set_name},
			{NULL,NULL,NULL,NULL},
			{"scene",NULL,NULL,set_Scene},
			{NULL,NULL,NULL,set_name}
		};

	ENTER(gfx_set_visibility);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			name=(char *)NULL;
			scene=ACCESS(Scene)(command_data->default_scene);
			off_flag=0;
			on_flag=0;
			(option_table[0]).to_be_modified = &name;
			(off_on_option_table[0]).to_be_modified= &off_flag;
			(off_on_option_table[1]).to_be_modified= &on_flag;
			(option_table[1]).user_data= &off_on_option_table;
			(option_table[2]).to_be_modified= &scene;
			(option_table[2]).user_data=command_data->scene_manager;
			(option_table[3]).to_be_modified= &name;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (off_flag&&on_flag)
				{
					display_message(ERROR_MESSAGE,"Only one of off/on");
					return_code=0;
				}
				if (return_code)
				{
					if (name)
					{
						if (scene_object=Scene_get_Scene_object_by_name(scene,name))
						{
							current_visibility=Scene_object_get_visibility(scene_object);
							if (on_flag)
							{
								new_visibility=g_VISIBLE;
							}
							else
							{
								if (off_flag)
								{
									new_visibility=g_INVISIBLE;
								}
								else
								{
									if (g_VISIBLE == current_visibility)
									{
										new_visibility=g_INVISIBLE;
									}
									else
									{
										new_visibility=g_VISIBLE;
									}
								}
							}
							if (new_visibility!=current_visibility)
							{
								Scene_object_set_visibility(scene_object,new_visibility);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"No graphics object named '%s' in scene",name);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Missing graphics object name");
						return_code=0;
					}
				}
			} /* parse error,help */
			DEACCESS(Scene)(&scene);
			if (name)
			{
				DEALLOCATE(name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_visibility.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_set_visibility.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_visibility */

static int execute_command_gfx_set(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 27 November 2001

DESCRIPTION :
Executes a GFX SET command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_set);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			option_table=CREATE(Option_table)();
#if defined (MOTIF)
			Option_table_add_entry(option_table, "line_width", &global_line_width,
				NULL, set_float_positive);
#endif /* defined (MOTIF) */
			Option_table_add_entry(option_table, "node_value", NULL,
				command_data_void, gfx_set_FE_nodal_value);
			Option_table_add_entry(option_table, "order", NULL,
				command_data_void, gfx_set_scene_order);
			Option_table_add_entry(option_table, "point_size", &global_point_size,
				NULL, set_float_positive);
			Option_table_add_entry(option_table, "transformation", NULL,
				command_data_void, gfx_set_transformation);
#if defined (MOTIF)
			Option_table_add_entry(option_table, "time", NULL,
				command_data_void, gfx_set_time);
#endif /* defined (MOTIF) */
			Option_table_add_entry(option_table, "visibility", NULL,
				command_data_void, gfx_set_visibility);
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx set", command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_set.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_set */

static int execute_command_gfx_smooth(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 April 2003

DESCRIPTION :
Executes a GFX SMOOTH command.
==============================================================================*/
{
	char *region_path;
	FE_value time;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *region;
	struct FE_field *fe_field;
	struct FE_region *fe_region;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_smooth);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		Cmiss_region_get_root_region_path(&region_path);
		fe_field = (struct FE_field *)NULL;
		if (command_data->default_time_keeper)
		{
			time = Time_keeper_get_time(command_data->default_time_keeper);
		}
		else
		{
			time = 0;
		}

		option_table = CREATE(Option_table)();
		/* egroup */
		Option_table_add_entry(option_table, "egroup", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* field */
		Option_table_add_set_FE_field_from_FE_region(
			option_table, "field" ,&fe_field,
			Cmiss_region_get_FE_region(command_data->root_region));
		/* time */
		Option_table_add_entry(option_table, "time", &time, NULL, set_FE_value);
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code)
		{
			if (Cmiss_region_get_region_from_path(command_data->root_region,
				region_path, &region) &&
				(fe_region = Cmiss_region_get_FE_region(region)))
			{
				if (fe_field)
				{
					return_code = FE_region_smooth_FE_field(fe_region, fe_field, time);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx smooth:  Must specify field to smooth");
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_gfx_smooth.  Invalid region");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		DEALLOCATE(region_path);
		if (fe_field)
		{
			DEACCESS(FE_field)(&fe_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_smooth.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_smooth */

int gfx_timekeeper(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
==============================================================================*/
{
	char every, loop, maximum_flag, minimum_flag, once, play, set_time_flag,
		skip, speed_flag, stop, swing;
	double maximum, minimum, set_time, speed;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"every_frame",NULL,NULL,set_char_flag},
		{"loop",NULL,NULL,set_char_flag},
		{"maximum",NULL,NULL,set_double_and_char_flag},
		{"minimum",NULL,NULL,set_double_and_char_flag},
		{"once",NULL,NULL,set_char_flag},
		{"play",NULL,NULL,set_char_flag},
		{"set_time",NULL,NULL,set_double_and_char_flag},
		{"skip_frames",NULL,NULL,set_char_flag},
		{"speed",NULL,NULL,set_double_and_char_flag},
		{"stop",NULL,NULL,set_char_flag},
		{"swing",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,NULL}
	};
	struct Cmiss_command_data *command_data;
	struct Time_keeper *time_keeper;

	ENTER(gfx_timekeeper);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data = (struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
				{
					if (!strcmp(state->current_token, "default"))
					{
						/* Continue */
						return_code = shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Only a default timekeeper at the moment");
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						"\n      TIMEKEEPER_NAME");
					/* By not shifting the parse state the rest of the help should come out */
					return_code = 1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_timekeeper.  Missing timekeeper name");
				return_code=0;
			}
			if (return_code)
			{
				/* initialise defaults */
				if (time_keeper = command_data->default_time_keeper)
				{
					maximum = Time_keeper_get_maximum(time_keeper);
					minimum = Time_keeper_get_minimum(time_keeper);
					set_time = Time_keeper_get_time(time_keeper);
					speed = Time_keeper_get_speed(time_keeper);
				}
				else
				{
					maximum = 0.0;
					minimum = 0.0;
					set_time = 0.0;
					speed = 30;
				}
				every = 0;
				loop = 0;
				maximum_flag = 0;
				minimum_flag = 0;
				once = 0;
				play = 0;
				set_time_flag = 0;
				skip = 0;
				speed_flag = 0;
				stop = 0;
				swing = 0;

				(option_table[0]).to_be_modified = &every;
				(option_table[1]).to_be_modified = &loop;
				(option_table[2]).to_be_modified = &maximum;
				(option_table[2]).user_data = &maximum_flag;
				(option_table[3]).to_be_modified = &minimum;				
				(option_table[3]).user_data = &minimum_flag;
				(option_table[4]).to_be_modified = &once;
				(option_table[5]).to_be_modified = &play;
				(option_table[6]).to_be_modified = &set_time;
				(option_table[6]).user_data = &set_time_flag;
				(option_table[7]).to_be_modified = &skip;
				(option_table[8]).to_be_modified = &speed;
				(option_table[8]).user_data = &speed_flag;
				(option_table[9]).to_be_modified = &stop;
				(option_table[10]).to_be_modified = &swing;
				return_code=process_multiple_options(state,option_table);

				if(return_code)
				{
					if((loop + once + swing) > 1)
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Specify only one of loop, swing or once");
						return_code = 0;
					}
					if(every && skip)
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Specify only one of every_frame or skip_frames");
						return_code = 0;
					}
					if(play && stop)
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Specify only one of play or stop");
						return_code = 0;
					}
				}
				if (return_code)
				{
					if ( time_keeper )
					{
						if ( set_time_flag )
						{
							Time_keeper_request_new_time(time_keeper, set_time);
						}
						if ( speed_flag )
						{
							Time_keeper_set_speed(time_keeper, speed);
						}
						if ( maximum_flag )
						{
							Time_keeper_set_maximum(time_keeper, maximum);
						}
						if ( minimum_flag )
						{
							Time_keeper_set_minimum(time_keeper, minimum);
						}
						if ( loop )
						{
							Time_keeper_set_play_loop(time_keeper);
						}
						if ( swing )
						{
							Time_keeper_set_play_swing(time_keeper);
						}
						if ( once )
						{
							Time_keeper_set_play_once(time_keeper);
						}
						if ( every )
						{
							Time_keeper_set_play_every_frame(time_keeper);
						}
						if ( skip )
						{
							Time_keeper_set_play_skip_frames(time_keeper);
						}
						if ( play )
						{
							Time_keeper_play(time_keeper, TIME_KEEPER_PLAY_FORWARD);
						}
						if ( stop )
						{
							Time_keeper_stop(time_keeper);
						}
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_timekeeper.  Missing command data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_timekeeper.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* gfx_timekeeper */


static int gfx_transform_tool(struct Parse_state *state,
	void *dummy_user_data,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
Executes a GFX TRANSFORM_TOOL command.
==============================================================================*/
{
	int free_spin_flag, return_code;
	struct Option_table *option_table;
	struct Cmiss_command_data *command_data;
	struct Interactive_tool *transform_tool;

	ENTER(execute_command_gfx_transform_tool);
	USE_PARAMETER(dummy_user_data);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void)
		&& (transform_tool=command_data->transform_tool))
	{
		/* initialize defaults */
		free_spin_flag = Interactive_tool_transform_get_free_spin(transform_tool);
		option_table=CREATE(Option_table)();
		/* free_spin/no_free_spin */
		Option_table_add_switch(option_table,"free_spin","no_free_spin",&free_spin_flag);
		if (return_code=Option_table_multi_parse(option_table,state))
		{
			Interactive_tool_transform_set_free_spin(transform_tool, free_spin_flag);
#if defined (WX_USER_INTERFACE)
			FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
				Graphics_window_update_Interactive_tool,(void *)transform_tool,
				command_data->graphics_window_manager);
#endif /*(WX_USER_INTERFACE)*/
			Cmiss_scene_viewer_package_update_Interactive_tool(
				command_data->scene_viewer_package,
				transform_tool);
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_transform_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_transform_tool */

#if defined (MOTIF) || defined (WX_USER_INTERFACE)
static int execute_command_gfx_update(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 October 1998

DESCRIPTION :
Executes a GFX UPDATE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Graphics_window *window;
	static struct Modifier_entry option_table[]=
	{
		{"window",NULL,NULL,set_Graphics_window},/*???DB. "on" ? */
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_update);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			/* initialize defaults */
			window=(struct Graphics_window *)NULL;
			(option_table[0]).to_be_modified= &window;
			(option_table[0]).user_data=command_data->graphics_window_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (window)
				{
					return_code=Graphics_window_update_now(window);
				}
				else
				{
					return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
						Graphics_window_update_now_iterator,(void *)NULL,
						command_data->graphics_window_manager);
				}
			} /* parse error,help */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_update.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_update.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_update */
#endif /* defined (MOTIF) || defined (WX_USER_INTERFACE) */

static int gfx_write_All(struct Parse_state *state,
	 void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 August  2007

DESCRIPTION :
If an zip file is not specified a file selection box is presented to the
user, otherwise files are written.
Can also write individual groups with the <group> option.
==============================================================================*/
{
	 FILE *com_file;
	 char *com_file_name, *data_file_name, *elem_file_name, *file_name, *node_file_name, *region_path,
			**valid_strings, *write_criterion_string;
	 enum FE_write_criterion write_criterion;
	 int number_of_valid_strings, data_return_code, elem_return_code, 
			node_return_code, return_code, data_fd,
			elem_fd, node_fd, com_return_code;
	 struct Cmiss_command_data *command_data;
	 struct FE_field_order_info *field_order_info;
	 struct Option_table *option_table;
	 struct MANAGER(Graphical_material) *graphical_material_manager;
	 struct MANAGER(Computed_field) *computed_field_manager;
	 struct LIST(Computed_field) *list_of_fields;
	 struct List_Computed_field_commands_data list_commands_data;
	 static char	*command_prefix;
#if defined (WX_USER_INTERFACE)
#if defined (__WIN32__)
	 char temp_data[L_tmpnam];
	 char temp_elem[L_tmpnam];
	 char temp_node[L_tmpnam];
#else /* (__WIN32__) */
	 char temp_data[] = "dataXXXXXX";
	 char temp_elem[] = "elemXXXXXX";
	 char temp_node[] = "nodeXXXXXX";
#endif /* (__WIN32__) */
#else /* (WX_USER_INTERFACE) */
	 char *temp_data;
	 char *temp_elem;
	 char *temp_node;
#endif /* (WX_USER_INTERFACE) */

	 ENTER(gfx_write_all);
	 USE_PARAMETER(dummy_to_be_modified);
	 if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	 {
			data_fd = 1;
			elem_fd = 1;
			node_fd = 1;
			data_return_code = 1;
			elem_return_code = 1;
			node_return_code = 1;
			com_return_code = 1;
			return_code = 1; 
			region_path = (char *)NULL;
			field_order_info = (struct FE_field_order_info *)NULL;
			file_name = (char *)NULL;
			com_file_name = (char *)NULL;		
			data_file_name = (char *)NULL;
			elem_file_name = (char *)NULL;
			node_file_name = (char *)NULL;

			write_criterion = FE_WRITE_COMPLETE_GROUP;
			
			option_table = CREATE(Option_table)();
			/* complete_group|with_all_listed_fields|with_any_listed_fields */ 
			write_criterion_string =
				 ENUMERATOR_STRING(FE_write_criterion)(write_criterion);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(FE_write_criterion)(
				 &number_of_valid_strings,
				 (ENUMERATOR_CONDITIONAL_FUNCTION(FE_write_criterion) *)NULL,
				 (void *)NULL);
			Option_table_add_enumerator(option_table, number_of_valid_strings,
				 valid_strings, &write_criterion_string);
			DEALLOCATE(valid_strings);
			/* fields */
			Option_table_add_entry(option_table, "fields", &field_order_info,
				 Cmiss_region_get_FE_region(command_data->root_region),
				 set_FE_fields_FE_region);
			/* group */
			Option_table_add_entry(option_table, "group", &region_path,
				 command_data->root_region, set_Cmiss_region_path);
			/* default option: file name */
			Option_table_add_entry(option_table, (char *)NULL, &file_name,
				 NULL, set_name);			
			if (return_code = Option_table_multi_parse(option_table, state))
			{
				 STRING_TO_ENUMERATOR(FE_write_criterion)(write_criterion_string,
						&write_criterion);
				 if (file_name)
				 {
						int length = strlen(file_name);
						if (ALLOCATE(com_file_name, char, length+6))
						{
							 strcpy(com_file_name, file_name);
							 strcat(com_file_name, ".com");
							 com_file_name[length+5]='\0';
						}
						if (ALLOCATE(data_file_name, char, length+9))
						{
							 strcpy(data_file_name, file_name);
							 strcat(data_file_name, ".exdata");
							 data_file_name[length+8]='\0';
						}
						if (ALLOCATE(elem_file_name, char, length+9))
						{
							 strcpy(elem_file_name, file_name);
							 strcat(elem_file_name, ".exelem");
							 elem_file_name[length+8]='\0';
						}
						if (ALLOCATE(node_file_name, char, length+9))
						{
							 strcpy(node_file_name, file_name);
							 strcat(node_file_name, ".exnode");
							 node_file_name[length+8]='\0';
						}
				 }
				 if ((!field_order_info ||
						(0 == get_FE_field_order_info_number_of_fields(field_order_info))) &&
						(FE_WRITE_COMPLETE_GROUP != write_criterion))
				 {
						display_message(WARNING_MESSAGE,
							 "gfx_write_All.  Must specify fields to use %s",
							 write_criterion_string);
						return_code = 0;
						data_return_code = 0;
						elem_return_code = 0;
						node_return_code = 0;		
				 }
				 if (!file_name)
				 {
/* 						com_file_name = "temp.com"; */
						if (!(com_file_name = confirmation_get_write_filename(".com",
										 command_data->user_interface
#if defined(WX_USER_INTERFACE)
										 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																																	 )))
						{
							 com_return_code = 0;
						}
						if (!(data_file_name = confirmation_get_write_filename(".exdata",
										 command_data->user_interface
#if defined(WX_USER_INTERFACE)
										 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																																	 )))
						{
							 data_return_code = 0;
						}
						if (!(elem_file_name = confirmation_get_write_filename(".exelem",
										 command_data->user_interface
#if defined(WX_USER_INTERFACE)
										 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																															)))
						{
							 elem_return_code = 0;
						}
	
						if (!(node_file_name = confirmation_get_write_filename(".exnode",
										 command_data->user_interface
#if defined(WX_USER_INTERFACE)
										 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																															)))
						{
							 node_return_code = 0;
						}
#if defined(WX_USER_INTERFACE)		
						file_name = confirmation_get_write_filename(".zip",
							 command_data->user_interface
							 , command_data->execute_command);
#endif /*defined (WX_USER_INTERFACE) */
				 }
#if defined (WX_USER_INTERFACE) && defined (__WIN32__)
				 if (com_file_name)
				 {
						com_file_name = CMISS_set_directory_and_filename_WIN32(com_file_name,
							 command_data);
				 }
				 if (data_file_name)
				 {
						data_file_name = CMISS_set_directory_and_filename_WIN32(data_file_name,
							 command_data);
				 }
				 if (elem_file_name)
				 {
						elem_file_name = CMISS_set_directory_and_filename_WIN32(elem_file_name,
							 command_data);
				 }
				 if (node_file_name)
				 {
						node_file_name = CMISS_set_directory_and_filename_WIN32(node_file_name,
							 command_data);
				 }
#endif /* defined (WX_USER_INTERFACE) && defined (__WIN32__) */
				 if (com_return_code)
				 {
						com_return_code = check_suffix(&com_file_name,".com");
				 }
				 if (data_return_code)
				 {
						data_return_code = check_suffix(&data_file_name,".exdata");
				 }
				 if (elem_return_code)
				 {
						elem_return_code = check_suffix(&elem_file_name,".exelem");
				 }
				 if (node_return_code)
				 {		
						node_return_code = check_suffix(&node_file_name,".exnode");
				 }
#if defined (WX_USER_INTERFACE) 
#if defined (__WIN32__)
				 /* 	Non MS-windows platform does not have mkstemp implemented,
						therefore tmpnam is used.*/
				 if (data_return_code)
				 {
						tmpnam(temp_data);
						if (temp_data == NULL)
						{
							 data_fd = -1;
						}
				 }
				 if (elem_return_code)
				 {
							 tmpnam(temp_elem);
							 if (temp_elem == NULL)
							 {
									elem_fd = -1;
							 }
				 }
				 if (node_return_code)
				 {		
						tmpnam(temp_node);
						if (temp_node == NULL)
						{
							 node_fd = -1;
						}
				 }
#else
				 /* Non MS-windows platform has mkstemp implemented into it*/
				 if (data_return_code)
				 {
							 data_fd = mkstemp((char *)temp_data);
				 }
				 if (elem_return_code)
				 {
							 elem_fd = mkstemp((char *)temp_elem);
				 }
				 if (node_return_code)
				 {		
							 node_fd = mkstemp((char *)temp_node);
				 }
#endif /* (__WIN32__) */
#else /* (WX_USER_INTERFACE) */
				 /* Non wx_user_interface won't be able to stored the file in
						a zip file at the moment */
				 if (data_return_code)
				 {
							 temp_data = data_file_name;
				 }
				 if (elem_return_code)
				 {
							 temp_elem = elem_file_name;
				 }
				 if (node_return_code)
				 {		
							 temp_node = node_file_name;
				 }
#endif /* (WX_USER_INTERFACE) */
				 if (data_fd == -1)
				 {
						display_message(ERROR_MESSAGE,
							 "gfx_write_All.  Could not open temprary data file");
				 }
				 else
				 {
						if (!(data_return_code = write_exregion_file_of_name(temp_data, 
										 command_data->data_root_region, region_path,
										 /*write_elements*/0, /*write_nodes*/1,
										 write_criterion, field_order_info)))
						{
							 display_message(ERROR_MESSAGE,
									"gfx_write_All.  Could not create temprary data file");
						}
				 }

				 if (elem_fd == -1)
				 {
						display_message(ERROR_MESSAGE,
							 "gfx_write_All.  Could not open temprary elem file");
				 }
				 else
				 {
						if (!(elem_return_code = write_exregion_file_of_name(temp_elem, 
										 command_data->root_region, region_path,
										 /*write_elements*/1, /*write_nodes*/0,
										 write_criterion, field_order_info)))
						{
							 display_message(ERROR_MESSAGE,
									"gfx_write_All.  Could not create temprary elem file");
						}
				 }

				 if (node_fd == -1)
				 {
						display_message(ERROR_MESSAGE,
							 "gfx_write_All.  Could not open temprary node file");
				 }
				 else
				 {			
						if (!(node_return_code = write_exregion_file_of_name(temp_node, 
										 command_data->root_region, region_path,
										 /*write_elements*/0, /*write_nodes*/1,
										 write_criterion, field_order_info)))
						{
							 display_message(ERROR_MESSAGE,
									"gfx_write_All.  Could not create temprary node file");
						}
				 }
				 if (com_return_code)
				 {		 
						if (com_file = fopen("temp_file_com.com", "w"))
						{
							 if (data_file_name)
							 {		
									fprintf(com_file, "gfx read data %s\n",data_file_name);
							 }
							 if (node_file_name)
							 {		
									fprintf(com_file, "gfx read nodes %s\n",node_file_name);
							 }
							 if (elem_file_name)
							 {		
									fprintf(com_file, "gfx read elements %s\n",elem_file_name);
							 }
							 fclose(com_file);
							 if (command_data->computed_field_package && (computed_field_manager=
										 Computed_field_package_get_computed_field_manager(
												command_data->computed_field_package)))
							 {
									if (list_of_fields = CREATE(LIST(Computed_field))())
									{
										 command_prefix="gfx define field ";
										 list_commands_data.command_prefix = command_prefix;
										 list_commands_data.listed_fields = 0;
										 list_commands_data.computed_field_list = list_of_fields;
										 list_commands_data.computed_field_manager =
												computed_field_manager;
										 while (FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
															 write_Computed_field_commands_if_managed_source_fields_in_list_to_comfile,
															 (void *)&list_commands_data, computed_field_manager) &&
												(0 != list_commands_data.listed_fields))
										 {
												list_commands_data.listed_fields = 0;
										 }			 
										 DESTROY(LIST(Computed_field))(&list_of_fields);
									} 
									else
									{
										 return_code=0;
									}
									if (!return_code)
									{
										 display_message(ERROR_MESSAGE,
												"gfx_write_All.  Could not list field commands");
									}
							 }

							 if (command_data->spectrum_manager)
							 {
									FOR_EACH_OBJECT_IN_MANAGER(Spectrum)(
										 for_each_spectrum_list_or_write_commands, (void *)"true", command_data->spectrum_manager);			
							 }
							 if (graphical_material_manager =
									Material_package_get_material_manager(command_data->material_package))
							 {
									command_prefix="gfx create material ";
									return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphical_material)(
										 write_Graphical_material_commands_to_comfile,(void *)command_prefix, 
										 graphical_material_manager);
							 }
							 return_code=FOR_EACH_OBJECT_IN_MANAGER(Scene)(
									for_each_graphics_object_in_scene_get_command_list, (void*) "true",
									command_data->scene_manager);
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
							 return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
									write_Graphics_window_commands_to_comfile,(void *)NULL,
									command_data->graphics_window_manager);
#endif /*defined (USE_CMGUI_GRAPHICS_WINDOW)*/
							 rename("temp_file_com.com", com_file_name);
						}
				 }
#if defined (WX_USER_INTERFACE)
				 if (data_file_name && elem_file_name && node_file_name)
				 {
						filedir_compressing_process_wx_compress(com_file_name, data_file_name,
							 elem_file_name, node_file_name, data_return_code, elem_return_code, 
							 node_return_code, file_name, temp_data, temp_elem, temp_node);
				 }
#if !defined (__WIN32__)
				 if (unlink(temp_data) == -1) 
				 {
						display_message(ERROR_MESSAGE,
							 "gfx_write_All.  Could not unlink temporary data file");
				 }
				 if (unlink(temp_elem) == -1) 
				 {
						display_message(ERROR_MESSAGE,
							 "gfx_write_All.  Could not unlink temporary elem file");
				 }
				 if (unlink(temp_node) == -1) 
				 {
						display_message(ERROR_MESSAGE,
							 "gfx_write_All.  Could not unlink temporary node file");
				 }
				 if (unlink(com_file_name) == -1) 
				 {
						display_message(ERROR_MESSAGE,
							 "compressing_process_wx_compress.  Could not unlink temporary com file");
				 }
#endif /*!definde (__WIN32__)*/
#endif /* defined (WX_USER_INTERFACE) */
				 if (com_file_name)
				 {
						DEALLOCATE(com_file_name);
				 }
				 if (node_file_name)
				 {
						DEALLOCATE(node_file_name);
				 }
				 if (elem_file_name)
				 {
						DEALLOCATE(elem_file_name);
				 }
				 if (data_file_name)
				 {
						DEALLOCATE(data_file_name);
				 }		
			}
			DESTROY(Option_table)(&option_table);
			if (region_path)
			{
				 DEALLOCATE(region_path);
			}
			if (field_order_info)
			{
				 DESTROY(FE_field_order_info)(&field_order_info);
			}
			if (file_name)
			{
				 DEALLOCATE(file_name);
			}
	 }
	 else
	 {
			display_message(ERROR_MESSAGE, "gfx_write_All.  Invalid argument(s)");
			return_code = 0;
	 }
	 LEAVE;
	 
	 return (return_code);
} /* gfx_write_elements */

#if defined (NEW_CODE)
static int gfx_write_Com(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Writes a com file
==============================================================================*/
{
	char write_all_curves_flag;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Modifier_entry option_table[]=
	{
		{"all",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,set_Curve}
	};
	struct Curve *curve;

	ENTER(gfx_write_Curve);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		return_code=1;
		write_all_curves_flag=0;
		curve=(struct Curve *)NULL;
		(option_table[0]).to_be_modified= &write_all_curves_flag;
		(option_table[1]).to_be_modified= &curve;
		(option_table[1]).user_data=command_data->curve_manager;
		if (return_code=process_multiple_options(state,option_table))
		{
			if (write_all_curves_flag&&!curve)
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Curve)(
					write_Curve,(void *)NULL,
					command_data->curve_manager);
			}
			else if (curve&&!write_all_curves_flag)
			{
				return_code=write_Curve(curve,(void *)NULL);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_write_Curve.  Specify either a curve name or 'all'");
				return_code=0;
			}
		}
		if (curve)
		{
			DEACCESS(Curve)(&curve);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_write_Curve.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_Curve */


#endif /* defined (NEW_CODE) */

static int gfx_write_Curve(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Writes an individual curve or all curves to filename(s) stemming from the name
of the curve, eg. "name" -> name.curve.com name.curve.exnode name.curve.exelem
==============================================================================*/
{
	char write_all_curves_flag;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Modifier_entry option_table[]=
	{
		{"all",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,set_Curve}
	};
	struct Curve *curve;

	ENTER(gfx_write_Curve);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct Cmiss_command_data *)command_data_void))
	{
		return_code=1;
		write_all_curves_flag=0;
		curve=(struct Curve *)NULL;
		(option_table[0]).to_be_modified= &write_all_curves_flag;
		(option_table[1]).to_be_modified= &curve;
		(option_table[1]).user_data=command_data->curve_manager;
		if (return_code=process_multiple_options(state,option_table))
		{
			if (write_all_curves_flag&&!curve)
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Curve)(
					write_Curve,(void *)NULL,
					command_data->curve_manager);
			}
			else if (curve&&!write_all_curves_flag)
			{
				return_code=write_Curve(curve,(void *)NULL);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_write_Curve.  Specify either a curve name or 'all'");
				return_code=0;
			}
		}
		if (curve)
		{
			DEACCESS(Curve)(&curve);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_write_Curve.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_Curve */

static int gfx_write_elements(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 March 2003

DESCRIPTION :
If an element file is not specified a file selection box is presented to the
user, otherwise the element file is written.
Can also write individual element groups with the <group> option.
==============================================================================*/
{
	char *file_ext = ".exelem";
	char *file_name, *region_path, **valid_strings, *write_criterion_string;
	enum FE_write_criterion write_criterion;
	int number_of_valid_strings, return_code;
	struct Cmiss_command_data *command_data;
	struct FE_field_order_info *field_order_info;
	struct Option_table *option_table;

	ENTER(gfx_write_elements);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		return_code = 1; 
		region_path = (char *)NULL;
		field_order_info = (struct FE_field_order_info *)NULL;
		file_name = (char *)NULL;
		write_criterion = FE_WRITE_COMPLETE_GROUP;

		option_table = CREATE(Option_table)();
		/* complete_group|with_all_listed_fields|with_any_listed_fields */ 
		write_criterion_string =
			ENUMERATOR_STRING(FE_write_criterion)(write_criterion);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(FE_write_criterion)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(FE_write_criterion) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
			valid_strings, &write_criterion_string);
		DEALLOCATE(valid_strings);
		/* fields */
		Option_table_add_entry(option_table, "fields", &field_order_info,
			Cmiss_region_get_FE_region(command_data->root_region),
			set_FE_fields_FE_region);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* default option: file name */
		Option_table_add_entry(option_table, (char *)NULL, &file_name,
			NULL, set_name);

		if (return_code = Option_table_multi_parse(option_table, state))
		{
			STRING_TO_ENUMERATOR(FE_write_criterion)(write_criterion_string,
				&write_criterion);
			if ((!field_order_info ||
				(0 == get_FE_field_order_info_number_of_fields(field_order_info))) &&
				(FE_WRITE_COMPLETE_GROUP != write_criterion))
			{
				display_message(WARNING_MESSAGE,
					"gfx_write_elements.  Must specify fields to use %s",
					write_criterion_string);
				return_code = 0;
				
			}
			if (!file_name)
			{
				if (!(file_name = confirmation_get_write_filename(file_ext,
					command_data->user_interface
#if defined(WX_USER_INTERFACE)
									 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																													)))
				{
					return_code = 0;
				}
			}
#if defined (WX_USER_INTERFACE) && (__WIN32__)
			if (file_name)
			{
				 file_name = CMISS_set_directory_and_filename_WIN32(file_name,
						command_data);
			}
#endif /* defined (WX_USER_INTERFACE) && (__WIN32__) */
			if (return_code)
			{
				/* open the file */
				if (return_code = check_suffix(&file_name,".exelem"))
				{
					return_code = write_exregion_file_of_name(file_name, 
						command_data->root_region, region_path,
						/*write_elements*/1, /*write_nodes*/0,
						write_criterion, field_order_info);
				}
			}
		}
		DESTROY(Option_table)(&option_table);
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
		if (field_order_info)
		{
			DESTROY(FE_field_order_info)(&field_order_info);
		}
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_write_elements.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_elements */

static int gfx_write_nodes(struct Parse_state *state,
	void *use_data, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 March 2003

DESCRIPTION :
If a nodes file is not specified a file selection box is presented to the user,
otherwise the nodes file is written.
Can now specify individual node groups to write with the <group> option.
If <use_data> is set, writing data, otherwise writing nodes.
==============================================================================*/
{
	static char *data_file_ext = ".exdata";
	static char *node_file_ext = ".exnode";
	char *file_ext, *file_name, *region_path, **valid_strings,
		*write_criterion_string;
	enum FE_write_criterion write_criterion;
	int number_of_valid_strings, return_code;
	struct Cmiss_command_data *command_data;
	struct Cmiss_region *root_region;
	struct FE_field_order_info *field_order_info;
	struct Option_table *option_table;

	ENTER(gfx_write_nodes);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		return_code = 1;
		region_path = (char *)NULL;
		if (use_data)
		{
			root_region = command_data->data_root_region;
			file_ext = data_file_ext;
		}
		else
		{
			root_region = command_data->root_region;
			file_ext = node_file_ext;
		}
		field_order_info = (struct FE_field_order_info *)NULL;
		file_name = (char *)NULL;
		write_criterion = FE_WRITE_COMPLETE_GROUP;

		option_table = CREATE(Option_table)();
		/* complete_group|with_all_listed_fields|with_any_listed_fields */ 
		write_criterion_string =
			ENUMERATOR_STRING(FE_write_criterion)(write_criterion);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(FE_write_criterion)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(FE_write_criterion) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
			valid_strings, &write_criterion_string);
		DEALLOCATE(valid_strings);
		/* fields */
		Option_table_add_entry(option_table, "fields", &field_order_info,
			Cmiss_region_get_FE_region(command_data->root_region),
			set_FE_fields_FE_region);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* default option: file name */
		Option_table_add_entry(option_table, (char *)NULL, &file_name,
			NULL, set_name);

		if (return_code = Option_table_multi_parse(option_table, state))
		{
			STRING_TO_ENUMERATOR(FE_write_criterion)(write_criterion_string,
				&write_criterion);
			if ((!field_order_info ||
				(0 == get_FE_field_order_info_number_of_fields(field_order_info))) &&
				(FE_WRITE_COMPLETE_GROUP != write_criterion))
			{
				display_message(WARNING_MESSAGE,
					"gfx_write_nodes.  Must specify fields to use %s",
					write_criterion_string);
				return_code = 0;
				
			}
			if (!file_name)
			{
				 if (!(file_name = confirmation_get_write_filename(file_ext,
								 command_data->user_interface
#if defined (WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /* defined (WX_USER_INTERFACE) */
																													)))
				{
					return_code = 0;
				}
			}
#if defined (WX_USER_INTERFACE) && (__WIN32__)
			if (file_name)
			{
				 file_name = CMISS_set_directory_and_filename_WIN32(file_name,
						command_data); 
			}
#endif /* defined (WX_USER_INTERFACE) && (__WIN32__) */
			if (return_code)
			{
				/* open the file */
				if (return_code = check_suffix(&file_name, file_ext))
				{
					return_code = write_exregion_file_of_name(file_name, root_region,
						region_path, /*write_elements*/0, /*write_nodes*/1,
						write_criterion, field_order_info);
				}
			}
		}
		DESTROY(Option_table)(&option_table);
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
		if (field_order_info)
		{
			DESTROY(FE_field_order_info)(&field_order_info);
		}
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_write_nodes.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_nodes */

static int gfx_write_region(struct Parse_state *state,
	void *dummy, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2003

DESCRIPTION :
If a nodes file is not specified a file selection box is presented to the user,
otherwise the nodes file is written.
Can now specify individual node groups to write with the <group> option.
If <use_data> is set, writing data, otherwise writing nodes.
==============================================================================*/
{
	char file_ext[] = ".fml", *file_name, *region_path;
	FILE *file;
	int return_code;
	struct Cmiss_command_data *command_data;
	struct FE_field_order_info *field_order_info;
	struct Option_table *option_table;

	ENTER(gfx_write_region);
	USE_PARAMETER(dummy);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		return_code = 1;
		Cmiss_region_get_root_region_path(&region_path);
		field_order_info = (struct FE_field_order_info *)NULL;
		file_name = (char *)NULL;

		option_table = CREATE(Option_table)();
		/* fields */
		Option_table_add_entry(option_table, "fields", &field_order_info,
			Cmiss_region_get_FE_region(command_data->root_region),
			set_FE_fields_FE_region);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			command_data->root_region, set_Cmiss_region_path);
		/* default option: file name */
		Option_table_add_entry(option_table, (char *)NULL, &file_name,
			NULL, set_name);

		if (return_code = Option_table_multi_parse(option_table, state))
		{
			if (!file_name)
			{
				if (!(file_name = confirmation_get_write_filename(file_ext,
					command_data->user_interface
#if defined (WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /* defined (WX_USER_INTERFACE) */
																													)))
				{
					return_code = 0;
				}
			}
#if defined (WX_USER_INTERFACE) && (__WIN32__)
			if (file_name)
			{
				 file_name = CMISS_set_directory_and_filename_WIN32(file_name,
						command_data); 
			}
#endif /* defined (WX_USER_INTERFACE) && (__WIN32__) */
			if (return_code)
			{
				/* open the file */
				if (return_code = check_suffix(&file_name, file_ext))
				{
					file = fopen(file_name, "w");
					return_code =
						write_fieldml_file(file, command_data->root_region, region_path, 1, 1, field_order_info);
					fclose(file);
				}
			}
		}
		DESTROY(Option_table)(&option_table);
		if (field_order_info)
		{
			DESTROY(FE_field_order_info)(&field_order_info);
		}
		DEALLOCATE(region_path);
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_write_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_region */

static int gfx_write_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
Executes a GFX WRITE TEXTURE command.
==============================================================================*/
{
	char *current_token, *file_name, *file_number_pattern,
		*image_file_format_string, **valid_strings;
	enum Image_file_format image_file_format;
	int number_of_bytes_per_component, number_of_valid_strings,
		original_depth_texels, original_height_texels, original_width_texels,
		return_code;
	struct Cmgui_image *cmgui_image;
	struct Cmgui_image_information *cmgui_image_information;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Texture *texture;

	ENTER(gfx_write_texture);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct Cmiss_command_data *)command_data_void))
	{
		texture = (struct Texture *)NULL;
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (command_data->texture_manager)
				{
					if (texture = FIND_BY_IDENTIFIER_IN_MANAGER(Texture,name)
						(current_token, command_data->texture_manager))
					{
						return_code = shift_Parse_state(state, 1);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx write texture:  Unknown texture : %s",current_token);
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_write_texture.  Missing texture manager");
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " TEXTURE_NAME");
				return_code = 1;
				/* by not shifting parse state the rest of the help should come out */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx write texture:  Missing texture name");
			return_code = 0;
		}
		if (return_code)
		{
			/* initialize defaults */
			file_name = (char *)NULL;
			file_number_pattern = (char *)NULL;
			/* default file format is to obtain it from the filename extension */
			image_file_format = UNKNOWN_IMAGE_FILE_FORMAT;
			if (texture)
			{
				/* by default, save as much information as there is in the texture */
				number_of_bytes_per_component =
					Texture_get_number_of_bytes_per_component(texture);
			}
			else
			{
				number_of_bytes_per_component = 1;
			}

			option_table = CREATE(Option_table)();
			/* image file format */
			image_file_format_string =
				ENUMERATOR_STRING(Image_file_format)(image_file_format);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Image_file_format)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Image_file_format) *)NULL,
				(void *)NULL);
			Option_table_add_enumerator(option_table, number_of_valid_strings,
				valid_strings, &image_file_format_string);
			/* bytes_per_component */
			Option_table_add_entry(option_table, "bytes_per_component",
				&number_of_bytes_per_component, (void *)NULL, set_int_positive);
			/* file */
			Option_table_add_entry(option_table, "file", &file_name,
				(void *)1, set_name);
			/* number_pattern */
			Option_table_add_entry(option_table, "number_pattern",
				&file_number_pattern, (void *)1, set_name);
			DEALLOCATE(valid_strings);
			return_code = Option_table_multi_parse(option_table, state);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (!file_name)
				{
					if (!(file_name = confirmation_get_write_filename(NULL,
									 command_data->user_interface
#if defined (WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /* defined (WX_USER_INTERFACE) */
																														)))
					{
						display_message(ERROR_MESSAGE,
							"gfx write texture:  No file name specified");
						return_code = 0;
					}					
				}
				if ((1 != number_of_bytes_per_component) &&
					(2 != number_of_bytes_per_component))
				{
					display_message(ERROR_MESSAGE,
						"gfx write texture:  bytes_per_component may be 1 or 2");
					return_code = 0;
				}
			}
			if (return_code)
			{
				cmgui_image_information = CREATE(Cmgui_image_information)();
				if (image_file_format_string)
				{
					STRING_TO_ENUMERATOR(Image_file_format)(
						image_file_format_string, &image_file_format);
				}
				Cmgui_image_information_set_image_file_format(
					cmgui_image_information, image_file_format);
				Cmgui_image_information_set_number_of_bytes_per_component(
					cmgui_image_information, number_of_bytes_per_component);
				Cmgui_image_information_set_io_stream_package(cmgui_image_information,
					command_data->io_stream_package);
				if (file_number_pattern)
				{
					if (strstr(file_name, file_number_pattern))
					{
						/* number images from 1 to the number of depth texels used */
						if (Texture_get_original_size(texture, &original_width_texels,
							&original_height_texels, &original_depth_texels))
						{
							Cmgui_image_information_set_file_name_series(
								cmgui_image_information, file_name, file_number_pattern,
								/*start_file_number*/1,
								/*stop_file_number*/original_depth_texels,
								/*file_number_increment*/1);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE, "gfx write texture:  "
							"File number pattern \"%s\" not found in file name \"%s\"",
							file_number_pattern, file_name);
						return_code = 0;
					}
				}
				else
				{
					Cmgui_image_information_add_file_name(cmgui_image_information,
						file_name);
				}
				if (return_code)
				{
					if (cmgui_image = Texture_get_image(texture))
					{
						if (!Cmgui_image_write(cmgui_image, cmgui_image_information))
						{
							display_message(ERROR_MESSAGE,
								"gfx write texture:  Error writing image %s", file_name);
							return_code = 0;
						}
						DESTROY(Cmgui_image)(&cmgui_image);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_write_texture.  Could not get image from texture");
						return_code = 0;
					}
				}
				DESTROY(Cmgui_image_information)(&cmgui_image_information);
			}
			DESTROY(Option_table)(&option_table);
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
			if (file_number_pattern)
			{
				DEALLOCATE(file_number_pattern);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_write_texture.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_texture */

static int execute_command_gfx_write(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 March 2003

DESCRIPTION :
Executes a GFX WRITE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_write);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			 //#if defined (NEW_CODE) 
			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table, "all", NULL,
				command_data_void, gfx_write_All);
			//#endif /* defined NEW_CODE) */
			Option_table_add_entry(option_table, "curve", NULL,
				command_data_void, gfx_write_Curve);
			Option_table_add_entry(option_table, "data", /*use_data*/(void *)1,
				command_data_void, gfx_write_nodes);
			Option_table_add_entry(option_table, "elements", NULL,
				command_data_void, gfx_write_elements);
			Option_table_add_entry(option_table, "nodes", /*use_data*/(void *)0,
				command_data_void, gfx_write_nodes);
			Option_table_add_entry(option_table, "region", NULL,
				command_data_void, gfx_write_region);
			Option_table_add_entry(option_table, "texture", NULL,
				command_data_void, gfx_write_texture);
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx write", command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_write.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_write */

static int execute_command_gfx(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Executes a GFX command.
==============================================================================*/
{
	int return_code;
	struct Option_table *option_table;
	struct Minimisation_package *minimisation_package;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_gfx);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct Cmiss_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			option_table=CREATE(Option_table)();
			minimisation_package = CREATE(Minimisation_package)(
				command_data->default_time_keeper, command_data->computed_field_package,
				command_data->root_region);
			Option_table_add_entry(option_table, "change_identifier", NULL,
				command_data_void, gfx_change_identifier);
			Option_table_add_entry(option_table, "convert", NULL,
				command_data_void, gfx_convert);
			Option_table_add_entry(option_table, "create", NULL,
				command_data_void, execute_command_gfx_create);
#if defined (MOTIF) || (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "data_tool", /*data_tool*/(void *)1,
			   command_data_void, execute_command_gfx_node_tool);
#endif /* defined (MOTIF) || (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (WX_USER_INTERFACE)*/
			Option_table_add_entry(option_table, "define", NULL,
				command_data_void, execute_command_gfx_define);
			Option_table_add_entry(option_table, "destroy", NULL,
				command_data_void, execute_command_gfx_destroy);
			Option_table_add_entry(option_table, "draw", NULL,
				command_data_void, execute_command_gfx_draw);
			Option_table_add_entry(option_table, "edit", NULL,
				command_data_void, execute_command_gfx_edit);
#if defined (MOTIF)
			Option_table_add_entry(option_table, "element_creator", NULL,
				command_data_void, execute_command_gfx_element_creator);
#endif /* defined (MOTIF) */
#if defined (MOTIF) || (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE)  || defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "element_point_tool", NULL,
				command_data_void, execute_command_gfx_element_point_tool);
#endif /* defined (MOTIF) || (GTK_USER_INTERFACE) || defined
					(WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE)  || defined (WX_USER_INTERFACE)*/
#if defined (MOTIF) || (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "element_tool", NULL,
				command_data_void, execute_command_gfx_element_tool);
#endif /* defined (MOTIF) || (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE) */
			Option_table_add_entry(option_table, "erase", NULL,
				command_data_void, execute_command_gfx_erase);
			Option_table_add_entry(option_table, "evaluate", NULL,
				command_data_void, gfx_evaluate);
			Option_table_add_entry(option_table, "export", NULL,
				command_data_void, execute_command_gfx_export);
#if defined (OLD_CODE)
			Option_table_add_entry(option_table, "filter", NULL,
				command_data_void, execute_command_gfx_filter);
#endif /* defined (OLD_CODE) */
			Option_table_add_entry(option_table, "list", NULL,
				command_data_void, execute_command_gfx_list);
			Option_table_add_entry(option_table, "minimise",
				NULL, minimisation_package, gfx_minimise);
			Option_table_add_entry(option_table, "modify", NULL,
				command_data_void, execute_command_gfx_modify);
#if defined (SGI_MOVIE_FILE)
			Option_table_add_entry(option_table, "movie", NULL,
				command_data_void, gfx_movie);
#endif /* defined (SGI_MOVIE_FILE) */
#if defined (MOTIF) || (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "node_tool", /*data_tool*/(void *)0,
				command_data_void, execute_command_gfx_node_tool);
#endif /* defined (MOTIF) || (GTK_USER_INTERFACE) || defined
					(WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE) */
#if defined (MOTIF) || (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "print", NULL,
				command_data_void, execute_command_gfx_print);
#endif /* defined (MOTIF) */
			Option_table_add_entry(option_table, "read", NULL,
				command_data_void, execute_command_gfx_read);
			Option_table_add_entry(option_table, "select", NULL,
				command_data_void, execute_command_gfx_select);
			Option_table_add_entry(option_table, "set", NULL,
				command_data_void, execute_command_gfx_set);
			Option_table_add_entry(option_table, "smooth", NULL,
				command_data_void, execute_command_gfx_smooth);
			Option_table_add_entry(option_table, "timekeeper", NULL,
				command_data_void, gfx_timekeeper);
			Option_table_add_entry(option_table, "transform_tool", NULL,
				command_data_void, gfx_transform_tool);
			Option_table_add_entry(option_table, "unselect", NULL,
				command_data_void, execute_command_gfx_unselect);
#if defined (MOTIF) || defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "update", NULL,
				command_data_void, execute_command_gfx_update);
#endif /* defined (MOTIF) || defined (WX_USER_INTERFACE) */
			Option_table_add_entry(option_table, "write", NULL,
				command_data_void, execute_command_gfx_write);
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			DESTROY(Minimisation_package)(&minimisation_package);
		}
		else
		{
			set_command_prompt("gfx",command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "execute_command_gfx.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx */

static int execute_command_cm(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 May 2003

DESCRIPTION :
Executes a cm (back end) command.
==============================================================================*/
{
#if defined (LINK_CMISS)
	char *current_token;
#endif /* defined (LINK_CMISS) */
	char *prompt;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_cm);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
#if defined (LINK_CMISS)
				current_token=state->current_token;
				if (CMISS)
				{
					/* somehow extract the whole command */
					return_code=CMISS_connection_process_command(&CMISS,
						state->command_string, Command_window_get_message_pane(
                  command_data->command_window));
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						display_message(ERROR_MESSAGE,
							"execute_command_cm.	No CMISS connection");
						return_code=0;
					}
					else
					{
						return_code=1;
					}
				}
#else /* defined (LINK_CMISS) */
				display_message(ERROR_MESSAGE,"execute_command_cm.	Define LINK_CMISS");
				return_code=0;
#endif /* defined (LINK_CMISS) */
#if defined (OLD_CODE)
				if (open_socket(command_data->command_window,
					command_data->basis_manager,command_data->node_manager,
					command_data->element_group_manager,command_data->node_group_manager,
					&(command_data->prompt_window),command_data->user_interface))
				{
#if defined (MOTIF)
					write_socket(state->command_string,CONN_ID1);
#endif /* defined (MOTIF) */
					return_code=1;
				}
				else
				{
					return_code=0;
				}
#endif /* defined (OLD_CODE) */
			}
			else
			{
				if (prompt=(char *)prompt_void)
				{
					set_command_prompt(prompt,command_data);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_cm.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_cm.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cm */

#if defined (CELL)
static int execute_command_cell_open(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 26 March 2001

DESCRIPTION :
Executes a CELL OPEN command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cell_interface *cell_interface;
	struct Cmiss_command_data *command_data;
	
	ENTER(execute_command_cell_open);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (!((current_token=state->current_token)&&
				!(strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				if (!(cell_interface=command_data->cell_interface))
				{
					/* add Cell 3D */
					if (cell_interface = CREATE(Cell_interface)(
						command_data->any_object_selection,
						&(command_data->background_colour),
						Material_package_get_default_material(command_data->material_package),
						command_data->default_light,
						command_data->default_light_model,
						command_data->default_scene,
						command_data->default_spectrum,
						command_data->default_time_keeper,
						command_data->graphics_object_list,
						command_data->glyph_list,
						command_data->interactive_tool_manager,
						command_data->light_manager,
						command_data->light_model_manager,
						Material_package_get_material_manager(command_data->material_package),
						command_data->scene_manager,
						command_data->spectrum_manager,
						command_data->texture_manager,
						command_data->user_interface,
						close_cell_window
#if defined (CELL_DISTRIBUTED)
						,command_data->element_point_ranges_selection,
						command_data->computed_field_package,
						command_data->element_manager,
						command_data->element_group_manager,
						command_data->fe_field_manager
#endif /* defined (CELL_DISTRIBUTED) */
						))
					{
						command_data->cell_interface=cell_interface;
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"execute_command_cell_open.  "
							"Could not create the cell_interface");
					}
				}
        else
        {
          /* Cell already exists, so pop it up */
          return_code = Cell_interface_pop_up(command_data->cell_interface);
        }
			}
			else
			{
				/* no help */
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_cell_open.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_cell_open.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cell_open */
#endif /* defined (CELL) */

#if defined (CELL)
static int execute_command_cell_close(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Executes a CELL CLOSE command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cell_interface *cell_interface;
	struct Cmiss_command_data *command_data;
  
	ENTER(execute_command_cell_close);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (!((current_token=state->current_token)&&
				!(strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
        if (cell_interface=command_data->cell_interface)
        {
          return_code = DESTROY(Cell_interface)(&cell_interface);
          command_data->cell_interface = (struct Cell_interface *)NULL;
        }
        else
        {
          display_message(ERROR_MESSAGE,"execute_command_close.  "
            "Missing Cell interface");
          return_code = 0;
        }
      }
			else
			{
				/* no help */
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_cell_close.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_cell_close.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cell_close() */
#endif /* defined (CELL) */

#if defined (CELL)
static int execute_command_cell_read_model(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 January 2003

DESCRIPTION :
Executes a CELL READ MODEL command.
==============================================================================*/
{
	char *current_token,*file_name;
	int return_code;
	struct Cell_interface *cell_interface;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_cell_read_model);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data = (struct Cmiss_command_data *)command_data_void)
		{
			file_name = (char *)NULL;
			option_table=CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
				&file_name, &(command_data->example_directory), set_file_name);
			/* default */
			Option_table_add_entry(option_table, NULL, &file_name,
				NULL, set_file_name);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
      {
        if (!((current_token=state->current_token)&&
          !(strcmp(PARSER_HELP_STRING,current_token)&&
            strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
        {
          if (!file_name)
          {
            if (!(file_name = confirmation_get_read_filename(".cell.xml",
              command_data->user_interface)))
            {
              return_code = 0;
            }
          }
          if (file_name)
          {
            if (check_suffix(&file_name,".cell.xml"))
            {
              if (cell_interface=command_data->cell_interface)
              {
                return_code = Cell_interface_read_model(cell_interface,
                  file_name);
              }
              else
              {
                display_message(ERROR_MESSAGE,
                  "execute_command_cell_read_model. "
                  "Missing Cell interface");
                return_code = 0;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,
                "execute_command_cell_read_model. "
                "Invalid file name: %s",file_name);
              return_code = 0;
            }
            DEALLOCATE(file_name);
          }
          else
          {
            display_message(ERROR_MESSAGE,"execute_command_cell_read_model. "
              "Unable to get the file name");
            return_code = 0;
          }
        }
        else
        {
          /* no help */
          return_code=1;
        }
      }
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_cell_read_model.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_cell_read_model.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cell_read_model */
#endif /* defined (CELL) */
        
#if defined (CELL)
static int execute_command_cell_read(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
Executes a CELL READ command.
==============================================================================*/
{
	int return_code;
	static struct Option_table *option_table;

	ENTER(execute_command_cell_read);
  USE_PARAMETER(prompt_void);
	/* check argument */
	if (state)
	{
		option_table = CREATE(Option_table)();
    Option_table_add_entry(option_table,"model",NULL,
      command_data_void,execute_command_cell_read_model);
    return_code = Option_table_parse(option_table,state);
    DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_cell_read.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cell_read */
#endif /* defined (CELL) */

#if defined (CELL)
static int execute_command_cell_write_model(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 January 2003

DESCRIPTION :
Executes a CELL WRITE MODEL command.
==============================================================================*/
{
	char *current_token,*file_name;
	int return_code;
	struct Cell_interface *cell_interface;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_cell_write_model);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data = (struct Cmiss_command_data *)command_data_void)
		{
			file_name = (char *)NULL;
			option_table=CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
				&file_name, &(command_data->example_directory), set_file_name);
			/* default */
			Option_table_add_entry(option_table, NULL, &file_name,
				NULL, set_file_name);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
      {
        if (!((current_token=state->current_token)&&
          !(strcmp(PARSER_HELP_STRING,current_token)&&
            strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
        {
          if (!file_name)
          {
            if (!(file_name = confirmation_get_write_filename(".cell.xml",
              command_data->user_interface)))
            {
              return_code = 0;
            }
          }
          if (file_name)
          {
            if (check_suffix(&file_name,".cell.xml"))
            {
              if (cell_interface=command_data->cell_interface)
              {
                return_code = Cell_interface_write_model(cell_interface,
                  file_name);
              }
              else
              {
                display_message(ERROR_MESSAGE,
                  "execute_command_cell_write_model. "
                  "Missing Cell interface");
                return_code = 0;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,
                "execute_command_cell_write_model. "
                "Invalid file name: %s",file_name);
              return_code = 0;
            }
            DEALLOCATE(file_name);
          }
          else
          {
            display_message(ERROR_MESSAGE,"execute_command_cell_write_model. "
              "Unable to get the file name");
            return_code = 0;
          }
        }
        else
        {
          /* no help */
          return_code=1;
        }
      }
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_cell_write_model.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_cell_write_model.  Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* execute_command_cell_write_model */
#endif /* defined (CELL) */
        
#if defined (CELL)
static int execute_command_cell_write_ipcell(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 January 2003

DESCRIPTION :
Executes a CELL WRITE IPCELL command.
==============================================================================*/
{
	char *current_token,*file_name;
	int return_code;
	struct Cell_interface *cell_interface;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_cell_write_ipcell);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data = (struct Cmiss_command_data *)command_data_void)
		{
			file_name = (char *)NULL;
			option_table=CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
				&file_name, &(command_data->example_directory), set_file_name);
			/* default */
			Option_table_add_entry(option_table, NULL, &file_name,
				NULL, set_file_name);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
      {
        if (!((current_token=state->current_token)&&
          !(strcmp(PARSER_HELP_STRING,current_token)&&
            strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
        {
          if (!file_name)
          {
            if (!(file_name = confirmation_get_write_filename(".ipcell",
              command_data->user_interface)))
            {
              return_code = 0;
            }
          }
#if defined (WX_USER_INTERFACE) && (__WIN32__)
			if (file_name)
			{
				 file_name = CMISS_set_directory_and_filename_WIN32(file_name,
						command_data); 
			}
#endif /* defined (WX_USER_INTERFACE) && (__WIN32__) */
          if (file_name)
          {
            if (check_suffix(&file_name,".ipcell"))
            {
              if (cell_interface=command_data->cell_interface)
              {
                return_code = Cell_interface_write_model_to_ipcell_file(
                  cell_interface,file_name);
              }
              else
              {
                display_message(ERROR_MESSAGE,
                  "execute_command_cell_write_ipcell. "
                  "Missing Cell interface");
                return_code = 0;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,
                "execute_command_cell_write_ipcell. "
                "Invalid file name: %s",file_name);
              return_code = 0;
            }
            DEALLOCATE(file_name);
          }
          else
          {
            display_message(ERROR_MESSAGE,"execute_command_cell_write_ipcell. "
              "Unable to get the file name");
            return_code = 0;
          }
        }
        else
        {
          /* no help */
          return_code=1;
        }
      }
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_cell_write_ipcell.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_cell_write_ipcell.  Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* execute_command_cell_write_ipcell */
#endif /* defined (CELL) */
        
#if defined (CELL)
static int execute_command_cell_write(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 02 April 2001

DESCRIPTION :
Executes a CELL WRITE command.
==============================================================================*/
{
	int return_code;
	static struct Option_table *option_table;

	ENTER(execute_command_cell_write);
  USE_PARAMETER(prompt_void);
	/* check argument */
	if (state)
	{
		option_table = CREATE(Option_table)();
    Option_table_add_entry(option_table,"model",NULL,
      command_data_void,execute_command_cell_write_model);
    Option_table_add_entry(option_table,"ipcell",NULL,
      command_data_void,execute_command_cell_write_ipcell);
    return_code = Option_table_parse(option_table,state);
    DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_cell_write.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cell_write */
#endif /* defined (CELL) */

#if defined (CELL)
static int cell_list_XMLParser_properties(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Executes a CELL LIST XMLPARSER_PROPERTIES command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;

	ENTER(cell_list_XMLParser_properties);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (!((state->current_token)&&
      !(strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
    {
      if (command_data=(struct Cmiss_command_data *)command_data_void)
      {
        if (cell_interface = command_data->cell_interface)
        {
          /* ??? Should just do a get_XMLParser_properties and then do the
           * listing here ???
           */
          return_code =
            Cell_interface_list_XMLParser_properties(cell_interface);
        }
        else
        {
          display_message(ERROR_MESSAGE,"cell_list_XMLParser_properties.  "
            "Missing Cell interface");
          return_code=0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "cell_list_XMLParser_properties.  Missing command_data");
        return_code=0;
      }
    }
    else
    {
      /* no help */
      return_code = 1;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_list_XMLParser_properties.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* cell_list_XMLParser_properties() */
#endif /* defined (CELL) */

#if defined (CELL)
static int cell_set_XMLParser_properties(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
Executes a CELL SET XMLPARSER_PROPERTIES command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;
  static struct Option_table *option_table;
  int *XMLParser_properties,i;

	ENTER(cell_set_XMLParser_properties);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    /* Leave this check here as we require an existing Cell interface to be
     * able to get the default values for the option table
     */
    if (!((state->current_token)&&
      !(strcmp(PARSER_HELP_STRING,state->current_token)&&
      strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
    {
      if (command_data=(struct Cmiss_command_data *)command_data_void)
      {
        if (cell_interface = command_data->cell_interface)
        {
          /* initialise defaults */
          if (XMLParser_properties =
            Cell_interface_get_XMLParser_properties(cell_interface))
          {
            option_table = CREATE(Option_table)();
            i = 0;
            /* validation */
            Option_table_add_entry(option_table,"do_validation",
              XMLParser_properties+i,NULL,set_int);
            i++;
            /* namespaces */
            Option_table_add_entry(option_table,"do_namespaces",
              XMLParser_properties+i,NULL,set_int);
            i++;
            /* entity expansion */
            Option_table_add_entry(option_table,"do_expand",
              XMLParser_properties+i,NULL,set_int);
            if (return_code = Option_table_multi_parse(option_table,state))
            {
              if (!Cell_interface_set_XMLParser_properties(cell_interface,
                XMLParser_properties))
              {
                display_message(ERROR_MESSAGE,"cell_set_XMLParser_properties.  "
                  "Unable to set the current XML parser properties");
                return_code=0;
              }
            }
            DESTROY(Option_table)(&option_table);
            DEALLOCATE(XMLParser_properties);
          }
          else
          {
            display_message(ERROR_MESSAGE,"cell_set_XMLParser_properties.  "
              "Unable to get the current XML parser properties");
            return_code=0;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"cell_set_XMLParser_properties.  "
            "Missing Cell interface");
          return_code=0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "cell_set_XMLParser_properties.  Missing command_data");
        return_code=0;
      }
    }
    else
    {
      /* no help ?? */
      return_code = 1;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_set_XMLParser_properties.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cell_set_XMLParser_properties() */
#endif /* defined (CELL) */

#if defined (CELL)
static int cell_set_variable_value(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 03 April 2001

DESCRIPTION :
Executes a CELL SET VARIABLE_VALUE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;
  struct Option_table *option_table;
  char *component_name,*variable_name,*value_string;

	ENTER(cell_set_variable_value);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (command_data=(struct Cmiss_command_data *)command_data_void)
    {
      option_table = CREATE(Option_table)();
      component_name = (char *)NULL;
      Option_table_add_entry(option_table,"component",&component_name,
        (void *)1,set_name);
      variable_name = (char *)NULL;
      Option_table_add_entry(option_table,"name",&variable_name,
        (void *)1,set_name);
      value_string = (char *)NULL;
      Option_table_add_entry(option_table,"value",&value_string,
        (void *)1,set_name);
      if (return_code = Option_table_multi_parse(option_table,state))
      {
        if (!((state->current_token)&&
          !(strcmp(PARSER_HELP_STRING,state->current_token)&&
            strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
        {
          if (cell_interface = command_data->cell_interface)
          {
            Cell_interface_set_variable_value_from_string(cell_interface,
              component_name,variable_name,value_string);
          }
          else
          {
            display_message(ERROR_MESSAGE,"cell_set_variable_value.  "
              "Missing Cell interface");
            return_code=0;
          }
        }
        else
        {
          /* no help */
          return_code = 1;
        }
      }
      if (component_name)
      {
        DEALLOCATE(component_name);
      }
      if (variable_name)
      {
        DEALLOCATE(variable_name);
      }
      if (value_string)
      {
        DEALLOCATE(value_string);
      }
      DESTROY(Option_table)(&option_table);
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "cell_set_variable_value.  Missing command_data");
      return_code=0;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_set_variable_value.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* cell_set_variable_value() */
#endif /* defined (CELL) */

#if defined (CELL)
static int cell_set_calculate(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 03 April 2001

DESCRIPTION :
Executes a CELL SET CALCULATE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;
  struct Option_table *option_table;
  float Tstart,Tend,dT,tabT;
  char *model_routine_name,*model_dso_name,*intg_routine_name,*intg_dso_name,
    *data_file_name;

	ENTER(cell_set_calculate);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (command_data=(struct Cmiss_command_data *)command_data_void)
    {
      if (!((state->current_token)&&
        !(strcmp(PARSER_HELP_STRING,state->current_token)&&
          strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
      {
        if (cell_interface = command_data->cell_interface)
        {
          option_table = CREATE(Option_table)();
          Tstart = Cell_interface_get_start_time(cell_interface);
          Option_table_add_entry(option_table,"start_time",&Tstart,
            NULL,set_float);
          Tend = Cell_interface_get_end_time(cell_interface);
          Option_table_add_entry(option_table,"end_time",&Tend,
            NULL,set_float);
          dT = Cell_interface_get_dt(cell_interface);
          Option_table_add_entry(option_table,"dt",&dT,
            NULL,set_float);
          tabT = Cell_interface_get_tabt(cell_interface);
          Option_table_add_entry(option_table,"tab",&tabT,
            NULL,set_float);
          model_routine_name =
            Cell_interface_get_model_routine_name(cell_interface);
          Option_table_add_entry(option_table,"model_routine",
            &model_routine_name,(void *)1,set_name);
          model_dso_name =
            Cell_interface_get_model_dso_name(cell_interface);
          Option_table_add_entry(option_table,"model_dso",
            &model_dso_name,NULL,set_file_name);
          intg_routine_name =
            Cell_interface_get_intg_routine_name(cell_interface);
          Option_table_add_entry(option_table,"integrator_routine",
            &intg_routine_name,(void *)1,set_name);
          intg_dso_name =
            Cell_interface_get_intg_dso_name(cell_interface);
          Option_table_add_entry(option_table,"integrator_dso",
            &intg_dso_name,NULL,set_file_name);
          data_file_name =
            Cell_interface_get_data_file_name(cell_interface);
          Option_table_add_entry(option_table,"data_file",
            &data_file_name,NULL,set_file_name);
          if (return_code = Option_table_multi_parse(option_table,state))
          {
            /* check for essential parameters */
            if (model_routine_name && intg_routine_name && (Tstart <= Tend) &&
              (dT > 0) && (tabT > 0))
            {
              return_code = Cell_interface_set_calculate(cell_interface,Tstart,
                Tend,dT,tabT,model_routine_name,model_dso_name,
                intg_routine_name,intg_dso_name,data_file_name);
            }
            else
            {
              display_message(ERROR_MESSAGE,"cell_set_calculate.  "
                "Invalid parameters");
              return_code = 0;
            }
          }
          DESTROY(Option_table)(&option_table);
          if (model_routine_name)
          {
            DEALLOCATE(model_routine_name);
          }
          if (model_dso_name)
          {
            DEALLOCATE(model_dso_name);
          }
          if (intg_routine_name)
          {
            DEALLOCATE(intg_routine_name);
          }
          if (intg_dso_name)
          {
            DEALLOCATE(intg_dso_name);
          }
          if (data_file_name)
          {
            DEALLOCATE(data_file_name);
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"cell_set_calculate.  "
            "Missing Cell interface");
          return_code=0;
        }
      }
      else
      {
        /* ??? Is this a good way to get the help out ???
         */
        option_table = CREATE(Option_table)();
        Tstart = 0.0;
        Option_table_add_entry(option_table,"start_time",&Tstart,
          NULL,set_float);
        Tend = 0.0;
        Option_table_add_entry(option_table,"end_time",&Tend,
          NULL,set_float);
        dT = 0.0;
        Option_table_add_entry(option_table,"dt",&dT,
          NULL,set_float);
        tabT = 0.0;
        Option_table_add_entry(option_table,"tab",&tabT,
          NULL,set_float);
        model_routine_name = (char *)NULL;
        Option_table_add_entry(option_table,"model_routine",
          &model_routine_name,(void *)1,set_name);
        model_dso_name = (char *)NULL;
        Option_table_add_entry(option_table,"model_dso",
          &model_dso_name,NULL,set_file_name);
        intg_routine_name = (char *)NULL;
        Option_table_add_entry(option_table,"integrator_routine",
          &intg_routine_name,(void *)1,set_name);
        intg_dso_name = (char *)NULL;
        Option_table_add_entry(option_table,"integrator_dso",
          &intg_dso_name,NULL,set_file_name);
        data_file_name = (char *)NULL;
        Option_table_add_entry(option_table,"data_file",
          &data_file_name,NULL,set_file_name);
        return_code = Option_table_multi_parse(option_table,state);
        DESTROY(Option_table)(&option_table);
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "cell_set_calculate.  Missing command_data");
      return_code=0;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_set_calculate.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* cell_set_calculate() */
#endif /* defined (CELL) */

#if defined (CELL)
static int cell_list_copy_tags(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Executes a CELL LIST COPY_TAGS command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;

	ENTER(cell_list_copy_tags);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (!((state->current_token)&&
      !(strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
    {
      if (command_data=(struct Cmiss_command_data *)command_data_void)
      {
        if (cell_interface = command_data->cell_interface)
        {
          return_code =
            Cell_interface_list_copy_tags(cell_interface);
        }
        else
        {
          display_message(ERROR_MESSAGE,"cell_list_copy_tags.  "
            "Missing Cell interface");
          return_code=0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "cell_list_copy_tags.  Missing command_data");
        return_code=0;
      }
    }
    else
    {
      /* no help */
      return_code = 1;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_list_copy_tags.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cell_list_copy_tags() */
#endif /* defined (CELL) */

#if defined (HOW_CAN_I_DO_THIS)
#if defined (CELL)
static int cell_set_copy_tags(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Executes a CELL SET COPY_TAGS command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;
  static struct Option_table *option_table;
  int i;
  char **copy_tags;

	ENTER(cell_set_copy_tags);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (!((state->current_token)&&
      !(strcmp(PARSER_HELP_STRING,state->current_token)&&
      strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
    {
      if (command_data=(struct Cmiss_command_data *)command_data_void)
      {
        if (cell_interface = command_data->cell_interface)
        {
          /* initialise defaults */
          if (copy_tags =
            Cell_interface_get_copy_tags(cell_interface))
          {
          }
          else
          {
            display_message(ERROR_MESSAGE,"cell_set_XMLParser_properties.  "
              "Unable to get the current XML parser properties");
            return_code=0;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"cell_set_XMLParser_properties.  "
            "Missing Cell interface");
          return_code=0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "cell_set_XMLParser_properties.  Missing command_data");
        return_code=0;
      }
    }
    else
    {
      /* no help ?? */
      return_code = 1;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_set_XMLParser_properties.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cell_set_XMLParser_properties() */
#endif /* defined (CELL) */
#endif /* defined (HOW_CAN_I_DO_THIS) */

#if defined (CELL)
static int cell_list_ref_tags(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Executes a CELL LIST REF_TAGS command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;

	ENTER(cell_list_ref_tags);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (!((state->current_token)&&
      !(strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
    {
      if (command_data=(struct Cmiss_command_data *)command_data_void)
      {
        if (cell_interface = command_data->cell_interface)
        {
          return_code =
            Cell_interface_list_ref_tags(cell_interface);
        }
        else
        {
          display_message(ERROR_MESSAGE,"cell_list_ref_tags.  "
            "Missing Cell interface");
          return_code=0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "cell_list_ref_tags.  Missing command_data");
        return_code=0;
      }
    }
    else
    {
      /* no help */
      return_code = 1;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_list_ref_tags.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cell_list_ref_tags() */
#endif /* defined (CELL) */

#if defined (CELL)
static int cell_list_calculate(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Executes a CELL LIST CALCULATE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;

	ENTER(cell_list_calculate);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (!((state->current_token)&&
      !(strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
    {
      if (command_data=(struct Cmiss_command_data *)command_data_void)
      {
        if (cell_interface = command_data->cell_interface)
        {
          return_code =
            Cell_interface_list_calculate(cell_interface);
        }
        else
        {
          display_message(ERROR_MESSAGE,"cell_list_calculate.  "
            "Missing Cell interface");
          return_code=0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "cell_list_calculate.  Missing command_data");
        return_code=0;
      }
    }
    else
    {
      /* no help */
      return_code = 1;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_list_calculate.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* cell_list_calculate() */
#endif /* defined (CELL) */

#if defined (CELL)
static int cell_list_components(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Executes a CELL LIST COMPONENTS command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;
  struct Option_table *option_table;
  char full;

	ENTER(cell_list_components);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (command_data=(struct Cmiss_command_data *)command_data_void)
    {
      option_table = CREATE(Option_table)();
      full = 0;
      Option_table_add_entry(option_table,"full",&full,NULL,set_char_flag);
      if (return_code = Option_table_multi_parse(option_table,state))
      {
        if (!((state->current_token)&&
          !(strcmp(PARSER_HELP_STRING,state->current_token)&&
            strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
        {
          if (cell_interface = command_data->cell_interface)
          {
            Cell_interface_list_components(cell_interface,(int)full);
          }
          else
          {
            display_message(ERROR_MESSAGE,"cell_list_components.  "
              "Missing Cell interface");
            return_code=0;
          }
        }
        else
        {
          /* no help */
          return_code = 1;
        }
      }
      DESTROY(Option_table)(&option_table);
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "cell_list_components.  Missing command_data");
      return_code=0;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_list_components.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* cell_list_components() */
#endif /* defined (CELL) */

#if defined (CELL)
static int cell_list_variables(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
Executes a CELL LIST VARIABLES command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;
  struct Option_table *option_table;
  char full,*component_name,*variable_name;

	ENTER(cell_list_variables);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (command_data=(struct Cmiss_command_data *)command_data_void)
    {
      option_table = CREATE(Option_table)();
      full = 0;
      Option_table_add_entry(option_table,"full",&full,NULL,set_char_flag);
      component_name = (char *)NULL;
      Option_table_add_entry(option_table,"component",&component_name,
        (void *)1,set_name);
      variable_name = (char *)NULL;
      Option_table_add_entry(option_table,"name",&variable_name,
        (void *)1,set_name);
      if (return_code = Option_table_multi_parse(option_table,state))
      {
        if (cell_interface = command_data->cell_interface)
        {
          if (!((state->current_token)&&
            !(strcmp(PARSER_HELP_STRING,state->current_token)&&
              strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
          {
            Cell_interface_list_variables(cell_interface,component_name,
              variable_name,(int)full);
          }
          else
          {
            /* no help */
            return_code = 1;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"cell_list_variables.  "
            "Missing Cell interface");
          return_code=0;
        }
      }
      DESTROY(Option_table)(&option_table);
      if (component_name)
      {
        DEALLOCATE(component_name);
      }
      if (variable_name)
      {
        DEALLOCATE(variable_name);
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "cell_list_variables.  Missing command_data");
      return_code=0;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_list_variables.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* cell_list_variables() */
#endif /* defined (CELL) */

#if defined (CELL)
static int cell_list_hierarchy(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Executes a CELL LIST HIERARCHY command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;
  struct Option_table *option_table;
  char full,*name;

	ENTER(cell_list_hierarchy);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (command_data=(struct Cmiss_command_data *)command_data_void)
    {
      option_table = CREATE(Option_table)();
      full = 0;
      Option_table_add_entry(option_table,"full",&full,NULL,set_char_flag);
      name = (char *)NULL;
      Option_table_add_entry(option_table,"name",&name,(void *)1,set_name);
      if (return_code = Option_table_multi_parse(option_table,state))
      {
        if (!((state->current_token)&&
          !(strcmp(PARSER_HELP_STRING,state->current_token)&&
            strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
        {
          if (cell_interface = command_data->cell_interface)
          {
            Cell_interface_list_hierarchy(cell_interface,(int)full,name);
          }
          else
          {
            display_message(ERROR_MESSAGE,"cell_list_hierarchy.  "
              "Missing Cell interface");
            return_code=0;
          }
        }
        else
        {
          /* no help */
          return_code = 1;
        }
      }
      DESTROY(Option_table)(&option_table);
      if (name)
      {
        DEALLOCATE(name);
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "cell_list_hierarchy.  Missing command_data");
      return_code=0;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_list_hierarchy.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* cell_list_hierarchy() */
#endif /* defined (CELL) */

#if defined (CELL)
static int execute_command_cell_list(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 2904 April 2001

DESCRIPTION :
Executes a CELL LIST command.
==============================================================================*/
{
	int return_code;
	static struct Option_table *option_table;

	ENTER(execute_command_cell_list);
  USE_PARAMETER(prompt_void);
	/* check argument */
	if (state)
	{
    option_table = CREATE(Option_table)();
    Option_table_add_entry(option_table,"XMLParser_properties",NULL,
      command_data_void,cell_list_XMLParser_properties);
    Option_table_add_entry(option_table,"copy_tags",NULL,
      command_data_void,cell_list_copy_tags);
    Option_table_add_entry(option_table,"ref_tags",NULL,
      command_data_void,cell_list_ref_tags);
    Option_table_add_entry(option_table,"components",NULL,
      command_data_void,cell_list_components);
    Option_table_add_entry(option_table,"variables",NULL,
      command_data_void,cell_list_variables);
    Option_table_add_entry(option_table,"hierarchy",NULL,
      command_data_void,cell_list_hierarchy);
    Option_table_add_entry(option_table,"calculate",NULL,
      command_data_void,cell_list_calculate);
    return_code = Option_table_parse(option_table,state);
    DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_cell_list.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cell_list() */
#endif /* defined (CELL) */

#if defined (CELL)
static int execute_command_cell_set(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Executes a CELL SET command.
==============================================================================*/
{
	int return_code;
	static struct Option_table *option_table;

	ENTER(execute_command_cell_set);
  USE_PARAMETER(prompt_void);
	/* check argument */
	if (state)
	{
    option_table = CREATE(Option_table)();
    Option_table_add_entry(option_table,"XMLParser_properties",NULL,
      command_data_void,cell_set_XMLParser_properties);
    Option_table_add_entry(option_table,"variable_value",NULL,
      command_data_void,cell_set_variable_value);
    Option_table_add_entry(option_table,"calculate",NULL,
      command_data_void,cell_set_calculate);
#if defined (HOW_CAN_I_DO_THIS)
    Option_table_add_entry(option_table,"copy_tags",NULL,
      command_data_void,cell_set_copy_tags);
#endif /* defined (HOW_CAN_I_DO_THIS) */
    return_code = Option_table_parse(option_table,state);
    DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_cell_set.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cell_set() */
#endif /* defined (CELL) */

#if defined (CELL)
static int execute_command_cell_calculate(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 03 April 2001

DESCRIPTION :
Executes a CELL SOLVE command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Cell_interface *cell_interface;
/*    struct Option_table *option_table; */
/*    char *data_file_name; */

	ENTER(execute_command_cell_calculate);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
    if (command_data=(struct Cmiss_command_data *)command_data_void)
    {
/*        option_table = CREATE(Option_table)(); */
/*        data_file_name = (char *)NULL; */
/*        Option_table_add_entry(option_table,"data_file",&data_file_name, */
/*          (void *)1,set_name); */
/*        if (return_code = Option_table_multi_parse(option_table,state)) */
      {
        if (!((state->current_token)&&
          !(strcmp(PARSER_HELP_STRING,state->current_token)&&
            strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
        {
          if (cell_interface = command_data->cell_interface)
          {
/*              if (data_file_name) */
/*              { */
/*                return_code = Cell_interface_set_data_file_name( */
/*                  cell_interface,data_file_name); */
/*              } */
/*              if (return_code) */
            {
              return_code = Cell_interface_calculate_model(cell_interface);
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"execute_command_cell_calculate.  "
              "Missing Cell interface");
            return_code=0;
          }
        }
        else
        {
          /* no help */
          return_code = 1;
        }
      }
/*        DESTROY(Option_table)(&option_table); */
/*        if (data_file_name) */
/*        { */
/*          DEALLOCATE(data_file_name); */
/*        } */
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "execute_command_cell_calculate.  Missing command_data");
      return_code=0;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_cell_calculate.  "
      "Missing state");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* execute_command_cell_calculate() */
#endif /* defined (CELL) */

#if defined (CELL)
static int execute_command_cell(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 02 April 2001

DESCRIPTION :
Executes a CELL command.
==============================================================================*/
{
	int return_code;
	static struct Option_table *option_table;

	ENTER(execute_command_cell);
  USE_PARAMETER(prompt_void);
	/* check argument */
	if (state)
	{
    option_table = CREATE(Option_table)();
    Option_table_add_entry(option_table,"open",NULL,command_data_void,
      execute_command_cell_open);
    Option_table_add_entry(option_table,"close",NULL,command_data_void,
      execute_command_cell_close);
    Option_table_add_entry(option_table,"list",NULL,command_data_void,
      execute_command_cell_list);
    Option_table_add_entry(option_table,"set",NULL,command_data_void,
      execute_command_cell_set);
    Option_table_add_entry(option_table,"read",NULL,command_data_void,
      execute_command_cell_read);
    Option_table_add_entry(option_table,"write",NULL,command_data_void,
      execute_command_cell_write);
    Option_table_add_entry(option_table,"calculate",NULL,command_data_void,
      execute_command_cell_calculate);
    return_code = Option_table_parse(option_table,state);
    DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_cell.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_cell */
#endif /* defined (CELL) */

static int execute_command_create(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 21 March 2001

DESCRIPTION :
Executes a CREATE command.
==============================================================================*/
{
	int return_code;
	struct Option_table *option_table;

	ENTER(execute_command_create);
	/* check argument */
	if (state)
	{
		option_table = CREATE(Option_table)();
		/* fem */
		Option_table_add_entry(option_table, "cm", NULL,
			command_data_void, gfx_create_cmiss);
		/* default */
		Option_table_add_entry(option_table, "", prompt_void, command_data_void,
			execute_command_cm);
		return_code=Option_table_parse(option_table,state);
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_create.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_create */

#if !defined (NO_HELP)
static int execute_command_help(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 24 November 1997

DESCRIPTION :
Executes a HELP command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_help);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
#if !defined (HYPERTEXT_HELP) && !defined (NETSCAPE_HELP)
#if defined (MOTIF)
					do_help(current_token);
#endif /* defined (MOTIF) */
#else
					strcpy(global_temp_string,command_data->help_url);
					strcat(global_temp_string,current_token);
					strcat(global_temp_string,"/");
#if defined (MOTIF)
					do_help(global_temp_string,command_data->examples_directory,
						command_data->execute_command,command_data->user_interface);
#endif /* defined (MOTIF) */
#endif
				}
				else
				{
					display_message(INFORMATION_MESSAGE," WORD");
					return_code=1;
				}
			}
			else
			{
#if !defined (HYPERTEXT_HELP) && !defined (NETSCAPE_HELP)
#if defined (MOTIF)
				do_help(" ",command_data->execute_command,command_data->user_interface);
#endif /* defined (MOTIF) */
#else
#if defined (MOTIF)
				do_help(command_data->help_url,command_data->examples_directory,
					command_data->execute_command,command_data->user_interface);
#endif /* defined (MOTIF) */
#endif
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_help.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_help.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_help */
#endif /* !defined (NO_HELP) */

static int execute_command_list_memory(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a LIST_MEMORY command.
==============================================================================*/
{
	char increment_counter, suppress_pointers;
	int count_number,return_code,set_counter;
	static struct Modifier_entry option_table[]=
	{
		{"increment_counter",NULL,NULL,set_char_flag},
		{"suppress_pointers",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,set_int}
	};

	ENTER(execute_command_list_memory);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		count_number=0;
		increment_counter = 0;
		suppress_pointers = 0;
		(option_table[0]).to_be_modified= &increment_counter;
		(option_table[1]).to_be_modified= &suppress_pointers;
		(option_table[2]).to_be_modified= &count_number;
		return_code=process_multiple_options(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (increment_counter)
			{
				set_counter = -1;
			}
			else
			{
				set_counter = 0;
			}
			if (suppress_pointers)
			{
				return_code=list_memory(count_number, /*show_pointers*/0,
					set_counter, /*show_structures*/0);
			}
			else
			{
				return_code=list_memory(count_number, /*show_pointers*/1,
					set_counter, /*show_structures*/1);
			}
		} /* parse error, help */
		else
		{
			/* no help */
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_list_memory.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_list_memory */

static int execute_command_open_url(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 30 September 2003

DESCRIPTION :
Executes a OPEN URL command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_open_url);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
				{
					do_help(state->current_token,
						/*help_examples_directory*/(char *)NULL,
						command_data->execute_command,
						command_data->user_interface);
				}
				else
				{
					display_message(INFORMATION_MESSAGE," URL");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_open_url.  Missing url");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_open_url.  Missing command_data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_open_url.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_open_url */

static int execute_command_read(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
Executes a READ command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Open_comfile_data open_comfile_data;
	struct Option_table *option_table;

	ENTER(execute_command_read);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* comfile */
				open_comfile_data.file_name=(char *)NULL;
				open_comfile_data.example_flag=0;
				open_comfile_data.execute_count=1;
				open_comfile_data.examples_directory=command_data->example_directory;
				open_comfile_data.example_symbol=CMGUI_EXAMPLE_DIRECTORY_SYMBOL;
				open_comfile_data.execute_command=command_data->execute_command;
				open_comfile_data.set_command=command_data->set_command;
				open_comfile_data.io_stream_package=command_data->io_stream_package;
				open_comfile_data.file_extension=".com";
#if defined (MOTIF) || (WX_USER_INTERFACE)
				open_comfile_data.comfile_window_manager =
					command_data->comfile_window_manager;
#endif /* defined (MOTIF) || (WX_USER_INTERFACE)*/
/* #if defined (WX_USER_INTERFACE) */
/* 				change_dir(state,NULL,command_data); */
/* #endif  (WX_USER_INTERFACE)*/ 
				open_comfile_data.user_interface=command_data->user_interface;
				Option_table_add_entry(option_table, "comfile", NULL,
					(void *)&open_comfile_data, open_comfile);
				return_code=Option_table_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("read",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_read.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_read.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_read */

static int open_example(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 12 December 2002

DESCRIPTION :
Opens an example.
==============================================================================*/
{
	char *example, *execute_flag, *found_cm, temp_string[100];
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(open_example);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
		   example = (char *)NULL;
			execute_flag = 0;
			option_table = CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table, "example",
				&example, (void *)1, set_name);
			/* execute */
			Option_table_add_entry(option_table, "execute",
				&execute_flag, NULL, set_char_flag);
			/* default */
			Option_table_add_entry(option_table, (void *)NULL,
				&example, NULL, set_name);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (!example)
				{
					display_message(ERROR_MESSAGE,
						"open_example.  You must specify an example name");
					return_code = 0;
				}
			}
			if (return_code)
			{
				/* set the examples directory */
				sprintf(temp_string,"set dir ");
				strcat(temp_string,CMGUI_EXAMPLE_DIRECTORY_SYMBOL);
				strcat(temp_string," ");
				strcat(temp_string,example);
				Execute_command_execute_string(command_data->execute_command,temp_string);
				/* The example_comfile and example_requirements strings are 
					currently set as a sideeffect of "set dir" */
				if (command_data->example_requirements)
				{
					if (found_cm = strstr(command_data->example_requirements, "cm"))
					{
						if ((found_cm[2] == 0) || (found_cm[2] == ':') || 
							(found_cm[2] == ','))
						{
							sprintf(temp_string,"create cm");
							Execute_command_execute_string(command_data->execute_command,
								temp_string);
						}
					}
				}
				sprintf(temp_string,"open comfile ");
				if (command_data->example_comfile)
				{
					strcat(temp_string,command_data->example_comfile);
				}
				else
				{
					strcat(temp_string,"example_");
					strcat(temp_string,example);
				}
				strcat(temp_string,";");
				strcat(temp_string,CMGUI_EXAMPLE_DIRECTORY_SYMBOL);
				if (execute_flag)
				{
					strcat(temp_string," execute");
				}
				return_code=Execute_command_execute_string(command_data->execute_command,
					temp_string);
			}
			if (example)
			{
				DEALLOCATE(example);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"open_example.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_example.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* open_comfile */

static int execute_command_open(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION :
Executes a OPEN command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Open_comfile_data open_comfile_data;
	struct Option_table *option_table;

	ENTER(execute_command_open);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* comfile */
				open_comfile_data.file_name=(char *)NULL;
				open_comfile_data.example_flag=0;
				open_comfile_data.execute_count=0;
				open_comfile_data.examples_directory=command_data->example_directory;
				open_comfile_data.example_symbol=CMGUI_EXAMPLE_DIRECTORY_SYMBOL;
				open_comfile_data.execute_command=command_data->execute_command;
				open_comfile_data.set_command=command_data->set_command;
				open_comfile_data.io_stream_package=command_data->io_stream_package;
				open_comfile_data.file_extension=".com";
#if defined (MOTIF) || (WX_USER_INTERFACE)
				open_comfile_data.comfile_window_manager =
					command_data->comfile_window_manager;
#endif /* defined (MOTIF) || (WX_USER_INTERFACE) */
				open_comfile_data.user_interface=command_data->user_interface;
				Option_table_add_entry(option_table, "comfile", NULL,
					(void *)&open_comfile_data, open_comfile);
				Option_table_add_entry(option_table, "example", NULL,
					command_data_void, open_example);
				Option_table_add_entry(option_table, "url", NULL,
					command_data_void, execute_command_open_url);
				return_code=Option_table_parse(option_table, state);
/* #if defined (WX_USER_INTERFACE)  */
/*  				change_dir(state,NULL,command_data); */
/* #endif (WX_USER_INTERFACE)*/ 
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("open",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_open.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_open.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_open */

static int execute_command_quit(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a QUIT command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Cmiss_command_data *command_data;

	ENTER(execute_command_quit);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (!((current_token=state->current_token)&&
			!(strcmp(PARSER_HELP_STRING,current_token)&&
			strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
		{
			if (command_data=(struct Cmiss_command_data *)command_data_void)
			{
				User_interface_end_application_loop(command_data->user_interface);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_quit.  Invalid command_data");
				return_code=0;
			}
		}
		else
		{
			/* no help */
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_quit.	Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_quit */

static int set_dir(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 May 2003

DESCRIPTION :
Executes a SET DIR command.
==============================================================================*/
{
#if defined (LINK_CMISS)
	char *token;
#endif /* defined (LINK_CMISS) */
	char *comfile_name, *directory_name, *example_directory, example_flag, 
		*example_requirements;
	int file_name_length, return_code;
	struct Cmiss_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{CMGUI_EXAMPLE_DIRECTORY_SYMBOL,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,NULL,
			set_char_flag},
		{NULL,NULL,NULL,set_name}
	};

	ENTER(set_dir);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (
#if defined (LINK_CMISS)
					token =
#endif /* defined (LINK_CMISS) */
					state->current_token)
			{
				directory_name = (char *)NULL;
				example_flag = 0;
				(option_table[0]).to_be_modified = &example_flag;
				(option_table[1]).to_be_modified = &directory_name;
				return_code=process_multiple_options(state,option_table);
				if (return_code)
				{
					if (example_flag)
					{
						if (directory_name)
						{							
							/* Lookup the example path */
							if (example_directory = 
								resolve_example_path(command_data->examples_directory, 
								directory_name, &comfile_name, &example_requirements))
							{
								if (command_data->example_directory)
								{
									DEALLOCATE(command_data->example_directory);
								}
								command_data->example_directory=example_directory;
								if (command_data->example_comfile)
								{
									DEALLOCATE(command_data->example_comfile);
								}
								if (comfile_name)
								{
									command_data->example_comfile = comfile_name;
								}
								else
								{
									command_data->example_comfile = (char *)NULL;
								}
								if (command_data->example_requirements)
								{
									DEALLOCATE(command_data->example_requirements);
								}
								if (example_requirements)
								{
									command_data->example_requirements = 
										example_requirements;
								}
								else
								{
									 command_data->example_requirements = (char *)NULL;
								}
#if defined (PERL_INTERPRETER)
								/* Set the interpreter variable */
								interpreter_set_string(command_data->interpreter, "example",
									example_directory, &return_code);
#endif /* defined (PERL_INTERPRETER) */
								
#if defined (LINK_CMISS)
								if (CMISS)
								{
									/* send command to the back end */
									/* have to reset the token position to get it to
										export the command */
									state->current_token = token;
									return_code=execute_command_cm(state,(void *)NULL,
										command_data_void);
								}
#endif /* defined (LINK_CMISS) */
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_dir.  Unable to resolve example path.");
								return_code = 0;
							}
						}
						else
						{
							file_name_length = 1;
							if (command_data->examples_directory)
							{
								file_name_length += strlen(command_data->examples_directory);
							}
							if (ALLOCATE(example_directory,char,file_name_length))
							{
								*example_directory='\0';
								if (command_data->examples_directory)
								{
									strcat(example_directory,command_data->examples_directory);
								}
								DEALLOCATE(command_data->example_directory);
								command_data->example_directory=example_directory;
#if defined (LINK_CMISS)
								if (CMISS)
								{
									/* send command to the back end */
									/* have to reset the token position to get it to
										export the command */
									state->current_token = token;
									return_code=execute_command_cm(state,(void *)NULL,
										command_data_void);
								}
#endif /* defined (LINK_CMISS) */
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_dir.  Insufficient memory");
							}
						}
					} 
					else
					{
						if(chdir(directory_name))
						{
							display_message(ERROR_MESSAGE,
								"set_dir.  Unable to change to directory %s",
								directory_name);
							
						}
						return_code=execute_command_cm(state,(void *)NULL,
							command_data_void);
					}
				}
				if (directory_name)
				{
					DEALLOCATE(directory_name);
				}
			}
			else
			{
				set_command_prompt("set dir",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_set_dir.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_set_dir.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_dir */

static int execute_command_set(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Executes a SET command.
==============================================================================*/
{
	int return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_set);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (command_data=(struct Cmiss_command_data *)command_data_void)
		{
			if (state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* directory */
				Option_table_add_entry(option_table, "directory", NULL,
					command_data_void, set_dir);
				/* default */
				Option_table_add_entry(option_table, "", NULL, command_data_void, 
					execute_command_cm);
				return_code=Option_table_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("set",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_set.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_set.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_set */

static int execute_command_system(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 19 February 1998

DESCRIPTION :
Executes a SET DIR #CMGUI_EXAMPLE_DIRECTORY_SYMBOL command.
???RC Obsolete?
==============================================================================*/
{
	char *command,*current_token,*system_command;
	int return_code;

	ENTER(execute_command_system);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	return_code=0;
	/* check argument */
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				command=strstr(state->command_string,current_token);
				if (ALLOCATE(system_command,char,strlen(command)+1))
				{
					strcpy(system_command,command);
					parse_variable(&system_command);
					system(system_command);
					DEALLOCATE(system_command);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_system.  Insufficient memory");
					return_code=0;
				}
			}
			else
			{
				/* no help */
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing graphics object name");
			display_parse_state_location(state);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_system.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_system */

/*
Global functions
----------------
*/
#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
void execute_command(char *command_string,void *command_data_void, int *quit,
  int *error)
/*******************************************************************************
LAST MODIFIED : 17 July 2002

DESCRIPTION:
==============================================================================*/
{
	char **token;
	int i,return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Parse_state *state;

	ENTER(execute_command);
	USE_PARAMETER(quit);
	if (command_data=(struct Cmiss_command_data *)command_data_void)
	{
		if (state=create_Parse_state(command_string))
			/*???DB.  create_Parse_state has to be extended */
		{
			i=state->number_of_tokens;
			/* check for comment */
			if (i>0)
			{
#if defined (OLD_CODE)
				/* add command to command history */			  
				display_message(INFORMATION_MESSAGE,
					"%s\n", command_string);
#endif /* defined (OLD_CODE) */
				/* check for a "<" as one of the of the tokens */
					/*???DB.  Include for backward compatability.  Remove ? */
				token=state->tokens;
				while ((i>0)&&strcmp(*token,"<"))
				{
					i--;
					token++;
				}
				if (i>0)
				{
					/* return to tree root */
					return_code=set_command_prompt("",command_data);
				}
				else
				{
					option_table = CREATE(Option_table)();
#if defined (SELECT_DESCRIPTORS)
					/* attach */
					Option_table_add_entry(option_table, "attach", NULL, command_data_void,
						execute_command_attach);
#endif /* !defined (SELECT_DESCRIPTORS) */
#if defined (CELL)
					/* cell */
					Option_table_add_entry(option_table, "cell", NULL, command_data_void,
						execute_command_cell);
#endif /* defined (CELL) */
#if defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE)
					/* command_window */
					Option_table_add_entry(option_table, "command_window", NULL, command_data->command_window,
						modify_Command_window);
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
					/* create */
					Option_table_add_entry(option_table, "create", NULL, command_data_void,
						execute_command_create);
#if defined (SELECT_DESCRIPTORS)
					/* detach */
					Option_table_add_entry(option_table, "detach", NULL, command_data_void,
						execute_command_detach);
#endif /* !defined (SELECT_DESCRIPTORS) */
					/* fem */
					Option_table_add_entry(option_table, "fem", NULL, command_data_void,
						execute_command_cm);
					/* gen */
					Option_table_add_entry(option_table, "gen", NULL, command_data_void,
						execute_command_cm);
					/* gfx */
					Option_table_add_entry(option_table, "gfx", NULL, command_data_void,
						execute_command_gfx);
#if !defined (NO_HELP)
					/* help */
					Option_table_add_entry(option_table, "help", NULL, command_data_void,
						execute_command_help);
#endif /* !defined (NO_HELP) */
					/* open */
					Option_table_add_entry(option_table, "open", NULL, command_data_void,
						execute_command_open);
					/* quit */
					Option_table_add_entry(option_table, "quit", NULL, command_data_void,
						execute_command_quit);
					/* list_memory */
					Option_table_add_entry(option_table, "list_memory", NULL, NULL,
						execute_command_list_memory);
					/* read */
					Option_table_add_entry(option_table, "read", NULL, command_data_void,
						execute_command_read);
					/* set */
					Option_table_add_entry(option_table, "set", NULL, command_data_void,
						execute_command_set);
					/* system */
					Option_table_add_entry(option_table, "system", NULL, command_data_void,
						execute_command_system);
#if defined (UNEMAP)
					/* unemap */
					Option_table_add_entry(option_table, "unemap", NULL,
						(void *)command_data->unemap_command_data,
						execute_command_unemap);
#endif /* defined (UNEMAP) */
					/* default */
					Option_table_add_entry(option_table, "", NULL, command_data_void,
						execute_command_cm);
					return_code=Option_table_parse(option_table, state);
					DESTROY(Option_table)(&option_table);
				}
			}
			destroy_Parse_state(&state);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmiss_execute_command.  Could not create parse state");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_execute_command.  Missing command_data");
		return_code=0;
	}

	*error = return_code;

	LEAVE;

} /* execute_command */

int cmiss_execute_command(char *command_string,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 August 2000

DESCRIPTION:
Takes a <command_string>, processes this through the F90 interpreter
and then executes the returned strings
==============================================================================*/
{
	int quit,return_code;
	struct Cmiss_command_data *command_data;

	ENTER(cmiss_execute_command);
	command_data = (struct Cmiss_command_data *)NULL;
	if (command_data=(struct Cmiss_command_data *)command_data_void)
	{
#if defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
		if (command_data->command_window)
		{
			add_to_command_list(command_string,command_data->command_window);
		}
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
		quit = 0;

		interpret_command(command_data->interpreter, command_string, (void *)command_data, 
		  &quit, &execute_command, &return_code);

#if defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE)
		if (command_data->command_window)
		{
			reset_command_box(command_data->command_window);
		}
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */

		if (quit)
		{
			Event_dispatcher_end_main_loop(command_data->event_dispatcher);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_execute_command.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_execute_command */
#else /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */
int cmiss_execute_command(char *command_string,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 July 2002

DESCRIPTION:
Execute a <command_string>. If there is a command
==============================================================================*/
{
	char **token;
	int i,return_code;
	struct Cmiss_command_data *command_data;
	struct Option_table *option_table;
	struct Parse_state *state;

	ENTER(cmiss_execute_command);
	if (command_data=(struct Cmiss_command_data *)command_data_void)
	{
		if (state=create_Parse_state(command_string))
			/*???DB.  create_Parse_state has to be extended */
		{
			i=state->number_of_tokens;
			/* check for comment */
			if (i>0)
			{
				/* add command to command history */
				/*???RC put out processed tokens instead? */
#if defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
				if (command_data->command_window)
				{
					add_to_command_list(command_string,command_data->command_window);
				}
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
				/* check for a "<" as one of the of the tokens */
					/*???DB.  Include for backward compatability.  Remove ? */
				token=state->tokens;
				while ((i>0)&&strcmp(*token,"<"))
				{
					i--;
					token++;
				}
				if (i>0)
				{
					/* return to tree root */
					return_code=set_command_prompt("", command_data);
				}
				else
				{
					option_table = CREATE(Option_table)();
#if defined (SELECT_DESCRIPTORS)
					/* attach */
					Option_table_add_entry(option_table, "attach", NULL, command_data_void,
						execute_command_attach);
#endif /* !defined (SELECT_DESCRIPTORS) */
#if defined (CELL)
					/* cell */
					Option_table_add_entry(option_table, "cell", NULL, command_data_void,
						execute_command_cell);
#endif /* defined (CELL) */
#if defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE)
					/* command_window */
					Option_table_add_entry(option_table, "command_window", NULL, command_data->command_window,
						modify_Command_window);
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
					/* create */
					Option_table_add_entry(option_table, "create", NULL, command_data_void,
						execute_command_create);
#if defined (SELECT_DESCRIPTORS)
					/* detach */
					Option_table_add_entry(option_table, "detach", NULL, command_data_void,
						execute_command_detach);
#endif /* !defined (SELECT_DESCRIPTORS) */
					/* fem */
					Option_table_add_entry(option_table, "fem", NULL, command_data_void,
						execute_command_cm);
					/* gen */
					Option_table_add_entry(option_table, "gem", NULL, command_data_void,
						execute_command_cm);
					/* gfx */
					Option_table_add_entry(option_table, "gfx", NULL, command_data_void,
						execute_command_gfx);
#if !defined (NO_HELP)
					/* help */
					Option_table_add_entry(option_table, "help", NULL, command_data_void,
						execute_command_help);
#endif /* !defined (NO_HELP) */
					/* open */
 					Option_table_add_entry(option_table, "open", NULL, command_data_void,
						execute_command_open);
					/* quit */
					Option_table_add_entry(option_table, "quit", NULL, command_data_void,
						execute_command_quit);
					/* list_memory */
					Option_table_add_entry(option_table, "list_memory", NULL, NULL,
						execute_command_list_memory);
					/* read */
					Option_table_add_entry(option_table, "read", NULL, command_data_void,
						execute_command_read);
					/* set */
					Option_table_add_entry(option_table, "set", NULL, command_data_void,
						execute_command_set);
					/* system */
					Option_table_add_entry(option_table, "system", NULL, command_data_void,
						execute_command_system);
#if defined (UNEMAP)
					/* unemap */
					Option_table_add_entry(option_table, "unemap", NULL,
						(void *)command_data->unemap_command_data,
						execute_command_unemap);
#endif /* defined (UNEMAP) */
					/* default */
					Option_table_add_entry(option_table, "", NULL, command_data_void,
						execute_command_cm);
					return_code=Option_table_parse(option_table, state);
					DESTROY(Option_table)(&option_table);
				}
			}
#if defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE)
			if (command_data->command_window)
			{
				reset_command_box(command_data->command_window);
			}
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
			destroy_Parse_state(&state);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmiss_execute_command.  Could not create parse state");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_execute_command.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_execute_command */
#endif  /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */

int cmiss_set_command(char *command_string,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 May 2003

DESCRIPTION:
Sets the <command_string> in the command box of the CMISS command_window, ready
for editing and entering. If there is no command_window, does nothing.
==============================================================================*/
{
	int return_code;
#if defined (MOTIF)
	struct Cmiss_command_data *command_data;
#endif /* defined (MOTIF) */

	ENTER(cmiss_set_command);
	if (command_string
#if defined (MOTIF)
		&& (command_data=(struct Cmiss_command_data *)command_data_void)
#endif /* defined (MOTIF) */
			)
	{
#if defined (MOTIF)
		if (command_data->command_window)
		{
			return_code=Command_window_set_command_string(
				command_data->command_window,command_string);
		}
#else /* defined (MOTIF) */
		USE_PARAMETER(command_data_void);
#endif /* defined (MOTIF) */
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"cmiss_set_command.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_set_command */

#if defined(USE_CMGUI_COMMAND_WINDOW)
static int display_error_message(char *message,void *command_window_void)
/*******************************************************************************
LAST MODIFIED : 25 July 1999

DESCRIPTION :
Display a cmgui error message.
==============================================================================*/
{
	int return_code;

	ENTER(display_error_message);
	if (command_window_void)
	{
		write_command_window("ERROR: ",
			(struct Command_window *)command_window_void);
		return_code=write_command_window(message,
			(struct Command_window *)command_window_void);
		write_command_window("\n",(struct Command_window *)command_window_void);
	}
	else
	{
		printf("ERROR: %s\n",message);
	}
	LEAVE;

	return (return_code);
} /* display_error_message */
#endif /* defined(USE_CMGUI_COMMAND_WINDOW) */

#if defined(USE_CMGUI_COMMAND_WINDOW)
static int display_information_message(char *message,void *command_window_void)
/*******************************************************************************
LAST MODIFIED : 25 July 1999

DESCRIPTION :
Display a cmgui information message.
==============================================================================*/
{
	int return_code;

	ENTER(display_error_message);
	if (command_window_void)
	{
		return_code=write_command_window(message,
			(struct Command_window *)command_window_void);
	}
	else
	{
		printf(message);
	}
	LEAVE;

	return (return_code);
} /* display_information_message */
#endif /* defined(USE_CMGUI_COMMAND_WINDOW) */

#if defined(USE_CMGUI_COMMAND_WINDOW)
static int display_warning_message(char *message,void *command_window_void)
/*******************************************************************************
LAST MODIFIED : 25 July 1999

DESCRIPTION :
Display a cmgui warning message.
???DB.  write_output is for the command_window - needs a better name.
==============================================================================*/
{
	int return_code;

	ENTER(display_warning_message);
	if (command_window_void)
	{
		write_command_window("WARNING: ",
			(struct Command_window *)command_window_void);
		return_code=write_command_window(message,
			(struct Command_window *)command_window_void);
		write_command_window("\n",(struct Command_window *)command_window_void);
	}
	else
	{
		printf("WARNING: %s\n",message);
	}
	LEAVE;

	return (return_code);
} /* display_warning_message */
#endif /* defined(USE_CMGUI_COMMAND_WINDOW) */

static int cmgui_execute_comfile(char *comfile_name,char *example_id,
	char *examples_directory,char *example_symbol,char **example_comfile_name,
	struct Execute_command *execute_command)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Executes the comfile specified on the command line.
==============================================================================*/
{
	int return_code;

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

struct Cmgui_command_line_options
/*******************************************************************************
LAST MODIFIED : 19 May 2006

DESCRIPTION :
Command line options to be parsed by read_cmgui_command_line_options.
==============================================================================*/
{
	char batch_mode_flag;
	char cm_start_flag;
	char *cm_epath_directory_name;
	char *cm_parameters_file_name;
	char command_list_flag;
	char console_mode_flag;
	char *epath_directory_name;
	char *example_file_name;
	char *execute_string;
	char write_help_flag;
	char *id_name;
	char mycm_start_flag;
	char no_display_flag;
	char server_mode_flag;
	int random_number_seed;
	int visual_id_number;
	/* default option; no token */
	char *command_file_name;
};

int set_string_no_command_line_option(struct Parse_state *state,
	void *string_address_void, void *string_description_void)
/*******************************************************************************
LAST MODIFIED : 6 August 2002

DESCRIPTION :
Calls set_string unless the first character of the current token is a hyphen.
Used to avoid parsing possible command line switches.
==============================================================================*/
{
	char *current_token;
	int return_code;

	ENTER(set_string_no_command_line_option);
	if (state)
	{
		if (current_token = state->current_token)
		{
			if ('-' != current_token[0])
			{
				return_code = set_string(state, string_address_void,
					string_description_void);
			}
			else
			{
				display_message(ERROR_MESSAGE, "Invalid command line option \"%s\"",
					current_token);
				display_parse_state_location(state);
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing string");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_string_no_command_line_option.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_string_no_command_line_option */

int ignore_entry_and_next_token(struct Parse_state *state,
	void *dummy_void, void *entry_description_void)
/*******************************************************************************
LAST MODIFIED : 8 August 2002

DESCRIPTION :
Used to consume and write help for command line parameters handled outside
of this parse state routine.
==============================================================================*/
{
	char *current_token;
	int return_code;

	ENTER(ignore_entry_and_next_token);
	USE_PARAMETER(dummy_void);
	if (state)
	{
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				return_code = shift_Parse_state(state,1);
			}
			else
			{
				display_message(INFORMATION_MESSAGE, (char *)entry_description_void);
				return_code = 1;
			}

		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing string");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ignore_entry_and_next_token.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* ignore_entry_and_next_token */

static int read_cmgui_command_line_options(struct Parse_state *state,
	void *dummy_to_be_modified, void *cmgui_command_line_options_void)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

DESCRIPTION :
Parses command line options from <state>.
==============================================================================*/
{
	int return_code;
	struct Cmgui_command_line_options *command_line_options;
	struct Option_table *option_table;

	ENTER(read_cmgui_command_line_options);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_line_options =
		(struct Cmgui_command_line_options *)cmgui_command_line_options_void))
	{
		option_table = CREATE(Option_table)();
#if defined (MOTIF)
		/* -background, handled by X11 */
		Option_table_add_entry(option_table, "-background", NULL,
			(void *)" X11_COLOUR_NAME", ignore_entry_and_next_token);
#endif /* defined (MOTIF) */
		/* -batch */
		Option_table_add_entry(option_table, "-batch",
			&(command_line_options->batch_mode_flag), NULL, set_char_flag);
		/* -cm */
		Option_table_add_entry(option_table, "-cm",
			&(command_line_options->cm_start_flag), NULL, set_char_flag);
		/* -cm_epath */
		Option_table_add_entry(option_table, "-cm_epath",
			&(command_line_options->cm_epath_directory_name),
			(void *)" PATH_TO_EXAMPLES_DIRECTORY", set_string);
		/* -cm_parameters */
		Option_table_add_entry(option_table, "-cm_parameters",
			&(command_line_options->cm_parameters_file_name),
			(void *)" PARAMETER_FILE_NAME", set_string);
		/* -command_list */
		Option_table_add_entry(option_table, "-command_list",
			&(command_line_options->command_list_flag), NULL, set_char_flag);
		/* -console */
		Option_table_add_entry(option_table, "-console",
			&(command_line_options->console_mode_flag), NULL, set_char_flag);
#if defined (MOTIF) || defined (__WXMOTIF__) || defined (__WXX11__)
		/* -display, handled by X11 */
		Option_table_add_entry(option_table, "-display", NULL,
			(void *)" X11_DISPLAY_NUMBER", ignore_entry_and_next_token);
#endif /* defined (MOTIF)  */
#if defined (GTK_USER_INTERFACE) || defined (__WXGTK__)
		/* --display, support the gtk convention for this tool */
		Option_table_add_entry(option_table, "--display", NULL,
			(void *)" X11_DISPLAY_NUMBER", ignore_entry_and_next_token);
#endif /* defined (GTK_USER_INTERFACE) || defined (__WXGTK__) */
		/* -epath */
		Option_table_add_entry(option_table, "-epath",
			&(command_line_options->epath_directory_name),
			(void *)" PATH_TO_EXAMPLES_DIRECTORY", set_string);
		/* -example */
		Option_table_add_entry(option_table, "-example",
			&(command_line_options->example_file_name),
			(void *)" EXAMPLE_ID", set_string);
		/* -execute */
		Option_table_add_entry(option_table, "-execute",
			&(command_line_options->execute_string),
			(void *)" EXECUTE_STRING", set_string);
#if defined (MOTIF)
		/* -foreground, handled by X11 */
		Option_table_add_entry(option_table, "-foreground", NULL,
			(void *)" X11_COLOUR_NAME", ignore_entry_and_next_token);
#endif /* defined (MOTIF) */
		/* -help */
		Option_table_add_entry(option_table, "-help",
			&(command_line_options->write_help_flag), NULL, set_char_flag);
		/* -id */
		Option_table_add_entry(option_table, "-id",
			&(command_line_options->id_name), (void *)" ID", set_string);
		/* -mycm */
		Option_table_add_entry(option_table, "-mycm",
			&(command_line_options->mycm_start_flag), NULL, set_char_flag);
		/* -no_display */
		Option_table_add_entry(option_table, "-no_display",
			&(command_line_options->no_display_flag), NULL, set_char_flag);
		/* -random */
		Option_table_add_entry(option_table, "-random",
			&(command_line_options->random_number_seed),
			(void *)" NUMBER_SEED", set_int_with_description);
		/* -server */
		Option_table_add_entry(option_table, "-server",
			&(command_line_options->server_mode_flag), NULL, set_char_flag);
		/* -visual */
		Option_table_add_entry(option_table, "-visual",
			&(command_line_options->visual_id_number),
			(void *)" NUMBER", set_int_with_description);
		/* [default option == command_file_name] */
		Option_table_add_entry(option_table, (char *)NULL,
			&(command_line_options->command_file_name),
			(void *)"COMMAND_FILE_NAME", set_string_no_command_line_option);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_cmgui_command_line_options.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* read_cmgui_command_line_options */

#if !defined (WIN32_USER_INTERFACE)
struct Cmiss_command_data *CREATE(Cmiss_command_data)(int argc,char *argv[], 
	char *version_string)
#else /* !defined (WIN32_USER_INTERFACE) */
struct Cmiss_command_data *CREATE(Cmiss_command_data)(int argc,char *argv[], 
	char *version_string, HINSTANCE current_instance,
	HINSTANCE previous_instance, LPSTR command_line,int initial_main_window_state)
#endif /* !defined (WIN32_USER_INTERFACE) */
/*******************************************************************************
LAST MODIFIED : 3 April 2003

DESCRIPTION :
Initialise all the subcomponents of cmgui and create the Cmiss_command_data
==============================================================================*/
{
	char *cm_examples_directory,*cm_parameters_file_name,*comfile_name,
		*example_id,*examples_directory,*examples_environment,*execute_string,
		*version_command_id,
		version_id_string[100],*version_ptr,version_temp[20];
	float default_light_direction[3]={0.0,-0.5,-1.0};
	int i, number_of_startup_materials, return_code;
	int batch_mode, console_mode, command_list, no_display, non_random,
		server_mode, start_cm, start_mycm, visual_id, write_help;
#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
	int status;
#endif /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */
#if defined (MOTIF)
	Display *display;
#define XmNbackgroundColour "backgroundColour"
#define XmCBackgroundColour "BackgroundColour"
#define XmNforegroundColour "foregroundColour"
#define XmCForegroundColour "ForegroundColour"
#define XmNexamplesDirectory "examplesDirectory"
#define XmCExamplesDirectory "ExamplesDirectory"
#define XmNstartupComfile "startupComfile"
#define XmCStartupComfile "StartupComfile"
#define XmNhelpDirectory "helpDirectory"
#define XmCHelpDirectory "HelpDirectory"
#define XmNhelpUrl "helpUrl"
#define XmCHelpUrl "HelpUrl"
	static XtResource resources[]=
	{
		{
			XmNbackgroundColour,
			XmCBackgroundColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(User_settings,background_colour),
			XmRString,
			"black"
		},
		{
			XmNforegroundColour,
			XmCForegroundColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(User_settings,foreground_colour),
			XmRString,
			"white"
		},
		{
			XmNexamplesDirectory,
			XmCExamplesDirectory,
			XmRString,
			sizeof(char *),
			XtOffsetOf(User_settings,examples_directory),
			XmRString,
			""
		},
		{
			XmNstartupComfile,
			XmCStartupComfile,
			XmRString,
			sizeof(char *),
			XtOffsetOf(User_settings,startup_comfile),
			XmRImmediate,
			(XtPointer)0
		},
		{
			XmNhelpDirectory,
			XmCHelpDirectory,
			XmRString,
			sizeof(char *),
			XtOffsetOf(User_settings,help_directory),
			XmRString,
			""
		},
		{
			XmNhelpUrl,
			XmCHelpUrl,
			XmRString,
			sizeof(char *),
			XtOffsetOf(User_settings,help_url),
			XmRString,
			"http://www.bioeng.auckland.ac.nz/cmiss/help/user_help.php"
		},
	};
#endif /* defined (MOTIF) */
#if defined (MOTIF)
/*???DB.  Need a setup routine for file I/O ? */
/*???DB.  Put in open_user_interface and always have ? */
	static MrmRegisterArg callbacks[]=
	{
		{"open_file_and_read",(XtPointer)open_file_and_read},
		{"open_file_and_write",(XtPointer)open_file_and_write}
	};
#endif /* defined (MOTIF) */
	struct Cmgui_command_line_options command_line_options;
	struct Cmiss_command_data *command_data;
	struct Colour ambient_colour, colour, default_colour;
	struct Computed_field *computed_field;
#if defined(USE_CMGUI_COMMAND_WINDOW)
	struct Command_window *command_window;
#endif /* defined(USE_CMGUI_COMMAND_WINDOW) */
	struct Coordinate_system rect_coord_system;
	struct FE_region *data_root_fe_region, *root_fe_region;
	struct Graphical_material *material;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Option_table *option_table;
	struct Parse_state *state;
	User_settings user_settings;

#if defined (MOTIF)
	XColor rgb;
#endif /* defined (MOTIF) */

#if defined (MOTIF)
	ENTER(main);
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
	ENTER(WinMain);
#endif /* defined (WIN32_USER_INTERFACE) */
	return_code = 1;

	if (ALLOCATE(command_data, struct Cmiss_command_data, 1))
	{
		/* initialize application specific global variables */
		command_data->execute_command = CREATE(Execute_command)();;
		command_data->set_command = CREATE(Execute_command)();
		command_data->event_dispatcher = (struct Event_dispatcher *)NULL;
		command_data->user_interface= (struct User_interface *)NULL;
		command_data->emoter_slider_dialog=(struct Emoter_dialog *)NULL;
#if defined (MOTIF)
		command_data->curve_editor_dialog=(Widget)NULL;
		command_data->data_grabber_dialog=(Widget)NULL;
		command_data->sync_2d_3d_dialog=(Widget)NULL;
		command_data->grid_field_calculator_dialog=(Widget)NULL;
		command_data->input_module_dialog=(Widget)NULL;
		command_data->data_viewer=(struct Node_viewer *)NULL;
		command_data->node_viewer=(struct Node_viewer *)NULL;
		command_data->element_point_viewer=(struct Element_point_viewer *)NULL;
		command_data->prompt_window=(struct Prompt_window *)NULL;
		command_data->projection_window=(struct Projection_window *)NULL;
		command_data->material_editor_dialog = (struct Material_editor_dialog *)NULL;
		/*???RC.  Temporary - should allow more than one */
		command_data->time_editor_dialog = (struct Time_editor_dialog *)NULL;
		/*???RC.  Temporary - should allow more than one */
#endif /* defined (MOTIF) */
#if defined (WX_USER_INTERFACE)
		command_data->data_viewer=(struct Node_viewer *)NULL;
		command_data->node_viewer=(struct Node_viewer *)NULL;
		command_data->element_point_viewer=(struct Element_point_viewer *)NULL;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (MOTIF) || defined (WX_USER_INTERFACE)
		command_data->scene_editor = (struct Scene_editor *)NULL;
		command_data->spectrum_editor_dialog = (struct Spectrum_editor_dialog *)NULL;
#endif /* defined (MOTIF) || defined (WX_USER_INTERFACE) */
		command_data->command_console = (struct Console *)NULL;
#if defined (UNEMAP)
		command_data->unemap_command_data=(struct Unemap_command_data *)NULL;
#endif /* defined (UNEMAP) */
#if defined (CELL)
		command_data->cell_interface = (struct Cell_interface *)NULL;
#endif /* defined (CELL) */
		command_data->example_directory=(char *)NULL;

#if defined (MOTIF) || (WX_USER_INTERFACE)
		command_data->comfile_window_manager=(struct MANAGER(Comfile_window) *)NULL;
#endif /* defined (MOTIF)  || (WX_USER_INTERFACE)*/
		command_data->default_light=(struct Light *)NULL;
		command_data->light_manager=(struct MANAGER(Light) *)NULL;
		command_data->default_light_model=(struct Light_model *)NULL;
		command_data->light_model_manager=(struct MANAGER(Light_model) *)NULL;
		command_data->environment_map_manager=(struct MANAGER(Environment_map) *)NULL;
		command_data->texture_manager=(struct MANAGER(Texture) *)NULL;
		command_data->volume_texture_manager=(struct MANAGER(VT_volume_texture) *)NULL;
		command_data->default_spectrum=(struct Spectrum *)NULL;
		command_data->spectrum_manager=(struct MANAGER(Spectrum) *)NULL;
		command_data->graphics_buffer_package=(struct Graphics_buffer_package *)NULL;
		command_data->scene_viewer_package=(struct Cmiss_scene_viewer_package *)NULL;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		command_data->graphics_window_manager=(struct MANAGER(Graphics_window) *)NULL;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
		command_data->root_region = (struct Cmiss_region *)NULL;
		command_data->data_root_region = (struct Cmiss_region *)NULL;
		command_data->curve_manager=(struct MANAGER(Curve) *)NULL;
		command_data->basis_manager=(struct MANAGER(FE_basis) *)NULL;
		command_data->streampoint_list=(struct Streampoint *)NULL;
#if defined (SELECT_DESCRIPTORS)
		command_data->device_list=(struct LIST(Io_device) *)NULL;
#endif /* defined (SELECT_DESCRIPTORS) */
		command_data->graphics_object_list=(struct LIST(GT_object) *)NULL;
		command_data->glyph_list=(struct LIST(GT_object) *)NULL;	
		command_data->any_object_selection=(struct Any_object_selection *)NULL;
		command_data->element_point_ranges_selection=(struct Element_point_ranges_selection *)NULL;
		command_data->element_selection=(struct FE_element_selection *)NULL;
		command_data->data_selection=(struct FE_node_selection *)NULL;
		command_data->node_selection=(struct FE_node_selection *)NULL;
		command_data->interactive_tool_manager=(struct MANAGER(Interactive_tool) *)NULL;
		command_data->io_stream_package = (struct IO_stream_package *)NULL;
		command_data->computed_field_package=(struct Computed_field_package *)NULL;
		command_data->default_scene=(struct Scene *)NULL;
		command_data->scene_manager=(struct MANAGER(Scene) *)NULL;
#if defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
		command_data->command_window=(struct Command_window *)NULL;
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
#if defined (SGI_MOVIE_FILE) && defined (MOTIF)
		command_data->movie_graphics_manager=(struct MANAGER(Movie_graphics) *)NULL;
#endif /* defined (SGI_MOVIE_FILE) && defined (MOTIF) */
		command_data->transform_tool=(struct Interactive_tool *)NULL;
		command_data->node_tool=(struct Node_tool *)NULL;
		command_data->element_tool=(struct Element_tool *)NULL;
		command_data->data_tool=(struct Node_tool *)NULL;
		command_data->element_point_tool=(struct Element_point_tool *)NULL;
#if defined (MOTIF)
		command_data->select_tool=(struct Select_tool *)NULL;

		command_data->element_creator=(struct Element_creator *)NULL;
#endif /* defined (MOTIF) */
		command_data->examples_directory=(char *)NULL;
		command_data->example_comfile=(char *)NULL;
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
		command_data->scene_object_transformation_data = 
			 (struct Scene_object_transformation_data*)NULL;
#if defined (PERL_INTERPRETER)
		command_data->interpreter = (struct Interpreter *)NULL;
#endif /* defined (PERL_INTERPRETER) */

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
	
		if (state = create_Parse_state_from_tokens(argc, argv))
		{
			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table, argv[0], NULL,
				(void *)&command_line_options, read_cmgui_command_line_options);
			if (!Option_table_parse(option_table, state))
			{
				write_help = 1;
				return_code = 0;
			}
			DESTROY(Option_table)(&option_table);
			destroy_Parse_state(&state);
		}
		else
		{
			return_code = 0;
		}
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
		if (write_help)
		{
			char *double_question_mark = "??";

			/* write question mark help for command line options */
			state = create_Parse_state_from_tokens(1, &double_question_mark);
			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table, argv[0], NULL,
				(void *)&command_line_options, read_cmgui_command_line_options);
			Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			destroy_Parse_state(&state);
		}

		command_data->io_stream_package = CREATE(IO_stream_package)();

#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
		/* SAB I want to do this before CREATEing the User_interface
			as X modifies the argc, argv removing the options it understands
			however I want a full copy for the interpreter so that we can use
			-display for example for both */
		create_interpreter(argc, argv, comfile_name, &command_data->interpreter, &status);

		if (!status)
		{
			return_code=0;			
		}

		interpreter_set_display_message_function(command_data->interpreter, display_message, &status);

		/* SAB Set a useful default for the interpreter variable, need to 
			specify full name as this function does not run embedded by
			a package directive */
		interpreter_set_string(command_data->interpreter, "cmiss::example", ".", &status);

		/* SAB Set the cmgui command data into the interpreter.  The Cmiss package
			is then able to export this when it is called from inside cmgui or
			when called directly from perl to load the appropriate libraries to
			create a cmgui externally. */
	 	interpreter_set_pointer(command_data->interpreter, "Cmiss::Cmgui_command_data",
			"Cmiss::Cmgui_command_data", command_data, &status);

#endif /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */

		if ((!command_list) && (!write_help))
		{
			if (command_data->event_dispatcher = CREATE(Event_dispatcher)())
			{
				if (!no_display)
				{
#if !defined (WIN32_USER_INTERFACE)
					if (!(command_data->user_interface = CREATE(User_interface)
							 (&argc, argv, command_data->event_dispatcher, "Cmgui",
								 "cmgui")))
					{
						display_message(ERROR_MESSAGE,"Could not create User interface");
						return_code=0;
					}
#else /* !defined (WIN32_USER_INTERFACE) */
					if (!(command_data->user_interface = CREATE(User_interface)
							 (current_instance, previous_instance, command_line,
								 initial_main_window_state, command_data->event_dispatcher)))
					{
						display_message(ERROR_MESSAGE,"Could not create User interface");
						return_code=0;
					}
#endif /* !defined (WIN32_USER_INTERFACE) */
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Could not create Event dispatcher.");
				return_code = 0;
			}
		}

#if defined (MOTIF)
		if (command_data->user_interface)
		{
			/* retrieve application specific constants */
			display = User_interface_get_display(command_data->user_interface);
			XtVaGetApplicationResources(User_interface_get_application_shell(
			   command_data->user_interface),&user_settings,resources,
			   XtNumber(resources),NULL);
			/*???DB.  User settings should be divided among tools */
			/* retrieve the background rgb settings */
			rgb.pixel=user_settings.background_colour;
			XQueryColor(display,DefaultColormap(display,DefaultScreen(display)),&rgb);
			/*???DB.  Get rid of 65535 ? */
			command_data->background_colour.red=(float)(rgb.red)/(float)65535;
			command_data->background_colour.green=(float)(rgb.green)/(float)65535;
			command_data->background_colour.blue=(float)(rgb.blue)/(float)65535;
			/* retrieve the foreground rgb settings */
			rgb.pixel=user_settings.foreground_colour;
			XQueryColor(display,DefaultColormap(display,DefaultScreen(display)),&rgb);
			/*???DB.  Get rid of 65535 ? */
			command_data->foreground_colour.red=(float)(rgb.red)/(float)65535;
			command_data->foreground_colour.green=(float)(rgb.green)/(float)65535;
			command_data->foreground_colour.blue=(float)(rgb.blue)/(float)65535;
			if ((command_data->foreground_colour.red ==
					 command_data->background_colour.red) &&
				(command_data->foreground_colour.green ==
					command_data->background_colour.green) &&
				(command_data->foreground_colour.blue ==
					command_data->background_colour.blue))
			{
				command_data->foreground_colour.red =
					1 - command_data->background_colour.red;
				command_data->foreground_colour.green =
					1 - command_data->background_colour.green;
				command_data->foreground_colour.blue =
					1 - command_data->background_colour.blue;
			}
		}
#endif /* defined (MOTIF) */

		/* use command line options in preference to defaults read from XResources */

		if (examples_directory)
		{
			command_data->examples_directory = examples_directory;
		}
		else if (examples_environment = getenv("CMISS_EXAMPLES"))
		{
			command_data->examples_directory = duplicate_string(examples_environment);
		}
		else
		{
			command_data->examples_directory = (char *)NULL;
		}
		command_data->cm_examples_directory = cm_examples_directory;
		command_data->cm_parameters_file_name = cm_parameters_file_name;
		command_data->help_directory = user_settings.help_directory;
		command_data->help_url = user_settings.help_url;

		/* create the managers */

#if defined (MOTIF) || (WX_USER_INTERFACE)
		/* comfile window manager */
		command_data->comfile_window_manager = CREATE(MANAGER(Comfile_window))();
#endif /* defined (MOTIF) || (WX_USER_INTERFACE) */

		/* light manager */
		if (command_data->light_manager=CREATE(MANAGER(Light))())
		{
			if (command_data->default_light=CREATE(Light)("default"))
			{
				set_Light_type(command_data->default_light,INFINITE_LIGHT);
				default_colour.red=1.0;
				default_colour.green=1.0;
				default_colour.blue=1.0;		
				set_Light_colour(command_data->default_light,&default_colour); 
				
				set_Light_direction(command_data->default_light,default_light_direction);
				/*???DB.  Include default as part of manager ? */
				ACCESS(Light)(command_data->default_light);
				if (!ADD_OBJECT_TO_MANAGER(Light)(command_data->default_light,
						 command_data->light_manager))
				{
					DEACCESS(Light)(&(command_data->default_light));
				}
			}
		}
		if (command_data->light_model_manager=CREATE(MANAGER(Light_model))())
		{
			if (command_data->default_light_model=CREATE(Light_model)("default"))
			{
				ambient_colour.red=0.2;
				ambient_colour.green=0.2;
				ambient_colour.blue=0.2;
				Light_model_set_ambient(command_data->default_light_model,&ambient_colour);
				Light_model_set_side_mode(command_data->default_light_model,
					LIGHT_MODEL_TWO_SIDED);
				/*???DB.  Include default as part of manager ? */			
				ACCESS(Light_model)(command_data->default_light_model);
				if (!ADD_OBJECT_TO_MANAGER(Light_model)(
						 command_data->default_light_model,command_data->light_model_manager))
				{
					DEACCESS(Light_model)(&(command_data->default_light_model));
				}			
			}
		}
		/* environment map manager */
		command_data->environment_map_manager=CREATE(MANAGER(Environment_map))();
		/* texture manager */
		command_data->texture_manager=CREATE(MANAGER(Texture))();
		/* volume texture manager */
		command_data->volume_texture_manager=CREATE(MANAGER(VT_volume_texture))();
		/* spectrum manager */
		if (command_data->spectrum_manager=CREATE(MANAGER(Spectrum))())
		{
			if (command_data->default_spectrum=CREATE(Spectrum)("default"))
			{
				Spectrum_set_simple_type(command_data->default_spectrum,
					BLUE_TO_RED_SPECTRUM);
				Spectrum_set_minimum_and_maximum(command_data->default_spectrum,0,1);
				/* ACCESS so can never be destroyed */
				ACCESS(Spectrum)(command_data->default_spectrum);
				if (!ADD_OBJECT_TO_MANAGER(Spectrum)(command_data->default_spectrum,
						 command_data->spectrum_manager))
				{
					DEACCESS(Spectrum)(&(command_data->default_spectrum));
				}
			}
		}
		/* create Material package and CMGUI default materials */
		if (command_data->material_package = ACCESS(Material_package)(CREATE(Material_package)
				(command_data->texture_manager, command_data->spectrum_manager)))
		{
			if (material = Material_package_get_default_material(command_data->material_package))
			{
#if defined (MOTIF)
				Graphical_material_set_ambient(material, &(command_data->foreground_colour));
				Graphical_material_set_diffuse(material, &(command_data->foreground_colour));
#endif /* defined (MOTIF) */
				Graphical_material_set_alpha(material, 1.0);
			}
		}
		command_data->graphics_font_package = CREATE(Graphics_font_package)();
		command_data->default_font = ACCESS(Graphics_font)(
			Graphics_font_package_get_font(command_data->graphics_font_package, "default"));
		number_of_startup_materials = sizeof(startup_materials) /
			sizeof(struct Startup_material_definition);
		for (i = 0; i < number_of_startup_materials; i++)
		{
			if (material = CREATE(Graphical_material)(startup_materials[i].name))
			{
				colour.red   = startup_materials[i].ambient[0];
				colour.green = startup_materials[i].ambient[1];
				colour.blue  = startup_materials[i].ambient[2];
				Graphical_material_set_ambient(material, &colour);
				colour.red   = startup_materials[i].diffuse[0];
				colour.green = startup_materials[i].diffuse[1];
				colour.blue  = startup_materials[i].diffuse[2];
				Graphical_material_set_diffuse(material, &colour);
				colour.red   = startup_materials[i].emission[0];
				colour.green = startup_materials[i].emission[1];
				colour.blue  = startup_materials[i].emission[2];
				Graphical_material_set_emission(material, &colour);
				colour.red   = startup_materials[i].specular[0];
				colour.green = startup_materials[i].specular[1];
				colour.blue  = startup_materials[i].specular[2];
				Graphical_material_set_specular(material, &colour);
				Graphical_material_set_alpha(material, startup_materials[i].alpha);
				Graphical_material_set_shininess(material,
					startup_materials[i].shininess);
				if (!Material_package_manage_material(command_data->material_package,
					material))
				{
					DESTROY(Graphical_material)(&material);
				}
			}
		}
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		if (command_data->user_interface)
		{
			command_data->graphics_buffer_package = CREATE(Graphics_buffer_package)(
				command_data->user_interface);
			Graphics_buffer_package_set_override_visual_id(
					command_data->graphics_buffer_package, visual_id);
		}
		/* graphics window manager.  Note there is no default window. */
		command_data->graphics_window_manager=CREATE(MANAGER(Graphics_window))();
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
		/* FE_element_shape manager */
		/*???DB.  To be done */
		command_data->element_shape_list=CREATE(LIST(FE_element_shape))();

		rect_coord_system.type = RECTANGULAR_CARTESIAN;

		command_data->curve_manager=CREATE(MANAGER(Curve))();

		command_data->basis_manager=CREATE(MANAGER(FE_basis))();

		command_data->root_region = ACCESS(Cmiss_region)(CREATE(Cmiss_region)());
		/* add FE_region to root_region */
		if (root_fe_region = CREATE(FE_region)((struct FE_region *)NULL,
			command_data->basis_manager, command_data->element_shape_list))
		{
			 Cmiss_region_attach_FE_region(command_data->root_region, root_fe_region);
		}
		
		command_data->data_root_region =
			ACCESS(Cmiss_region)(CREATE(Cmiss_region)());
		/* add FE_region to data_root_region but make it use fields and elements
			 from the root_region with the data_hack! */
		if (data_root_fe_region = create_data_hack_FE_region(root_fe_region))
		{
			Cmiss_region_attach_FE_region(command_data->data_root_region,
				data_root_fe_region);
		}

		/* add callbacks so root_region and data_root_region keep their children
			 synchronised */
		Cmiss_region_add_callback(command_data->root_region,
			Cmiss_region_synchronise_children_with_FE_region,
			(void *)(command_data->data_root_region));
		Cmiss_region_add_callback(command_data->data_root_region,
			Cmiss_region_synchronise_children_with_FE_region,
			(void *)(command_data->root_region));

		/* create graphics object list */
		/*???RC.  Eventually want graphics object manager */
		command_data->graphics_object_list=CREATE(LIST(GT_object))();

#if defined (SELECT_DESCRIPTORS)
		/* create device list */
		/*SAB.  Eventually want device manager */
		command_data->device_list=CREATE(LIST(Io_device))();
#endif /* defined (SELECT_DESCRIPTORS) */

		command_data->glyph_list = make_standard_glyphs(command_data->default_font);

		/* global list of selected objects */
		command_data->any_object_selection=CREATE(Any_object_selection)();
		command_data->element_point_ranges_selection=
			CREATE(Element_point_ranges_selection)(/*root_fe_region*/);
		command_data->element_selection =
			CREATE(FE_element_selection)(root_fe_region);
		command_data->node_selection = CREATE(FE_node_selection)(root_fe_region);
		command_data->data_selection =
			 CREATE(FE_node_selection)(data_root_fe_region);	

		/* interactive_tool manager */
		command_data->interactive_tool_manager=CREATE(MANAGER(Interactive_tool))();
		/* computed field manager and default computed fields zero, xi,
			default_coordinate, etc. */
		/*???RC should the default computed fields be established in
		  CREATE(Computed_field_package)? */
		command_data->computed_field_package=CREATE(Computed_field_package)();
		computed_field_manager=Computed_field_package_get_computed_field_manager(
			command_data->computed_field_package);

		/* Add Computed_fields to the Computed_field_package */
		if (command_data->computed_field_package)
		{
			Computed_field_register_types_coordinate(
				command_data->computed_field_package);
			Computed_field_register_types_component_operations(
				command_data->computed_field_package);
			Computed_field_register_types_trigonometry(
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
			Computed_field_register_types_derivatives(
				command_data->computed_field_package);
			Computed_field_register_types_fibres(
				command_data->computed_field_package);
			Computed_field_register_types_logical_operators(
				command_data->computed_field_package);
			if (command_data->root_region)
			{
				Computed_field_register_types_lookup(
					command_data->computed_field_package, 
					command_data->root_region);
			}
			Computed_field_register_types_matrix_operations(
				command_data->computed_field_package);
			if (command_data->root_region)
			{
				Computed_field_register_types_region_operations(
					command_data->computed_field_package, 
					command_data->root_region);
			}
			Computed_field_register_types_vector_operations(
				command_data->computed_field_package);
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
			if (command_data->graphics_window_manager)
			{
				Computed_field_register_type_window_projection(
					command_data->computed_field_package,
					command_data->graphics_window_manager);
			}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
			if (command_data->texture_manager)
			{
				Computed_field_register_type_sample_texture(
					command_data->computed_field_package, 
					command_data->texture_manager);
			}
			if (command_data->root_region)
			{
				Computed_field_register_type_integration(
					command_data->computed_field_package, 
					command_data->root_region);
			}
			if (command_data->root_region)
			{
				Computed_field_register_types_finite_element(
						command_data->computed_field_package, command_data->root_region);
				if (!((computed_field=CREATE(Computed_field)("cmiss_number"))&&
						 Computed_field_set_coordinate_system(computed_field,
							 &rect_coord_system)&&
						 Computed_field_set_type_cmiss_number(computed_field)&&
						 Computed_field_set_read_only(computed_field)&&
						 ADD_OBJECT_TO_MANAGER(Computed_field)(computed_field,
							 computed_field_manager)))
				{
					DESTROY(Computed_field)(&computed_field);
				}
				if (!((computed_field=CREATE(Computed_field)("xi"))&&
						 Computed_field_set_coordinate_system(computed_field,
							 &rect_coord_system)&&
						 Computed_field_set_type_xi_coordinates(computed_field)&&
						 Computed_field_set_read_only(computed_field)&&
						 ADD_OBJECT_TO_MANAGER(Computed_field)(computed_field,
							 computed_field_manager)))
				{
					DESTROY(Computed_field)(&computed_field);
				}
			}
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

#if defined (MOTIF)
		/* now set up the conversion routines */
		/*???DB.  Can this be put elsewhere ? */
		conversion_init();
		/* initialize the coordinate widget manager */
		/*???DB.  Still needs to be turned into a manager */
		coord_widget_init();
#endif /* defined (MOTIF) */
#if defined (SGI_MOVIE_FILE) && defined (MOTIF)
		command_data->movie_graphics_manager = CREATE(MANAGER(Movie_graphics))();
#endif /* defined (SGI_MOVIE_FILE) && defined (MOTIF) */

		/* scene manager */
		/*???RC & SAB.   LOTS of managers need to be created before this 
		  and the User_interface too */
		if (command_data->scene_manager=CREATE(MANAGER(Scene))())
		{
			if (command_data->default_scene=CREATE(Scene)("default"))
			{
				Scene_enable_graphics(command_data->default_scene,command_data->glyph_list,
					Material_package_get_material_manager(command_data->material_package),
					Material_package_get_default_material(command_data->material_package),
					command_data->default_font, command_data->light_manager,
					command_data->spectrum_manager,command_data->default_spectrum,
					command_data->texture_manager);
				Scene_set_graphical_element_mode(command_data->default_scene,
					GRAPHICAL_ELEMENT_LINES,
					Computed_field_package_get_computed_field_manager(
						command_data->computed_field_package),
					command_data->root_region, command_data->data_root_region,
					command_data->element_point_ranges_selection,
					command_data->element_selection,command_data->node_selection,
					command_data->data_selection,command_data->user_interface);
				/*???RC.  May want to use functions to modify default_scene here */
				/* eg. to add model lights, etc. */
				/* ACCESS default so can never be destroyed */
				/*???RC.  Should be able to change: eg. gfx set default scene NAME */
				/*???DB.  Include default as part of manager ? */
				ACCESS(Scene)(command_data->default_scene);
				if (!ADD_OBJECT_TO_MANAGER(Scene)(command_data->default_scene,
						 command_data->scene_manager))
				{
					DEACCESS(Scene)(&(command_data->default_scene));
				}
			}
		}

		command_data->default_time_keeper=ACCESS(Time_keeper)(
			CREATE(Time_keeper)("default", command_data->event_dispatcher,
				command_data->user_interface));
		if(command_data->default_scene)
		{
			Scene_enable_time_behaviour(command_data->default_scene,
				command_data->default_time_keeper);
		}
		if (command_data->computed_field_package && command_data->default_time_keeper)
		{
			Computed_field_register_types_time(command_data->computed_field_package,
				command_data->default_time_keeper);
		}

		if (command_data->user_interface)
		{
			/* set up image library */
#if defined (UNIX) /* switch (Operating_System) */
			Open_image_environment(*argv);
#elif defined (WIN32_USER_INTERFACE) /* switch (Operating_System) */
			/* SAB Passing a string to this function so that it
				starts up, should get the correct thing from
				the windows system */
			Open_image_environment("cmgui");
#endif /* switch (Operating_System) */
			command_data->transform_tool=create_Interactive_tool_transform(
				command_data->user_interface);
			ADD_OBJECT_TO_MANAGER(Interactive_tool)(command_data->transform_tool,
				command_data->interactive_tool_manager);
			command_data->node_tool=CREATE(Node_tool)(
				command_data->interactive_tool_manager,
				command_data->root_region, /*use_data*/0,
				command_data->node_selection,
				command_data->computed_field_package,
				Material_package_get_default_material(command_data->material_package),
				command_data->user_interface,
				command_data->default_time_keeper,
				command_data->execute_command);
			command_data->data_tool=CREATE(Node_tool)(
				command_data->interactive_tool_manager,
				command_data->data_root_region, /*use_data*/1,
				command_data->data_selection,
				command_data->computed_field_package,
				Material_package_get_default_material(command_data->material_package),
				command_data->user_interface,
				command_data->default_time_keeper,
				command_data->execute_command);
			command_data->element_tool=CREATE(Element_tool)(
				command_data->interactive_tool_manager,
				command_data->root_region,
				command_data->element_selection,
				command_data->element_point_ranges_selection,
				command_data->computed_field_package,
				Material_package_get_default_material(command_data->material_package),
				command_data->user_interface,
				command_data->default_time_keeper,
				command_data->execute_command);
			command_data->element_point_tool=CREATE(Element_point_tool)(
				command_data->interactive_tool_manager,
				command_data->element_point_ranges_selection,
				command_data->computed_field_package,
				Material_package_get_default_material(command_data->material_package),
				command_data->user_interface,
				command_data->default_time_keeper,
				command_data->execute_command);
#if defined (MOTIF)
			command_data->select_tool=CREATE(Select_tool)(
				command_data->interactive_tool_manager,
				command_data->any_object_selection,
				Material_package_get_default_material(command_data->material_package),
				command_data->user_interface);
#endif /* defined (MOTIF) */
		}

#if defined (WIN32_USER_INTERFACE)
		command_data->hInstance=current_instance;
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		if (command_data->user_interface)
		{
			command_data->scene_viewer_package = CREATE(Cmiss_scene_viewer_package)
				(command_data->graphics_buffer_package,
				&command_data->background_colour,
				command_data->interactive_tool_manager, command_data->transform_tool,
				command_data->light_manager, command_data->default_light,
				command_data->light_model_manager, command_data->default_light_model,
				command_data->scene_manager, command_data->default_scene,
				command_data->texture_manager, command_data->user_interface);
		}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */

		/* properly set up the Execute_command objects */
		Execute_command_set_command_function(command_data->execute_command,
			cmiss_execute_command, (void *)command_data);
		Execute_command_set_command_function(command_data->set_command,
			cmiss_set_command, (void *)command_data);

#if defined (UNEMAP)
		command_data->unemap_command_data = CREATE(Unemap_command_data)(
			command_data->event_dispatcher,
			command_data->execute_command,
			command_data->user_interface,
#if defined (MOTIF)
			command_data->node_tool,
			command_data->transform_tool,
#endif /* defined (MOTIF) */
			command_data->glyph_list,
			command_data->computed_field_package,
			command_data->basis_manager,
			command_data->root_region,
			command_data->data_root_region,
			command_data->graphics_buffer_package,
			Material_package_get_material_manager(command_data->material_package),
			Material_package_get_default_material(command_data->material_package),
			command_data->default_font,
			command_data->io_stream_package,
			command_data->interactive_tool_manager,
			command_data->light_manager,
			command_data->default_light,
			command_data->light_model_manager,
			command_data->default_light_model,
			command_data->texture_manager,
			command_data->scene_manager,
			command_data->spectrum_manager,
			command_data->element_point_ranges_selection,
			command_data->element_selection,
			command_data->data_selection,
			command_data->node_selection,
			(struct System_window *)NULL,
			command_data->default_time_keeper
			);
#endif /* defined (UNEMAP) */

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
			if (!no_display)
			{
#if defined (MOTIF)
				/* register the callbacks in the global name table */
				if (MrmSUCCESS==MrmRegisterNames(callbacks,XtNumber(callbacks)))
				{
#endif /* defined (MOTIF) */
					/* create the main window */
					/* construct the version ID string which is exported in the command
						windows version atom */
					strcpy(version_id_string,"cmiss*");
					/* version number */
					if (version_ptr=strstr(version_string,"version "))
					{
						strncpy(version_temp,version_ptr+8,11);
						version_temp[11] = 0;
						strcat(version_id_string,version_temp);
					}
					strcat(version_id_string,"*");
					/* id from runtime */
					if (version_command_id)
					{
						strcat(version_id_string,version_command_id);
					}
					strcat(version_id_string,"*");
					/* link and runtime options */
					if (start_mycm)
					{
						strcat(version_id_string, "mycm ");
					}
					else 
					{
						if (start_cm)
						{
							strcat(version_id_string, "cm ");
						}
					}

					if (!server_mode)
					{
#if defined(USE_CMGUI_COMMAND_WINDOW) 
						if (console_mode)
						{
#endif /* defined(USE_CMGUI_COMMAND_WINDOW) */
							if (!(command_data->command_console = CREATE(Console)(
										command_data->execute_command,
										command_data->event_dispatcher, /*stdin*/0)))
							{
								display_message(ERROR_MESSAGE,"main.  "
									"Unable to create console.");
							}
#if defined(USE_CMGUI_COMMAND_WINDOW)
						}
						else
						{
							if (command_window=CREATE(Command_window)(command_data->execute_command,
									command_data->user_interface,version_id_string))
							{
								command_data->command_window=command_window;
								if (!batch_mode)
								{
									/* set up messages */
									set_display_message_function(ERROR_MESSAGE,
										display_error_message,command_window);
									set_display_message_function(INFORMATION_MESSAGE,
										display_information_message,command_window);
									set_display_message_function(WARNING_MESSAGE,
										display_warning_message,command_window);
#if defined (PERL_INTERPRETER)
									redirect_interpreter_output(command_data->interpreter, &return_code);
#endif /* defined (PERL_INTERPRETER) */
								}
#if defined (MOTIF)
								XSetErrorHandler(x_error_handler);
#endif /* defined (MOTIF) */
							}
							else
							{
								display_message(ERROR_MESSAGE,"Unable to create command window");
								return_code=0;
							}
						}
#endif /* defined(USE_CMGUI_COMMAND_WINDOW) */
					}
#if defined (MOTIF)
				}
				else
				{
					display_message(ERROR_MESSAGE,"Unable to register callbacks");
					return_code=0;
				}
#endif /* defined (MOTIF) */
			}
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
#if defined (MOTIF)
				/*???DB.  SIGBUS is not POSIX */
				case SIGBUS:
				{
					printf("Bus error occurred\n");
					display_message(ERROR_MESSAGE,"Bus error occurred");
				} break;
#endif /* defined (MOTIF) */
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
			DESTROY(Cmiss_command_data)(&command_data);
			command_data = (struct Cmiss_command_data *)NULL;
		}
	}
	else
	{
		command_data = (struct Cmiss_command_data *)NULL;
	}
	LEAVE;

	return (command_data);
} /* CREATE(Cmiss_command_data) */

int Cmiss_command_data_main_loop(struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Process events until some events request the program to finish.
==============================================================================*/
{
	int return_code;

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

int DESTROY(Cmiss_command_data)(struct Cmiss_command_data **command_data_address)
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Clean up the command_data, deallocating all the associated memory and resources.
==============================================================================*/
{
	int return_code;
#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
	int status;
#endif /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */
	struct Cmiss_command_data *command_data;

	ENTER(DESTROY(Cmiss_command_data));

	if (command_data_address && (command_data = *command_data_address))
	{
		/* clean-up memory */
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		if (command_data->scene_viewer_package)
		{
			DESTROY(Cmiss_scene_viewer_package)(&command_data->scene_viewer_package);		
		}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */

#if defined (SGI_MOVIE_FILE) && defined (MOTIF)
		DESTROY(MANAGER(Movie_graphics))(&command_data->movie_graphics_manager);
#endif /* defined (SGI_MOVIE_FILE) && defined (MOTIF) */

#if defined (CELL)
		/*created in execute_command_cell_open in command/cmiss.c */
		if (command_data->cell_interface)
		{
			DESTROY(Cell_interface)(&command_data->cell_interface);
		}
#endif /* defined (CELL) */
#if defined (UNEMAP)
		DESTROY(Unemap_command_data)(&command_data->unemap_command_data);
#endif /* defined (UNEMAP) */
		if (command_data->emoter_slider_dialog)
		{
			DESTROY(Emoter_dialog)(&command_data->emoter_slider_dialog);
		}
#if defined (MOTIF)
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
		if (command_data->time_editor_dialog)
		{
			DESTROY(Time_editor_dialog)(&(command_data->time_editor_dialog));
		}
		if (command_data->material_editor_dialog)
		{
			DESTROY(Material_editor_dialog)(&(command_data->material_editor_dialog));
		}
#endif /* defined (MOTIF) */
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
#if defined (MOTIF) || defined (WX_USER_INTERFACE)
		if (command_data->scene_editor)
		{
			DESTROY(Scene_editor)(&(command_data->scene_editor));
		}
		if (command_data->spectrum_editor_dialog)
		{
			DESTROY(Spectrum_editor_dialog)(&(command_data->spectrum_editor_dialog));
		}
#endif /* defined (MOTIF) || defined (WX_USER_INTERFACE) */

#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		DESTROY(MANAGER(Graphics_window))(
			&command_data->graphics_window_manager);
		/* Must destroy the graphics_buffer_package after the windows which use it */
		if (command_data->graphics_buffer_package)
		{
			DESTROY(Graphics_buffer_package)(&command_data->graphics_buffer_package);
		}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */

#if defined (MOTIF)
		if (command_data->element_creator)
		{
			DESTROY(Element_creator)(&command_data->element_creator);
		}
		/* destroy Interactive_tools and manager */
		if (command_data->select_tool)
		{
			DESTROY(Select_tool)(&command_data->select_tool);
		}
#endif /* defined (MOTIF) */
#if !defined (WX_USER_INTERFACE)
		if (command_data->data_tool)
		{
			 DESTROY(Node_tool)(&command_data->data_tool);
		}
		if (command_data->node_tool)
		{
			 DESTROY(Node_tool)(&command_data->node_tool);
		}
		if (command_data->element_tool)
		{
			 DESTROY(Element_tool)(&command_data->element_tool);
		}
		if (command_data->element_point_tool)
		{
			 DESTROY(Element_point_tool)(&command_data->element_point_tool);
		}
#else
		 /* deallocate the current region path when cmiss command_data is
			 being destroyed to prevent multiple deallocations of the same
			 address under DESTROY(Node_tool) which cause segfault in
			 cmgui-wx since the interactive tools are set up differently*/
		char *path;
		path = Node_tool_get_current_region_path(command_data->node_tool);	
		if (path)
		{
			 DEALLOCATE(path);
		}
		path = Node_tool_get_current_region_path(command_data->data_tool);	
		if (path)
		{
			 DEALLOCATE(path);
		}
#endif /* !defined(WX_USER_INTERFACE)*/
		DESTROY(MANAGER(Interactive_tool))(
			 &(command_data->interactive_tool_manager));

		DEACCESS(Scene)(&(command_data->default_scene));
		DESTROY(MANAGER(Scene))(&command_data->scene_manager);
		DESTROY(Time_keeper)(&command_data->default_time_keeper);
		if (command_data->computed_field_package)
		{
			Computed_field_package_remove_types(command_data->computed_field_package);
			DESTROY(Computed_field_package)(&command_data->computed_field_package);
		}

		DESTROY(Any_object_selection)(&(command_data->any_object_selection));
		DESTROY(FE_node_selection)(&(command_data->data_selection));
		DESTROY(FE_node_selection)(&(command_data->node_selection));
		DESTROY(FE_element_selection)(&(command_data->element_selection));
		DESTROY(Element_point_ranges_selection)(
			&(command_data->element_point_ranges_selection));

#if defined (SELECT_DESCRIPTORS)
		DESTROY(LIST(Io_device))(&command_data->device_list);
#endif /* defined (SELECT_DESCRIPTORS) */

		DESTROY(LIST(GT_object))(&command_data->graphics_object_list);
		DESTROY(LIST(GT_object))(&command_data->glyph_list);

		DESTROY(MANAGER(Curve))(&command_data->curve_manager);

		/* remove callbacks synchronising root_region and data_root_region since
			 they were established by the Command_data -- this must be done before
			 the regions are destroyed */
		Cmiss_region_remove_callback(command_data->root_region,
			Cmiss_region_synchronise_children_with_FE_region,
			(void *)(command_data->data_root_region));
		Cmiss_region_remove_callback(command_data->data_root_region,
			Cmiss_region_synchronise_children_with_FE_region,
			(void *)(command_data->root_region));

		DEACCESS(Cmiss_region)(&(command_data->data_root_region));
		DEACCESS(Cmiss_region)(&(command_data->root_region));
		DESTROY(MANAGER(FE_basis))(&command_data->basis_manager);
		DESTROY(LIST(FE_element_shape))(&command_data->element_shape_list);
		DEACCESS(Spectrum)(&(command_data->default_spectrum));
		DESTROY(MANAGER(Spectrum))(&command_data->spectrum_manager);
		DEACCESS(Material_package)(&command_data->material_package);
		DEACCESS(Graphics_font)(&command_data->default_font);
		DESTROY(Graphics_font_package)(&command_data->graphics_font_package);
		DESTROY(MANAGER(VT_volume_texture))(&command_data->volume_texture_manager);
		DESTROY(MANAGER(Texture))(&command_data->texture_manager);
		DESTROY(MANAGER(Environment_map))(&command_data->environment_map_manager);				
		DEACCESS(Light_model)(&(command_data->default_light_model));
		DESTROY(MANAGER(Light_model))(&command_data->light_model_manager);
		DEACCESS(Light)(&(command_data->default_light));
		DESTROY(MANAGER(Light))(&command_data->light_manager);
		if (command_data->scene_object_transformation_data)
		{
			 DEALLOCATE(command_data->scene_object_transformation_data);
		}
#if defined (MOTIF) || (WX_USER_INTERFACE)
		DESTROY(MANAGER(Comfile_window))(&command_data->comfile_window_manager);
#endif /* defined (MOTIF) || (WX_USER_INTERFACE) */

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

#if defined (MOTIF)
		coord_widget_finish();
#endif /* defined (MOTIF) */

		Close_image_environment();

		DESTROY(Execute_command)(&command_data->execute_command);
		DESTROY(Execute_command)(&command_data->set_command);

#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
		destroy_interpreter(command_data->interpreter, &status);
#endif /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */

		if (command_data->command_console)
		{
			DESTROY(Console)(&command_data->command_console);
		}
#if defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
		if (command_data->command_window)
		{
			DESTROY(Command_window)(&command_data->command_window);
		}
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */

		if (command_data->user_interface)
		{
			/* reset up messages */
			set_display_message_function(ERROR_MESSAGE,
				(Display_message_function *)NULL, NULL);
			set_display_message_function(INFORMATION_MESSAGE,
				(Display_message_function *)NULL, NULL);
			set_display_message_function(WARNING_MESSAGE,
				(Display_message_function *)NULL, NULL);
			/* close the user interface */
			DESTROY(User_interface)(&(command_data->user_interface));
		}

		if (command_data->event_dispatcher)
		{
			DESTROY(Event_dispatcher)(&command_data->event_dispatcher);
		}		

		DESTROY(IO_stream_package)(&command_data->io_stream_package);

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
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Cmiss_command_data).  "
			"Invalid arguments");
		return_code=0;
	}
	
	/* Write out any memory blocks still ALLOCATED when MEMORY_CHECKING is
		on.  When MEMORY_CHECKING is off this function does nothing */
	list_memory(/*count_number*/0, /*show_pointers*/0, /*increment_counter*/0,
		/*show_structures*/1);
	
	LEAVE;
	
	return (return_code);
} /* DESTROY(Cmiss_command_data) */

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
		root_region=command_data->root_region;
	}
	LEAVE;

	return (root_region);
} /* Cmiss_command_data_get_root_region */

struct Time_keeper *Cmiss_command_data_get_default_time_keeper(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 30 July 2003

DESCRIPTION :
Returns the default time_keeper from the <command_data>.
==============================================================================*/
{
	struct Time_keeper *default_time_keeper;

	ENTER(Cmiss_command_data_get_default_time_keeper);
	default_time_keeper=(struct Time_keeper *)NULL;
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

struct Fdio_package* Cmiss_command_data_get_fdio_package(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 10 March 2005

DESCRIPTION :
Gets an Fdio_package for this <command_data>
==============================================================================*/
{
	struct Fdio_package *fdio_package;

	ENTER(Cmiss_command_data_get_fdio_package);
	fdio_package = (struct Fdio_package *)NULL;
	if (command_data)
	{
		fdio_package = CREATE(Fdio_package)(command_data->event_dispatcher);
	}
	LEAVE;

	return (fdio_package);
}

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

Timer_package_id Cmiss_command_data_get_timer_package(
	struct Cmiss_command_data *command_data
)
/*******************************************************************************
LAST MODIFIED : 11 April 2005

DESCRIPTION :
Gets a Timer_package for this <command_data>
==============================================================================*/
{
	struct Timer_package *timer_package;

	ENTER(Cmiss_command_data_get_timer_package);
	timer_package = (struct Timer_package *)NULL;
	if (command_data)
	{
		timer_package = CREATE(Timer_package)(command_data->event_dispatcher);
	}
	LEAVE;

	return (timer_package);
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

struct FE_element_selection *Cmiss_command_data_get_element_selection(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 4 July 2005

DESCRIPTION :
Returns the selected_element object from the <command_data>.
==============================================================================*/
{
	struct FE_element_selection *element_selection;

	ENTER(Cmiss_command_data_get_element_selection);
	element_selection=(struct FE_element_selection *)NULL;
	if (command_data)
	{
		element_selection = command_data->element_selection;
	}
	LEAVE;

	return (element_selection);
} /* Cmiss_command_data_get_element_selection */

struct FE_node_selection *Cmiss_command_data_get_node_selection(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 4 July 2005

DESCRIPTION :
Returns the selected_node object from the <command_data>.
==============================================================================*/
{
	struct FE_node_selection *node_selection;

	ENTER(Cmiss_command_data_get_node_selection);
	node_selection=(struct FE_node_selection *)NULL;
	if (command_data)
	{
		node_selection = command_data->node_selection;
	}
	LEAVE;

	return (node_selection);
} /* Cmiss_command_data_get_node_selection */

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

struct MANAGER(Texture) *Cmiss_command_data_get_texture_manager(
	struct Cmiss_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 7 November 2006

DESCRIPTION :
Returns the texture manager from the <command_data>.
==============================================================================*/
{
	struct MANAGER(Texture) *texture_manager;

	ENTER(Cmiss_command_data_get_texture_manager);
	texture_manager=(struct MANAGER(Texture) *)NULL;
	if (command_data)
	{
		texture_manager = command_data->texture_manager;
	}
	LEAVE;

	return (texture_manager);
} /* Cmiss_command_data_get_texture_manager */

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
