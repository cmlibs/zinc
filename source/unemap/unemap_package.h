/*******************************************************************************
FILE : unemap_package.h

LAST MODIFIED : 19 June 2000

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
#include "interaction/interactive_tool.h"
#include "selection/element_point_ranges_selection.h"
#include "selection/element_selection.h"  
#include "selection/node_selection.h"
/* for enum Electrodes_option.???JWMaybe we should move this elsewhere?*/
/* or not use it at all - see map_remove_map_electrodes_if_changed */
#include "unemap/mapping.h" 
/* for enum Region_type.???JWMaybe we should move this elsewhere?*/
#include "unemap/rig.h" 

/*
Global types
------------
*/
struct Unemap_package
/* Still need structure, for (NULL) function parameters, */ 
/*even when UNEMAP_USE_NODE not defined*/
/*******************************************************************************
LAST MODIFIED : 28 April 2000

DESCRIPTION :
Stores information needed to construct rig_node element,nodes, fields,
and map element,nodes, fields. This information is also uses to clean up the 
element,nodes, fields when they are no longer required.
The map information is also used to compare a new map to the previous, to see if
the node and element groups need to be regenerated, or just have the values changed.

Currently Unemap_package is rather cluttered.
Currently storing info to make map (eg signals), info for 3D window, 
eg light model, and managers. Split into along these lines??JW

==============================================================================*/
{
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(GROUP(FE_element))	*element_group_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager,*node_group_manager; 
	struct MANAGER(FE_basis) *fe_basis_manager;
	struct MANAGER(FE_element) *element_manager;

	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct FE_element_selection *element_selection;
	struct FE_node_selection *data_selection,*node_selection;

	struct MANAGER(Computed_field) *computed_field_manager;
	struct LIST(GT_object) *glyph_list;
	/* fields of the rig_nodes, so we know what to clean up, and what to*/
	/*construct draw package with */
	/* for MIXED enum Region_type and hence rig_node groups, elctrode postion fields */
	/*	can be different  eg  a torso with rectangular cart coords and a sock with */
	/* prolate spheroidal coords. Therefore store multiple fields */
	int number_of_electrode_position_fields;
	struct FE_field **electrode_position_fields;
	int number_of_map_electrode_position_fields;
	struct FE_field **map_electrode_position_fields;
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
	struct Computed_field *signal_value_at_time_field;
	struct Computed_field *time_field;
	int number_of_rig_node_groups;
	struct GROUP(FE_node) **rig_node_groups;
	/* info about the last map(s) */
	int number_of_maps;
	struct Map_info **maps_info;
	/* these are for the cmgui graphics window */
	/* flag to record if done "view all" on scene*/
	int viewed_scene; 
	/* should this and below be be in struct Map ?*/
	/*or Map_drawing_information?  */
	struct Colour *background_colour;
	struct Colour *electrode_colour;
	struct Colour *no_interpolation_colour; 
	struct Light *light;	
	struct Light_model *light_model;	
	struct Scene *scene;	
	struct Scene_viewer *scene_viewer;	
	struct User_interface *user_interface;
	struct Graphical_material *map_graphical_material;
	struct Graphical_material *electrode_graphical_material;
	struct Time_keeper *time_keeper;	
	struct Computed_field_package *computed_field_package;
	struct MANAGER(Texture) *texture_manager;
	struct MANAGER(Scene) *scene_manager;
	struct MANAGER(Light_model) *light_model_manager;
	struct MANAGER(Light) *light_manager;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct MANAGER(FE_node) *data_manager;	
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	int access_count;
}; /* struct Unemap_package */

#if defined (UNEMAP_USE_NODES)

PROTOTYPE_OBJECT_FUNCTIONS(Unemap_package);

struct Unemap_package *CREATE(Unemap_package)(
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(FE_basis) *fe_basis_manager,
	struct MANAGER(FE_element) *element_manager,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct FE_node_selection *data_selection,
	struct MANAGER(Computed_field) *computed_field_manager,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct MANAGER(Scene) *scene_manager,
	struct MANAGER(Light_model) *light_model_manager,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(FE_node) *data_manager,
	struct LIST(GT_object) *glyph_list,
	struct Colour *colour);
/*******************************************************************************
LAST MODIFIED : 26 May 2000

DESCRIPTION:
Create a Unemap package, and fill in the managers.
The fields are filed in with set_unemap_package_fields()
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

int set_unemap_package_viewed_scene(struct Unemap_package *package,
	int scene_viewed);
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
sets the viewed_scene flag of the unemap <package> to 1 if <scene_viewed> >0,
0 if <scene_viewed> = 0.
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
gets the map_electrode_position_field in <package>.
==============================================================================*/

int set_unemap_package_map_electrode_position_field(struct Unemap_package *package,
	struct FE_field *map_electrode_position_field);
/*******************************************************************************
LAST MODIFIED : October 19 1999

DESCRIPTION :
Sets the map_electrode_position_field  in <package>.
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

struct GT_object *get_unemap_package_map_torso_arm_labels(
	struct Unemap_package *package,int map_number);
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
gets the map_torso_arm_labels for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
??JW perhaps should maintain a list of GT_objects, cf CMGUI
==============================================================================*/

int set_unemap_package_map_torso_arm_labels(struct Unemap_package *package,
	struct GT_object *torso_arm_labels,int map_number);
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
Sets the torso_arm_labels  for map_info <map_number> in <package>.
Set (and get) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3...
??JW perhaps should maintain a list of GT_objects, cf CMGUI
==============================================================================*/

FE_value get_unemap_package_map_electrode_size(struct Unemap_package *package,
	int map_number);
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
gets the map_electrode_size for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/

int set_unemap_package_map_electrode_size(struct Unemap_package *package,
	FE_value electrode_size,int map_number);
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Sets the electrode_size  for map_info <map_number> in <package>.
Set (and get) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3...
==============================================================================*/

enum Electrodes_option get_unemap_package_map_electrodes_option(
	struct Unemap_package *package,int map_number);
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
gets the map_electrodes_option for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/

int set_unemap_package_map_electrodes_option(struct Unemap_package *package,
	enum Electrodes_option electrodes_option,int map_number);
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
Sets the electrodes_option  for map_info <map_number> in <package>.
Set (and get) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3...
==============================================================================*/

int get_unemap_package_map_colour_electrodes_with_signal(
	struct Unemap_package *package,int map_number);
/*******************************************************************************
LAST MODIFIED : 26 May 2000

DESCRIPTION :
gets the map_colour_electrodes_with_signal for map_info <map_number> in <package>.
get (and set) with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... i.e 
==============================================================================*/

int set_unemap_package_map_colour_electrodes_with_signal(
	struct Unemap_package *package,int colour_electrodes_with_signal,
	int map_number);
/*******************************************************************************
LAST MODIFIED : 26 May 2000

DESCRIPTION :
Sets the colour_electrodes_with_signal  for map_info <map_number> in <package>.
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

int free_unemap_package_map_contours(struct Unemap_package *package,int map_number);
/*******************************************************************************
LAST MODIFIED : 16 May 2000

DESCRIPTION :
Frees the array of map contour GT_element_settings stored in the <package>
<map_number> map_info.
==============================================================================*/

int get_unemap_package_map_contours(struct Unemap_package *package,
	int map_number,int *number_of_contours,
	struct GT_element_settings ***contour_settings);
/*******************************************************************************
LAST MODIFIED : 16 May 2000

DESCRIPTION :
gets the <number_of_contours> and <contour_settings> for map_info <map_number> 
in <package>.
get with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3... Returns -1 on error
==============================================================================*/

int set_unemap_package_map_contours(struct Unemap_package *package,int map_number,
	int number_of_contours,struct GT_element_settings **contour_settings);
/*******************************************************************************
LAST MODIFIED : 16 May 2000

DESCRIPTION :
sets the <number_of_contours> and <contour_settings> for map_info <map_number> 
in <package>.
set with map_number 0,1,2... (an array), but package->number_of_maps
is 1,2,3...
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

int unemap_package_rig_node_group_has_electrodes(struct Unemap_package *package,
	int rig_node_group_number);
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :determines if the <package> rig_node group specified by 
<rig_node_group_number> contains at least one node with a device_type field
set to "ELECTRODE". See also rig_node_has_electrode_defined
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

int free_unemap_package_rig_fields(struct Unemap_package *unemap_package);
/*******************************************************************************
LAST MODIFIED : 17 May 2000

DESCRIPTION :
Frees the <unemap_package> rig's computed and fe fields
==============================================================================*/

int free_unemap_package_time_computed_fields(struct Unemap_package *unemap_package);
/*******************************************************************************
LAST MODIFIED : 4 May 2000

DESCRIPTION :
Frees the time related computed fields (used by the map electrode glyphs) 
stored in the unemap package.
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

struct Colour *get_unemap_package_background_colour(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED :  September 2 1999

DESCRIPTION :
gets the Colour of the unemap package.
==============================================================================*/

int set_unemap_package_background_colour(struct Unemap_package *package,
	struct Colour *background_colour);
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

struct Scene_viewer *get_unemap_package_scene_viewer(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED :  30 May 2000

DESCRIPTION :
gets the Scene_viewer of the unemap package.
==============================================================================*/

int set_unemap_package_scene_viewer(struct Unemap_package *package,
	struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 30 May 2000

DESCRIPTION :
Sets the Scene_viewer of the unemap package.
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

struct Graphical_material *get_unemap_package_map_graphical_material(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED :  September 3 1999

DESCRIPTION :
gets the map_graphical_material of the unemap package.
==============================================================================*/

int set_unemap_package_map_graphical_material(struct Unemap_package *package,
	struct Graphical_material *map_graphical_material);
/*******************************************************************************
LAST MODIFIED : September 3 1999

DESCRIPTION :
Sets the map_graphical_material of the unemap package.
==============================================================================*/

struct Graphical_material *get_unemap_package_electrode_graphical_material(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED :  26 May 2000

DESCRIPTION :
gets the electrode_graphical_material of the unemap package.
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

struct MANAGER(Spectrum) *get_unemap_package_spectrum_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : April 18 2000

DESCRIPTION :
gets the spectrum_manager of the unemap package.
==============================================================================*/

struct Computed_field *get_unemap_package_signal_value_at_time_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/

int set_unemap_package_signal_value_at_time_field(struct Unemap_package *package,
	struct Computed_field *signal_value_at_time_field);
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/

struct Computed_field *get_unemap_package_time_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/

int set_unemap_package_time_field(struct Unemap_package *package,
	struct Computed_field *time_field);
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/

int unemap_package_align_scene(struct Unemap_package *package); /*FOR AJP*/
/*******************************************************************************
LAST MODIFIED : 20 Jun 2000

DESCRIPTION : Aligns the <package> scene so that the largest cardinal dimension 
component of the scene is the scene viewer's up vector, and the viewer is 
looking along the scene's smallest cardinal dimension component towards the 
centre of the scene. Eg if a scene is 100 by 30 by 150 (in x,y,z)
then we'd look along the y axis, with the z axis as the up vector. 

cf Scene_viewer_view_all.
Should be in Scene_viewer? RC doesn't think so.
==============================================================================*/

#endif /* #if defined(UNEMAP_USE_NODES) */
#endif /* !defined (UNEMAP_PACKAGE_H) */
