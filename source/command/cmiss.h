/*******************************************************************************
FILE : cmiss.h

LAST MODIFIED : 12 August 2002

DESCRIPTION :
Functions and types for executing cmiss commands.

This should only be included in cmgui.c and command/cmiss.c
==============================================================================*/
#if !defined (COMMAND_CMISS_H)
#define COMMAND_CMISS_H

#include "command/command.h"
#include "finite_element/finite_element.h"
#include "graphics/graphics_object.h"

/*
Global types
------------
*/
struct Cmiss_command_data
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
==============================================================================*/
{
	char *cm_examples_directory,*cm_parameters_file_name,*example_directory,
		*examples_directory,*example_comfile,*help_directory,*help_url;
#if defined (CELL)
	struct Cell_interface *cell_interface;
#endif /* defined (CELL) */
	struct Console *command_console;
#if defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE)
	struct Command_window *command_window;
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
	struct Colour background_colour,foreground_colour;
	struct Execute_command *execute_command,*set_command;
#if defined (MOTIF)
	struct Element_point_tool *element_point_tool;
	struct Element_tool *element_tool;
#endif /* defined (MOTIF) */
	struct Event_dispatcher *event_dispatcher;
	struct Node_tool *data_tool,*node_tool;
#if defined (MOTIF)
	struct Select_tool *select_tool;
#endif /* defined (MOTIF) */
	struct Interactive_tool *transform_tool;
#if defined (SELECT_DESCRIPTORS)
	struct LIST(Io_device) *device_list;
#endif /* defined (SELECT_DESCRIPTORS) */
	/*???RC.  Single list of graphics objects - eventually set up manager ? */
	struct LIST(GT_object) *graphics_object_list;
	/* list of glyphs = simple graphics objects with only geometry */
	struct LIST(GT_object) *glyph_list;
	struct FE_time *fe_time;
#if defined (MOTIF)
	struct MANAGER(Comfile_window) *comfile_window_manager;
#endif /* defined (MOTIF) */
	struct Computed_field_package *computed_field_package;
	struct MANAGER(Environment_map) *environment_map_manager;
	struct MANAGER(FE_basis) *basis_manager;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *data_manager,*node_manager;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct Graphical_material *default_graphical_material;
#if defined (MOTIF) || defined (GTK_USER_INTERFACE)
	struct MANAGER(Graphics_window) *graphics_window_manager;
#endif /* defined (MOTIF) || defined (GTK_USER_INTERFACE) */
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
		/*???DB.  Also manages faces and lines */
	struct MANAGER(GROUP(FE_node)) *data_group_manager,*node_group_manager;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct MANAGER(Light) *light_manager;
	struct Light *default_light;
	struct MANAGER(Light_model) *light_model_manager;
	struct Light_model *default_light_model;
#if defined (SGI_MOVIE_FILE)
	struct MANAGER(Movie_graphics) *movie_graphics_manager;
#endif /* defined (SGI_MOVIE_FILE) */
	struct MANAGER(Texture) *texture_manager;
	struct MANAGER(Control_curve) *control_curve_manager;
	struct MANAGER(Scene) *scene_manager;
	struct Scene *default_scene;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;
	struct Modifier_entry *set_file_name_option_table;
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
#if defined (MOTIF)
#if defined (MIRAGE)
	struct Tracking_editor_dialog *tracking_editor_dialog;
		/*???DB.  Should this be a widget like the other tools ? */
#endif /* defined (MIRAGE) */
#endif /* defined (MOTIF) */
	struct User_interface *user_interface;
#if defined (MOTIF)
	Widget control_curve_editor_dialog,data_grabber_dialog,
		emoter_slider_dialog,grid_field_calculator_dialog,input_module_dialog,
		interactive_data_editor_dialog,interactive_node_editor_dialog,
		node_group_slider_dialog, sync_2d_3d_dialog;
	struct Node_viewer *data_viewer,*node_viewer;
	struct Element_point_viewer *element_point_viewer;
	struct Element_creator *element_creator;
	struct Material_editor_dialog *material_editor_dialog;
	struct Scene_editor *scene_editor;
	struct Spectrum_editor_dialog *spectrum_editor_dialog;
	struct Time_editor_dialog *time_editor_dialog;
#endif /* defined (MOTIF) */
#if defined (UNEMAP)
	struct Unemap_command_data *unemap_command_data;
#endif /* defined (UNEMAP) */
}; /* struct Cmiss_command_data */

/*
Global functions
----------------
*/

int cmiss_execute_command(char *command_string,void *command_data);
/*******************************************************************************
LAST MODIFIED : 9 November 1998

DESCRIPTION :
Execute a <command_string>.
==============================================================================*/

#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
void execute_command(char *command_string,void *command_data_void, int *quit,
  int *error);
/*******************************************************************************
LAST MODIFIED : 28 March 2000

DESCRIPTION:
==============================================================================*/
#endif /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */

int cmiss_set_command(char *command_string,void *command_data_void);
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION:
Sets the <command_string> in the command box of the CMISS command_window, ready
for editing or entering. If there is no command_window, does nothing.
==============================================================================*/

#endif /* !defined (COMMAND_CMISS_H) */
