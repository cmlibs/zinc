/*******************************************************************************
FILE : cell_window.h

LAST MODIFIED : 28 August 2000

DESCRIPTION :
Functions for using the Cell structure.
==============================================================================*/
#if !defined (CELL_WINDOW_H)
#define CELL_WINDOW_H

#if defined (MOTIF)
#include <Xm/Xm.h>
#endif /* if defined (MOTIF) */
#include "xml.h"
#include "general/debug.h"
#include "interaction/interactive_tool.h"
#include "selection/any_object_selection.h"
#include "user_interface/message.h"
#include "unemap/analysis_work_area.h"

/*
Global types
============
*/
enum Cell_array
/*******************************************************************************
LAST MODIFIED : 24 May 1999

DESCRIPTION :
The different arrays.
???DB.  Should be hidden inside .c ?
==============================================================================*/
{
  ARRAY_STATE,
  ARRAY_DERIVED,
  ARRAY_MODEL,
  ARRAY_CONTROL,
  ARRAY_PARAMETERS,
  ARRAY_PROTOCOL,
  ARRAY_AII,
  ARRAY_AIO,
  ARRAY_ARI,
  ARRAY_ARO,
  ARRAY_UNKNOWN
}; /* Cell_array */

struct Cell_user_settings
/*******************************************************************************
LAST MODIFIED : 28 February 1999

DESCRIPTION :
Stores the user settings for the Cell window.
???DB.  Should be hidden inside .c
==============================================================================*/
{
  char *default_foreground,*default_background,*channel_colour,
    *channel_label_colour,*exchanger_colour,*exchanger_label_colour,
    *pump_colour,*pump_label_colour,*mech_colour,*mech_label_colour,
    *ion_colour,*ion_label_colour,*label_font;
  Pixel text_emphasis_colour;
}; /* Cell_user_settings */

struct Cell_window
/*******************************************************************************
LAST MODIFIED : 28 August 2000

DESCRIPTION :
Main structure containing all CELL data.
???DB.  Should be hidden inside .c
==============================================================================*/
{
  int default_values; /* used to distinguish between reading default values
                         and new, user defined values */
	int single_cell; /* set to 1 when wrinting out ipcell files for single
											cell calculations */
  struct XML_Tree tree;
  char *current_model,*current_model_id;
  struct User_interface *user_interface;
  struct Computed_field_package *computed_field_package;
	struct Element_point_ranges_selection *element_point_ranges_selection;
  struct Execute_command *execute_command;
  FILE *output_file;
  Widget window;
  Widget shell;
  Widget output_pane; /* used to display messages - NOT now!! */
  struct Cell_user_settings *user_settings;
  struct /* menu */
  {
    int save;
    int debug;
  } menu;
  struct /* image map */
  {
    Widget drawing_area;
    Pixmap pixmap;
    Window window;
    int width;
    int height;
  } image_map;
  struct /* UnEMAP stuff */
  {
    int number_of_saved_buffers;
    struct Analysis_work_area analysis;
    struct Signal_buffer **saved_buffers;
		struct Unemap_package *package;
  } unemap;
  struct /* time stuff */
  {
    float *TABT;
    float *TSTART;
    float *TEND;
    float DT;
  } time_steps;
  struct /* control curve stuff */
  {
    Widget control_curve_editor_dialog;
    struct MANAGER(Control_curve) *control_curve_manager;
  } control_curve;
  struct /* Cell 3d stuff */
  {
		struct Any_object_selection *any_object_selection;
		struct MANAGER(Interactive_tool) *interactive_tool_manager;
		struct Interactive_tool *interactive_tool;
    Widget form,toolbar_form,toolbar_widget;
    struct Scene_viewer *scene_viewer;
    struct Colour *background_colour;
    struct MANAGER(Light) *light_manager;
    struct Light *default_light;
    struct MANAGER(Light_model) *light_model_manager;
    struct Light_model *default_light_model;
    struct MANAGER(Scene) *scene_manager;
    struct Scene *scene;
    struct MANAGER(Texture) *texture_manager;
    struct LIST(GT_object) *graphics_object_list;
    struct MANAGER(Graphical_material) *graphical_material_manager;
    struct Graphical_material *default_graphical_material;
    struct LIST(GT_object) *glyph_list;
    struct MANAGER(FE_field) *fe_field_manager;
    struct MANAGER(GROUP(FE_element)) *element_group_manager;
    struct MANAGER(FE_node) *node_manager;
    struct MANAGER(FE_element) *element_manager;
    struct MANAGER(GROUP(FE_node)) *node_group_manager;
    struct MANAGER(GROUP(FE_node)) *data_group_manager;
    struct MANAGER(Spectrum) *spectrum_manager;
    struct Spectrum *default_spectrum;
    void *node_group_callback_id;
  } cell_3d;
#if defined (CELL_USE_NODES)
  struct /* distributed modelling stuff */
  {
    int edit;
    Widget node_chooser_label,node_chooser_form,node_chooser_widget;
    Widget description_label;
    Widget apply_button,reset_button;
		Widget export_menu_button;
  } distributed;
#endif /* defined (CELL_USE_NODES) */
  struct /* distributed modelling stuff */
  {
    /* information about the element point being edited; note element in
       identifier is not accessed */
    struct Element_point_ranges_identifier element_point_identifier;
    int element_point_number;
    /* accessed local copy of the element being edited */
    struct FE_element *element_copy;
    FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
    Widget element_form,element_widget,point_number_text,
      grid_field_form,grid_field_widget,grid_value_text;
    int edit;
    Widget description_label;
    Widget apply_button,reset_button;
		Widget export_menu_button;
  } distributed;
  struct Model_dialog *model_dialog;
  struct Variables_dialog *variables_dialog;
  struct Cell_variable *variables;
  struct Cell_component *components;
  struct Cell_parameter *parameters;
  struct Cell_output *outputs;
  struct Cell_graphic *graphics;
  struct Export_dialog *export_dialog;
  struct Export_control_curve_dialog *export_control_curve_dialog;
}; /* struct Cell_window */

/*
Global functions
================
*/
struct Cell_window *create_Cell_window(struct User_interface *user_interface,
  char *filename,struct MANAGER(Control_curve) *control_curve_manager,
  struct Unemap_package *package,
	struct Any_object_selection *any_object_selection,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
  struct Colour *background_colour,struct MANAGER(Light) *light_manager,
  struct Light *default_light,struct MANAGER(Light_model) *light_model_manager,
  struct Light_model *default_light_model,struct MANAGER(Scene) *scene_manager,
  struct Scene *default_scene,struct MANAGER(Texture) *texture_manager,
  struct LIST(GT_object) *graphics_object_list,
  struct MANAGER(Graphical_material) *graphical_material_manager,
  struct LIST(GT_object) *glyph_list,struct MANAGER(FE_field) *fe_field_manager,
  struct MANAGER(GROUP(FE_element)) *element_group_manager,
  struct MANAGER(FE_node) *node_manager,
  struct MANAGER(FE_element) *element_manager,
  struct MANAGER(GROUP(FE_node)) *node_group_manager,
  struct MANAGER(GROUP(FE_node)) *data_group_manager,
  struct Graphical_material *default_graphical_material,
  struct MANAGER(Spectrum) *spectrum_manager,struct Spectrum *default_spectrum,
  struct Computed_field_package *computed_field_package,
	struct Element_point_ranges_selection *element_point_ranges_selection,
  struct Execute_command *execute_command);
/*******************************************************************************
LAST MODIFIED : 28 August 2000

DESCRIPTION :
Create the structures and retrieve the cell window from the uil file. <filename>
specifies a file to print messages to, if non-NULL.
==============================================================================*/
int write_cell_window(char *message,struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 03 February 1999

DESCRIPTION :
Writes the <message> to the <cell> window.
==============================================================================*/
#if defined (CELL_USE_NODES)
void update_cell_window_from_node(struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 15 September 1999

DESCRIPTION :
Updates the cell window from a node, if a node exists in the node chooser
==============================================================================*/
#endif /* defined (CELL_USE_NODES) */
int Cell_read_model(char *filename,struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
Wrapper function used to execute the CELL READ MODEL command.
==============================================================================*/

#endif /* if !defined (CELL_WINDOW_H) */
