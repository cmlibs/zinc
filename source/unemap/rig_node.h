/*******************************************************************************
FILE : rig_node.h

LAST MODIFIED : 3 April 2000

DESCRIPTION :
Essentially the same functionality as rig.h, but using nodes and fields to store
the rig and signal information. rather that special structures.
==============================================================================*/
#if !defined (RIG_NODE_H)
#define RIG_NODE_H

#include <stddef.h>
#include <stdio.h>
#include "finite_element/finite_element.h"
#include "general/geometry.h"
#include "unemap/unemap_package.h"

/*
Global types
------------
*/
struct Draw_package
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
The fields of the rig_node which we wish to draw, with draw_signal().
Draw_package struct usually constructed from the fields stored in a 
unemap_package struct. If a field is set to NULL, then it isn't drawn.
==============================================================================*/
{	
	int access_count;
	/* fields of the rig_nodes */
	/* so we know what fields to get from the nodes*/
	struct FE_field *channel_number_field;
	struct FE_field *device_name_field;
	struct FE_field *device_type_field;
	/* the time series for the signals */	
	struct FE_field *channel_gain_field;
	struct FE_field *channel_offset_field; 
	struct FE_field *signal_field;	
	struct FE_field *signal_maximum_field;	
	struct FE_field *signal_minimum_field;
	struct FE_field *signal_status_field;	
}; /* struct Draw_package */

/*
Global functions
----------------
*/
#if defined (UNEMAP_USE_NODES)
PROTOTYPE_OBJECT_FUNCTIONS(Draw_package);
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct Draw_package *CREATE(Draw_package)(void);
/*******************************************************************************
LAST MODIFIED :  9 July 1999

DESCRIPTION :
Create a Draw_package, set all it's fields to NULL.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int DESTROY(Draw_package)(struct Draw_package **package_address);
/*******************************************************************************
LAST MODIFIED : 9 July 1999

DESCRIPTION :
Frees the memory for the Draw_package node and sets <*package_address> to NULL.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_electrode_position_field(
	struct Draw_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_electrode_position_field(struct Draw_package *package,
	struct FE_field *electrode_position_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the draw package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_device_name_field(
	struct Draw_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_device_name_field(struct Draw_package *package,
	struct FE_field *device_name_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the draw package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_device_type_field(
	struct Draw_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_device_type_field(struct Draw_package *package,
	struct FE_field *device_type_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the draw package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_channel_number_field(
	struct Draw_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_channel_number_field(struct Draw_package *package,
	struct FE_field *channel_number_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the draw package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_signal_field(
	struct Draw_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_signal_status_field(
	struct Draw_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_signal_minimum_field(
	struct Draw_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_signal_field(struct Draw_package *package,
	struct FE_field *signal_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the draw package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_signal_status_field(struct Draw_package *package,
	struct FE_field *signal_status_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the draw package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_signal_minimum_field(
	struct Draw_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_signal_minimum_field(struct Draw_package *package,
	struct FE_field *signal_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the draw package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_signal_maximum_field(
	struct Draw_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_signal_maximum_field(struct Draw_package *package,
	struct FE_field *signal_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the draw package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_channel_offset_field(
	struct Draw_package *package);
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_channel_offset_field(struct Draw_package *package,
	struct FE_field *channel_offset_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Draw_package_channel_gain_field(
	struct Draw_package *package);
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
Gets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Draw_package_channel_gain_field(struct Draw_package *package,
	struct FE_field *channel_gain_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the unemap package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

int extract_signal_information(struct FE_node *device_node,
	struct Draw_package *draw_package,struct Device *device,
	int signal_number,int first_data,int last_data,
	int *number_of_signals_address,int *number_of_values_address,
	float **times_address,float **values_address,
	enum Event_signal_status **status_address,char **name_address,
	int *highlight_address,float *signal_minimum_address,
	float *signal_maximum_address);
/*******************************************************************************
LAST MODIFIED : 13 August 1999

DESCRIPTION :
Extracts the specified signal information.  The specification arguments are:
- <device_node>, <draw_package>, <device> identify where the information is
	stored
- <signal_number> specifies which signal (zero indicates all)
- <first_data>, <last_data> specify the part of the signal required.  If
	<first_data> is greater than <last_data> then return the whole signal
The extraction arguments are:
- <number_of_signals_address> if not NULL, <*number_of_signals_address> is set
	to the number of signals extracted
- <number_of_values_address> if not NULL, <*number_of_values_address> is set to
	the number of values extracted for each signal
- <times_address> if not NULL, an array with number of values entries is
	allocated, filled in with the times and assigned to <*times_address>
- <values_address> if not NULL, an array with (number of signals)*(number of
	values) entries is allocated, filled in with the values and assigned to
	<*values_address>
- <status_address> if not NULL, an array with number of signals entries is
	allocated, filled in with the signal statuses and assigned to
	<*status_address>
- <name_address> if not NULL, a copy of the name is made and assigned to
	<*name_address>
- <highlight_address> if not NULL <*highlight_address> is set to zero if the
	signals are not highlighted and a non-zero if they are highlighted
- <signal_minimum_address> if not NULL <*signal_minimum_address> is set to
	the minimum value to be displayed (not necessarily the minimum of the values)
- <signal_maximum_address> if not NULL <*signal_maximum_address> is set to
	the maximum value to be displayed (not necessarily the maximum of the values)
==============================================================================*/

#if defined (UNEMAP_USE_NODES)
struct GROUP(FE_node) *file_read_config_FE_node_group(char *file_name,
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 15 June 1999

DESCRIPTION :
Reads and returns configuration file into  a node group.
cf file_read_FE_node_group() in import_finite_element.c
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct GROUP(FE_node) *file_read_signal_FE_node_group(char *file_name,
	struct Unemap_package *unemap_package);
/*******************************************************************************
LAST MODIFIED : 15 May 1999

DESCRIPTION :
Reads and returns node group from a signal file. Signal file includes the
configuration info.
==============================================================================*/

int get_rig_node_group_signal_min_max_at_time(struct GROUP(FE_node) *node_group,
	struct FE_field *signal_field,struct FE_field *signal_status_field,FE_value time,
	FE_value *min,FE_value *max);
/*******************************************************************************
LAST MODIFIED : 6 October 1999

DESCRIPTION :
Returns the <min> and <max>  signal values at the rig nodes in the rig_node_group
<node_group>, field <signal_field>, time <time>
==============================================================================*/

int rig_node_group_set_map_electrode_position_lambda_r(int map_number,
	struct Unemap_package *package,FE_value sock_lambda,FE_value torso_r);
/*******************************************************************************
LAST MODIFIED : 14 October 1999

DESCRIPTION :
Sets the node group's nodal map_electrode_postions from the nodal electrode_positions, 
and changes the node group's map_electrode_postions lambda or r values to <value>
==============================================================================*/

int rig_node_group_add_map_electrode_position_field(int map_number,
	struct Unemap_package *package,struct FE_field *map_electrode_position_field);
/*******************************************************************************
LAST MODIFIED : 13 October 1999

DESCRIPTION :
Add an electrode_position_field  to the rig_node_group nodes,
in addition to the one created with create_config_template_node.

==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */
#endif /* #if !defined (RIG_NODE_H) */
