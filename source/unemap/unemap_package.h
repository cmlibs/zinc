/*******************************************************************************
FILE : unemap_package.h

LAST MODIFIED : 29 April 2002

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
#include "unemap/mapping.h"

/*
Global types
------------
*/

struct Unemap_package
/* Still need structure, for (NULL) function parameters, */ 
/*even when UNEMAP_USE_NODE not defined*/
/*******************************************************************************
LAST MODIFIED : 31 August 2000

DESCRIPTION :
Stores information needed to construct rig_node element,nodes, fields,
and map element,nodes, fields. This information is also uses to clean up the 
element,nodes, fields when they are no longer required.
==============================================================================*/
{
	struct MANAGER(FE_field) *fe_field_manager;
	struct FE_time *fe_time;
	struct MANAGER(GROUP(FE_element))	*element_group_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(FE_node) *data_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager,*node_group_manager; 
	struct MANAGER(FE_basis) *fe_basis_manager;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct FE_node_selection *node_selection;
	/* fields of the rig_nodes, so we know what to clean up, and what to*/
	/*construct draw package with */	
	struct FE_field *channel_gain_field;
	struct FE_field *channel_number_field;
	struct FE_field *channel_offset_field;
	struct FE_field *device_name_field;
	struct FE_field *device_type_field;
#if defined (UNEMAP_USE_NODES) 
	struct FE_field *display_end_time_field;
	struct FE_field *display_start_time_field;
	struct FE_field *highlight_field;
#endif /* defined (UNEMAP_USE_NODES) */	
	/*store as a string rather than node group AND element group AND data group, etc*/
	/* also used as a flag to determine if the default_torso has been loaded */
	char *default_torso_name;
	/* map_fit_field also in map_3d_package. Need to store here between creation*/
	/* of map_fit_field and creation of map_3d_package. ??JW Remove from map_3d_package? */
	struct FE_field *map_fit_field; 
	struct FE_field *delaunay_signal_field;  
	struct FE_field *read_order_field;
	struct FE_field *signal_field;
	struct FE_field *signal_minimum_field;
	struct FE_field *signal_maximum_field;
	struct FE_field *signal_status_field;
	struct Computed_field *signal_value_at_time_field;
	struct Computed_field *offset_signal_value_at_time_field;
	struct Computed_field *scaled_offset_signal_value_at_time_field;
	struct Time_object *potential_time_object;
	int access_count;
}; /* struct Unemap_package */

#if defined (UNEMAP_USE_3D)

PROTOTYPE_OBJECT_FUNCTIONS(Unemap_package);

struct Unemap_package *CREATE(Unemap_package)(
	struct MANAGER(FE_field) *fe_field_manager, struct FE_time *fe_time,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(FE_node) *data_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(FE_basis) *fe_basis_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(Computed_field) *computed_field_manager,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct FE_node_selection *node_selection);
/*******************************************************************************
LAST MODIFIED : 31 August 2000

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
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
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
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
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
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
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
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_unemap_package_display_start_time_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/

int set_unemap_package_display_start_time_field(struct Unemap_package *package,
	struct FE_field *display_start_time_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES)*/

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_unemap_package_display_end_time_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/

int set_unemap_package_display_end_time_field(struct Unemap_package *package,
	struct FE_field *display_end_time_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_unemap_package_read_order_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/

int set_unemap_package_read_order_field(struct Unemap_package *package,
	struct FE_field *read_order_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_unemap_package_map_fit_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/

int set_unemap_package_map_fit_field(struct Unemap_package *package,
	struct FE_field *read_order_field);
/*******************************************************************************
LAST MODIFIED :  6 October 2000

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_unemap_package_delaunay_signal_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 8 December 2000

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/

int set_unemap_package_delaunay_signal_field(struct Unemap_package *package,
	struct FE_field *read_order_field);
/*******************************************************************************
LAST MODIFIED :  8 December 2000

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_unemap_package_highlight_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/

int set_unemap_package_highlight_field(struct Unemap_package *package,
	struct FE_field *highlight_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
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
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
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
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
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
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
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
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
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
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)

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
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct Time_object *get_unemap_package_potential_time_object(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 25 January 2002

DESCRIPTION :
Get the potential time object.
==============================================================================*/

int set_unemap_package_potential_time_object(struct Unemap_package *package,
	struct Time_object *potential_time_object);
/*******************************************************************************
LAST MODIFIED : 25 January 2002

DESCRIPTION :
Sets the potential time object.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct MANAGER(FE_field) *get_unemap_package_FE_field_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct FE_time *get_unemap_package_FE_time(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 15 November 2001

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct MANAGER(Computed_field) *get_unemap_package_Computed_field_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 3 September 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct FE_node_selection *get_unemap_package_FE_node_selection(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 31 August 2000

DESCRIPTION :
gets a FE_node_selection of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct MANAGER(GROUP(FE_element)) *get_unemap_package_element_group_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct MANAGER(FE_node) *get_unemap_package_node_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct MANAGER(FE_node) *get_unemap_package_data_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct MANAGER(FE_element) *get_unemap_package_element_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct MANAGER(Interactive_tool) *get_unemap_package_interactive_tool_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 4 September 2000

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct MANAGER(FE_basis) *get_unemap_package_basis_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct MANAGER(GROUP(FE_node)) *get_unemap_package_data_group_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct MANAGER(GROUP(FE_node)) *get_unemap_package_node_group_manager(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
gets a manager of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int unemap_package_rig_node_group_has_electrodes(struct Unemap_package *package,
	struct GROUP(FE_node) *rig_node_group);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :determines if the  <rig_node group> 
 contains at least one node with a device_type field
set to "ELECTRODE". See also rig_node_has_electrode_defined
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int free_unemap_package_time_computed_fields(struct Unemap_package *unemap_package);
/*******************************************************************************
LAST MODIFIED : 4 May 2000

DESCRIPTION :
Frees the time related computed fields (used by the map electrode glyphs) 
stored in the unemap package. Also frees any associated fe_fields
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int free_unemap_package_rig_fields(struct Unemap_package *unemap_package);
/*******************************************************************************
LAST MODIFIED : 17 May 2000

DESCRIPTION :
Frees the <unemap_package> rig's computed and fe fields
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int free_unemap_package_rig_node_group_glyphs(
	struct Map_drawing_information *drawing_information,
	struct Unemap_package *package,
	struct GROUP(FE_node) *rig_node_group);
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
Frees up any glyphs used by the nodes in the rig_node_group
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int free_unemap_package_rig_node_group(struct Unemap_package *package,	
	struct GROUP(FE_node) **rig_node_group);
/*******************************************************************************
LAST MODIFIED : 17 July 2000 

DESCRIPTION :Frees the node, element and data groups of <rig_node_group>
Note: DOESN't free the glyphs of the node group. See 
free_unemap_package_rig_node_group_glyphs for this
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct Computed_field *get_unemap_package_signal_value_at_time_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int set_unemap_package_signal_value_at_time_field(struct Unemap_package *package,
	struct Computed_field *signal_value_at_time_field);
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct Computed_field *get_unemap_package_offset_signal_value_at_time_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int set_unemap_package_offset_signal_value_at_time_field(struct Unemap_package *package,
	struct Computed_field *offset_signal_value_at_time_field);
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct Computed_field *get_unemap_package_scaled_offset_signal_value_at_time_field(
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
gets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int set_unemap_package_scaled_offset_signal_value_at_time_field(
	struct Unemap_package *package,
	struct Computed_field *scaled_offset_signal_value_at_time_field);
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
char *get_unemap_package_default_torso_name(	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 3 November 2000

DESCRIPTION :
gets the <default_torso_name> of the unemap_package
==============================================================================*/

int set_unemap_package_default_torso_name(struct Unemap_package *package,
	char *default_torso_name);
/*******************************************************************************
LAST MODIFIED : 3 November 2000

DESCRIPTION :
sets the <default_torso_name> of the unemap_package
==============================================================================*/
#endif /* #if defined(UNEMAP_USE_3D) */

#endif /* !defined (UNEMAP_PACKAGE_H) */
