/*******************************************************************************
FILE : unemap_package.h

LAST MODIFIED :  2 September 1999

DESCRIPTION :

==============================================================================*/
#if !defined (UNEMAP_PACKAGE_H)
#define UNEMAP_PACKAGE_H

#include "general/managed_group.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/colour.h"
#include "graphics/spectrum.h"
#include "finite_element/finite_element.h"
#include "unemap/rig.h" /* for enum Region_type.???JWMaybe we should move this elsewhere?*/

/*
Global types
------------
*/
struct Unemap_package/* Still need structure, for (NULL) function parameters, */ 
/*even when UNEMAP_USE_NODE not defined*/
/*******************************************************************************
LAST MODIFIED : 9 July 1999

DESCRIPTION :
Stores information needed to construct rig_node element,nodes, fields,
and map element,nodes, fields. This information is also uses to clean up the 
element,nodes, fields when they are no longer required.
The map information is also used to compare a new map to the previous, to see if
the node and element groups need to be regenerated, or just have the values changed.

==============================================================================*/
{
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(GROUP(FE_element))	*element_group_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager,*node_group_manager; 
	struct MANAGER(FE_basis) *fe_basis_manager;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct LIST(GT_object) *glyph_list;
	/* fields of the rig_nodes, so we know what to clean up, and what to*/
	/*construct draw package with */
	/* for MIXED enum Region_type and hence rig_node groups, elctrode postion fields */
	/*	can be different  eg  a torso with rectangular cart coords and a sock with */
	/* prolate spheroidal coords. Therefore store multiple fields */
	int number_of_electrode_position_fields;
	struct FE_field **electrode_position_fields;
	/* these fields are always the same, no matter what the  Region_type*/
	struct FE_field *device_name_field; 
	struct FE_field *device_type_field;
	struct FE_field *channel_number_field;
	struct FE_field *signal_field;
	struct FE_field *signal_minimum_field;
	struct FE_field *signal_maximum_field;
	struct FE_field *signal_status_field;
	struct FE_field *channel_gain_field;
	struct FE_field *channel_offset_field;
	int number_of_rig_node_groups;
	struct GROUP(FE_node) **rig_node_groups;
	/* info about the last map(s) */
	int number_of_maps;
	struct Map_info **maps_info;
	/* these are for the cmgui graphics window */
	int viewed_scene; /* flag to record if done "view all" on scene*/
	struct Colour no_interpolation_colour; /* should this and below be be in struct Map ?*/
	struct Graphics_window *window; /*or Map_drawing_information?  */
	struct Colour background_colour;
	struct Light *light;	
	struct Light_model *light_model;	
	struct Scene *scene;	
	struct User_interface *user_interface;
	struct Graphical_material *graphical_material;
	struct Time_keeper *time_keeper;	
	struct Computed_field_package *computed_field_package;
	struct MANAGER(Graphics_window) *graphics_window_manager;
	struct MANAGER(Texture) *texture_manager;
	struct MANAGER(Scene) *scene_manager;
	struct MANAGER(Light_model) *light_model_manager;
	struct MANAGER(Light) *light_manager;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct MANAGER(FE_node) *data_manager;	
	int access_count;
}; /* struct Unemap_package */

#if defined (UNEMAP_USE_NODES)

PROTOTYPE_OBJECT_FUNCTIONS(Unemap_package);

struct Unemap_package *CREATE(Unemap_package)
		 ( struct MANAGER(FE_field) *fe_field_manager,
			 struct MANAGER(GROUP(FE_element))	*element_group_manager,
			 struct MANAGER(FE_node) *node_manager,	
			 struct MANAGER(GROUP(FE_node)) *data_group_manager,
			 struct MANAGER(GROUP(FE_node)) *node_group_manager,
			 struct MANAGER(FE_basis) *fe_basis_manager,
			 struct MANAGER(FE_element) *element_manager,
			 struct MANAGER(Computed_field) *computed_field_manager,
			 struct MANAGER(Graphics_window) *graphics_window_manager,
			 struct MANAGER(Texture) *texture_manager,
			 struct MANAGER(Scene) *scene_manager,
			 struct MANAGER(Light_model) *light_model_manager,
			 struct MANAGER(Light) *light_manager,
			 struct MANAGER(Spectrum) *spectrum_manager,
			 struct MANAGER(Graphical_material) *graphical_material_manager,
			 struct MANAGER(FE_node) *data_manager,
			 struct LIST(GT_object) *glyph_list);
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :

==============================================================================*/

int DESTROY(Unemap_package)(struct Unemap_package **package_address);
/*******************************************************************************
LAST MODIFIED : 26 April 1999

DESCRIPTION :
Frees the memory for the Unemap_package node field and 
sets <*package_address> to NULL.
==============================================================================*/

struct Colour *get_unemap_package_no_interpolation_colour(struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
gets the no_interpolation_colour of the unemap package.
==============================================================================*/

int get_unemap_package_viewed_scene(struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
gets the viewed_scene flag of the unemap package.
==============================================================================*/

int set_unemap_package_viewed_scene(struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
sets the viewed_scene flag of the unemap package to 1.
==============================================================================*/

struct FE_field *get_unemap_package_electrode_position_field(
	struct Unemap_package *package,int field_number);
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/

int set_unemap_package_electrode_position_field(struct Unemap_package *package,
	struct FE_field *electrode_position_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/

struct FE_field *get_unemap_package_map_electrode_position_field(
	struct Unemap_package *package,int map_number);
/*******************************************************************************
LAST MODIFIED : October 19 1999

DESCRIPTION :
gets the map_electrode_position_field for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/

int set_unemap_package_map_electrode_position_field(struct Unemap_package *package,
	struct FE_field *map_electrode_position_field,int map_number);
/*******************************************************************************
LAST MODIFIED : October 19 1999

DESCRIPTION :
Sets the map_electrode_position_field  for map_info <map_number> in <package>.
Set (and get) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3...
==============================================================================*/

struct GT_object *get_unemap_package_map_electrode_glyph(
	struct Unemap_package *package,int map_number);
/*******************************************************************************
LAST MODIFIED : October 21 1999

DESCRIPTION :
gets the map_electrode_glyph for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/

int set_unemap_package_map_electrode_glyph(struct Unemap_package *package,
	struct GT_object *electrode_glyph,int map_number);
/*******************************************************************************
LAST MODIFIED : October 21 1999

DESCRIPTION :
Sets the electrode_glyph  for map_info <map_number> in <package>.
Set (and get) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3...
==============================================================================*/

struct FE_field *get_unemap_package_device_name_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/

int set_unemap_package_device_name_field(struct Unemap_package *package,
	struct FE_field *device_name_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/

struct FE_field *get_unemap_package_device_type_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/

int set_unemap_package_device_type_field(struct Unemap_package *package,
	struct FE_field *device_type_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/

struct FE_field *get_unemap_package_channel_number_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/

int set_unemap_package_channel_number_field(struct Unemap_package *package,
	struct FE_field *channel_number_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/

struct FE_field *get_unemap_package_signal_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/

int set_unemap_package_signal_field(struct Unemap_package *package,
	struct FE_field *signal_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/

struct FE_field *get_unemap_package_signal_minimum_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/

int set_unemap_package_signal_minimum_field(struct Unemap_package *package,
	struct FE_field *signal_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/

struct FE_field *get_unemap_package_signal_maximum_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/

int set_unemap_package_signal_maximum_field(struct Unemap_package *package,
	struct FE_field *signal_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/

struct FE_field *get_unemap_package_signal_status_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : October 8 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/

int set_unemap_package_signal_status_field(struct Unemap_package *package,
	struct FE_field *signal_field);
/*******************************************************************************
LAST MODIFIED : October 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/

struct FE_field *get_unemap_package_channel_offset_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/

int set_unemap_package_channel_offset_field(struct Unemap_package *package,
	struct FE_field *channel_offset_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/


struct FE_field *get_unemap_package_channel_gain_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/

int set_unemap_package_channel_gain_field(struct Unemap_package *package,
	struct FE_field *channel_gain_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/

struct MANAGER(FE_field) *get_unemap_package_FE_field_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/

struct MANAGER(Computed_field) *get_unemap_package_Computed_field_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 3 September 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/

struct MANAGER(GROUP(FE_element)) *get_unemap_package_element_group_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/

struct MANAGER(FE_node) *get_unemap_package_node_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/

struct MANAGER(FE_element) *get_unemap_package_element_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/

struct MANAGER(FE_basis) *get_unemap_package_basis_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/

struct MANAGER(GROUP(FE_node)) *get_unemap_package_data_group_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/

struct MANAGER(GROUP(FE_node)) *get_unemap_package_node_group_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/

int get_unemap_package_map_number_of_map_rows(
	struct Unemap_package *package,int map_number);
/*******************************************************************************
LAST MODIFIED : September 8 1999

DESCRIPTION :
gets the number_of_map_rows for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3...
Returns -1 on error
==============================================================================*/

int get_unemap_package_map_number_of_map_columns(
	struct Unemap_package *package,int map_number);
/*******************************************************************************
LAST MODIFIED : September 8 1999

DESCRIPTION :
gets the number_of_map_columns for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3...
Returns -1 on error
==============================================================================*/

int get_unemap_package_map_rig_node_group_number(
	struct Unemap_package *package,int map_number);
/*******************************************************************************
LAST MODIFIED :October 19 1999

DESCRIPTION :
gets the rig_node_group_number for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3...
Returns -1 on error
==============================================================================*/

enum Region_type get_unemap_package_map_region_type(
	struct Unemap_package *package,int map_number);
/*******************************************************************************
LAST MODIFIED : September 8 1999

DESCRIPTION :
gets the region_type for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/

char *get_unemap_package_map_fit_name(
	struct Unemap_package *package,int map_number);
/*******************************************************************************
LAST MODIFIED : September 8 1999

DESCRIPTION :
gets the fit_name for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/

struct FE_node_order_info *get_unemap_package_map_node_order_info(
	struct Unemap_package *package,int map_number);
/*******************************************************************************
LAST MODIFIED : September 8 1999

DESCRIPTION :
gets the node_order_info for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/

struct FE_field *get_unemap_package_map_position_field(
	struct Unemap_package *package,int map_number);
/*******************************************************************************
LAST MODIFIED : September 8 1999

DESCRIPTION :
gets the map_position_field for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/

struct FE_field *get_unemap_package_map_fit_field(
	struct Unemap_package *package,int map_number);
/*******************************************************************************
LAST MODIFIED : September 8 1999

DESCRIPTION :
gets the map_fit_field for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/

struct GROUP(FE_node) *get_unemap_package_rig_node_group(
	struct Unemap_package *package,int node_number);
/*******************************************************************************
LAST MODIFIED : August 16 1999

DESCRIPTION :
gets the rig_node_group of the unemap package.
==============================================================================*/

int set_unemap_package_rig_node_group(struct Unemap_package *package,
	struct GROUP(FE_node) *rig_node_group);
/*******************************************************************************
LAST MODIFIED : August 16 1999

DESCRIPTION :
Allocates and sets the latest rig_node_group pointer of the unemap package.
==============================================================================*/

struct GROUP(FE_node) *get_unemap_package_map_node_group(
	struct Unemap_package *package,int map_number);
/*******************************************************************************
LAST MODIFIED : 9 September 1999

DESCRIPTION :
gets the node_group for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/

struct GROUP(FE_element) *get_unemap_package_map_element_group(
	struct Unemap_package *package,int map_number);
/*******************************************************************************
LAST MODIFIED : September 10 1999

DESCRIPTION :
gets the element_group for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/

int set_unemap_package_map_element_group(struct Unemap_package *package,
	struct GROUP(FE_element) *map_element_group,int map_number);
/*******************************************************************************
LAST MODIFIED : September 10 1999

DESCRIPTION :
Sets the element_group for map_info <map_number> in <package>.
Set (and get) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3...
==============================================================================*/

int free_unemap_package_rig_computed_fields(struct Unemap_package *unemap_package);
/*******************************************************************************
LAST MODIFIED : August 20 1999

DESCRIPTION :
Frees the fields stored in the unemap_package that are used by the computed 
field manager
==============================================================================*/

int free_unemap_package_rig_fields(struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : August 19 1999

DESCRIPTION :
Frees the fields stored in the unemap_package that are used by the rid_node_group
==============================================================================*/

int free_unemap_package_rig_node_group_glyphs(struct Unemap_package *package,
	int rig_node_group_number);
/*******************************************************************************
LAST MODIFIED : 22 October 1999

DESCRIPTION :
Frees up any glyphs used by the nodes in the rig_node_group
==============================================================================*/

int free_unemap_package_rig_node_groups(struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : August 17 1999

DESCRIPTION :
Frees the rig node groups stored in the unemap_package
==============================================================================*/

int free_unemap_package_rig_info(struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : August 27 1999

DESCRIPTION :
Frees the fields, nodes, elements stored in unemap_package that are 
associated with the rig
==============================================================================*/

struct MANAGER(Graphics_window) *get_unemap_package_Graphics_window_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/

struct MANAGER(Light) *get_unemap_package_Light_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/

struct MANAGER(Texture) *get_unemap_package_Texture_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/

struct MANAGER(Scene) *get_unemap_package_Scene_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/

struct MANAGER(Light_model) *get_unemap_package_Light_model_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/

struct MANAGER(Graphical_material) *get_unemap_package_Graphical_material_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/

struct Graphics_window *get_unemap_package_window(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED :  September 2 1999

DESCRIPTION :
gets the Graphics_window of the unemap package.
==============================================================================*/

int set_unemap_package_window(struct Unemap_package *package,
	struct Graphics_window *window);
/*******************************************************************************
LAST MODIFIED : September 2 1999

DESCRIPTION :
Sets the Graphics_window of the unemap package.
==============================================================================*/

struct Colour *get_unemap_package_background_colour(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED :  September 2 1999

DESCRIPTION :
gets the Colour of the unemap package.
==============================================================================*/

int set_unemap_package_background_colour(struct Unemap_package *package,
	struct Colour background_colour);
/*******************************************************************************
LAST MODIFIED : September 2 1999

DESCRIPTION :
Sets the Colour of the unemap package.
==============================================================================*/

struct Light *get_unemap_package_light(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED :  September 2 1999

DESCRIPTION :
gets the Light of the unemap package.
==============================================================================*/

int set_unemap_package_light(struct Unemap_package *package,
	struct Light *light);
/*******************************************************************************
LAST MODIFIED : September 2 1999

DESCRIPTION :
Sets the Light of the unemap package.
==============================================================================*/

struct Light_model *get_unemap_package_light_model(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED :  September 2 1999

DESCRIPTION :
gets the Light_model of the unemap package.
==============================================================================*/

int set_unemap_package_light_model(struct Unemap_package *package,
	struct Light_model *light_model);
/*******************************************************************************
LAST MODIFIED : September 2 1999

DESCRIPTION :
Sets the Light_model of the unemap package.
==============================================================================*/

struct Scene *get_unemap_package_scene(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED :  September 2 1999

DESCRIPTION :
gets the Scene of the unemap package.
==============================================================================*/

int set_unemap_package_scene(struct Unemap_package *package,
	struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : September 2 1999

DESCRIPTION :
Sets the Scene of the unemap package.
==============================================================================*/

struct User_interface *get_unemap_package_user_interface(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED :  September 2 1999

DESCRIPTION :
gets the User_interface of the unemap package.
==============================================================================*/

int set_unemap_package_user_interface(struct Unemap_package *package,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : September 2 1999

DESCRIPTION :
Sets the User_interface of the unemap package.
==============================================================================*/

struct Graphical_material *get_unemap_package_graphical_material(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED :  September 3 1999

DESCRIPTION :
gets the Graphical_material of the unemap package.
==============================================================================*/

int set_unemap_package_graphical_material(struct Unemap_package *package,
	struct Graphical_material *graphical_material);
/*******************************************************************************
LAST MODIFIED : September 3 1999

DESCRIPTION :
Sets the Graphical_material of the unemap package.
==============================================================================*/

struct Computed_field_package *get_unemap_package_computed_field_package(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED :  September 7 1999

DESCRIPTION :
gets the Computed_field_package of the unemap package.
==============================================================================*/

int set_unemap_package_computed_field_package(struct Unemap_package *package,
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : September 7 1999

DESCRIPTION :
Sets the Computed_field_package of the unemap package.
==============================================================================*/

struct Time_keeper *get_unemap_package_time_keeper(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED :  September 3 1999

DESCRIPTION :
gets the Time_keeper of the unemap package.
==============================================================================*/

int set_unemap_package_time_keeper(struct Unemap_package *package,
	struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : September 3 1999

DESCRIPTION :
Sets the Time_keeper of the unemap package.
==============================================================================*/

struct LIST(GT_object) *get_unemap_package_glyph_list(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 21 October 1999

DESCRIPTION :
gets the glyph_list of the unemap package.
==============================================================================*/

int unemap_package_make_map_scene(struct Unemap_package *package,
	struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : September 27 1999

DESCRIPTION :
Creates the unemap_package scene, if isn't already present.
==============================================================================*/	

int free_unemap_package_maps(struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : September 10 1999

DESCRIPTION :
Attempt to destroy all the package's Map_infos.
Set the package number_of_maps to 0.
==============================================================================*/
	
int free_unemap_package_map_info(struct Unemap_package *package,int map_number);
/*******************************************************************************
LAST MODIFIED : September 16 1999

DESCRIPTION :
Attempt to destroy  the package's map_number Map_infos.
DOESN'T alter  the package number_of_maps. 
==============================================================================*/

int set_unemap_package_map_info(struct Unemap_package *package,int map_number,
	int rig_node_group_number,int number_of_map_rows,int number_of_map_columns,
	enum Region_type region_type,char *name,
	struct FE_node_order_info *node_order_info,
	struct FE_field *map_position_field,struct FE_field *map_fit_field,
	struct GROUP(FE_node) *node_group);
/*******************************************************************************
LAST MODIFIED : September 8 1999

DESCRIPTION :
Sets the <package> <map_number> Map_info, by freeing any existing one
and creating a new one.
==============================================================================*/	
#endif /* #if defined(UNEMAP_USE_NODES) */
#endif /* !defined (UNEMAP_PACKAGE_H) */
