/*******************************************************************************
FILE : rig_node.h

LAST MODIFIED : 27 June 2000

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
/*
Global types
------------
*/
#if defined (UNEMAP_USE_NODES)
struct Rig_node_sort
/*******************************************************************************
LAST MODIFIED : 7 August 2000

DESCRIPTION :
So we can sort arrays of nodes, (or rather, arrays of Rig_node_sorts) 
by  read_order or event_time using heapsort or quicksort 
==============================================================================*/
{
	struct FE_node *node;
	int read_order;
	FE_value event_time; /* I think event_time is an FE_value*/
};/* Rig_node_sort */

struct Min_max_iterator
/*******************************************************************************
LAST MODIFIED : 7 August 2000

DESCRIPTION :
Used by get_rig_node_group_signal_min_max_at_time, set_rig_node_signal_min_max etc
to store info about the minimum and maximum signal values at a rig node group at 
time. Not all the fields are not necesarily used simultaneously.
No ACCessing of feilds  as just temp reference for iteration.
==============================================================================*/
{
	FE_value max,min,time;
	int count;
	int started; /*have we started accumulating info yet? */
	struct FE_field *channel_gain_field,*channel_offset_field,*display_start_time_field,
		*display_end_time_field,*signal_minimum_field,*signal_maximum_field,
		*signal_status_field;	
	struct FE_field_component *signal_component;
};
#endif /* defined (UNEMAP_USE_NODES) */

struct Signal_drawing_package
/*******************************************************************************
LAST MODIFIED : 29 July 1999

DESCRIPTION :
The fields of the rig_node which we wish to draw, with draw_signal().
Signal_drawing_package struct usually constructed from the fields stored in a 
unemap_package struct. If a field is set to NULL, then it isn't drawn.
==============================================================================*/
{	
	int access_count;
	/* fields of the rig_nodes */
	/* so we know what fields to get from the nodes*/
	struct FE_field *channel_number_field;
	struct FE_field *device_name_field;
	struct FE_field *device_type_field;	
	struct FE_field *display_end_time_field;
	struct FE_field *display_start_time_field;
	struct FE_field *read_order_field;	
	struct FE_field *channel_gain_field;
	struct FE_field *channel_offset_field; 
	struct FE_field *signal_field;	
	struct FE_field *signal_maximum_field;	
	struct FE_field *signal_minimum_field;
	struct FE_field *signal_status_field;	
}; /* struct Signal_drawing_package */

/*
Global functions
----------------
*/
#if defined (UNEMAP_USE_NODES)
PROTOTYPE_OBJECT_FUNCTIONS(Signal_drawing_package);
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct Min_max_iterator *CREATE(Min_max_iterator)(void);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION:
Create a Min_max_iterator, set fields to NULL/0.
==============================================================================*/

int DESTROY(Min_max_iterator)(struct Min_max_iterator **iterator_address);
/*******************************************************************************
LAST MODIFIED : 8 August 2000 

DESCRIPTION :
Destroy a Min_max_iterator. Don't DEACCESS fields, 'cos never accessed them.
This is only temp references for iteration.
==============================================================================*/

int get_Min_max_iterator_count(struct Min_max_iterator *min_max_iterator, 
	int *count);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the count of Min_max_iterato
==============================================================================*/

int set_Min_max_iterator_count(struct Min_max_iterator *min_max_iterator, 
	int count);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the count of Min_max_iterato
==============================================================================*/

int get_Min_max_iterator_started(struct Min_max_iterator *min_max_iterator, 
	int *started);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the started of Min_max_iterato
==============================================================================*/

int set_Min_max_iterator_started(struct Min_max_iterator *min_max_iterator, 
	int started);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the started of Min_max_iterato
==============================================================================*/

int get_Min_max_iterator_time(struct Min_max_iterator *min_max_iterator, 
	FE_value *time);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the time of Min_max_iterato
==============================================================================*/

int set_Min_max_iterator_time(struct Min_max_iterator *min_max_iterator, FE_value time);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the time of Min_max_iterato
==============================================================================*/

int get_Min_max_iterator_min(struct Min_max_iterator *min_max_iterator, 
	FE_value *min);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the min of Min_max_iterato
==============================================================================*/

int set_Min_max_iterator_min(struct Min_max_iterator *min_max_iterator, FE_value min);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the min of Min_max_iterato
==============================================================================*/


int get_Min_max_iterator_max(struct Min_max_iterator *min_max_iterator, 
	FE_value *max);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the max of Min_max_iterato
==============================================================================*/

int set_Min_max_iterator_max(struct Min_max_iterator *min_max_iterator, FE_value max);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the max of Min_max_iterato
==============================================================================*/

int get_Min_max_iterator_channel_gain_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *channel_gain_field);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the channel_gain_field of Min_max_iterato
==============================================================================*/

int set_Min_max_iterator_channel_gain_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *channel_gain_field);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the channel_gain_field of Min_max_iterato
==============================================================================*/

int get_Min_max_iterator_channel_offset_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *channel_offset_field);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the channel_offset_field of Min_max_iterato
==============================================================================*/

int set_Min_max_iterator_channel_offset_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *channel_offset_field);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the channel_offset_field of Min_max_iterato
==============================================================================*/

int get_Min_max_iterator_display_start_time_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *display_start_time_field);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the display_start_time_field of Min_max_iterato
==============================================================================*/

int set_Min_max_iterator_display_start_time_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *display_start_time_field);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the display_start_time_field of Min_max_iterato
==============================================================================*/

int get_Min_max_iterator_display_end_time_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *display_end_time_field);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the display_end_time_field of Min_max_iterato
==============================================================================*/

int set_Min_max_iterator_display_end_time_field(
	struct Min_max_iterator *min_max_iterator, 
	struct FE_field *display_end_time_field);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the display_end_time_field of Min_max_iterato
==============================================================================*/

int get_Min_max_iterator_signal_minimum_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *signal_minimum_field);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the signal_minimum_field of Min_max_iterato
==============================================================================*/

int set_Min_max_iterator_signal_minimum_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *signal_minimum_field);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the signal_minimum_field of Min_max_iterato
==============================================================================*/

int get_Min_max_iterator_signal_maximum_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *signal_maximum_field);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the signal_maximum_field of Min_max_iterato
==============================================================================*/

int set_Min_max_iterator_signal_maximum_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *signal_maximum_field);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the signal_maximum_field of Min_max_iterato
==============================================================================*/

int get_Min_max_iterator_signal_status_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *signal_status_field);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the signal_status_field of Min_max_iterato
==============================================================================*/

int set_Min_max_iterator_signal_status_field(struct Min_max_iterator *min_max_iterator, 
	struct FE_field *signal_status_field);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the signal_status_field of Min_max_iterato
==============================================================================*/

int get_Min_max_iterator_signal_component(struct Min_max_iterator *min_max_iterator, 
	struct FE_field_component *signal_component);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
gets the signal_component of Min_max_iterato
==============================================================================*/

int set_Min_max_iterator_signal_component(struct Min_max_iterator *min_max_iterator, 
	struct FE_field_component *signal_component);
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
Sets the signal_component of Min_max_iterato
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct Signal_drawing_package *CREATE(Signal_drawing_package)(void);
/*******************************************************************************
LAST MODIFIED :  9 July 1999

DESCRIPTION :
Create a Signal_drawing_package, set all it's fields to NULL.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int DESTROY(Signal_drawing_package)(struct Signal_drawing_package **package_address);
/*******************************************************************************
LAST MODIFIED : 9 July 1999

DESCRIPTION :
Frees the memory for the Signal_drawing_package node and sets <*package_address> to NULL.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_electrode_position_field(
	struct Signal_drawing_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_electrode_position_field(struct Signal_drawing_package *package,
	struct FE_field *electrode_position_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_device_name_field(
	struct Signal_drawing_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_device_name_field(struct Signal_drawing_package *package,
	struct FE_field *device_name_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_device_type_field(
	struct Signal_drawing_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_device_type_field(struct Signal_drawing_package *package,
	struct FE_field *device_type_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_channel_number_field(
	struct Signal_drawing_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the  signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_channel_number_field(struct Signal_drawing_package *package,
	struct FE_field *channel_number_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_display_start_time_field(
	struct Signal_drawing_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the  signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_display_start_time_field(struct Signal_drawing_package *package,
	struct FE_field *display_start_time_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_display_end_time_field(
	struct Signal_drawing_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the  signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_display_end_time_field(struct Signal_drawing_package *package,
	struct FE_field *display_end_time_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_read_order_field(
	struct Signal_drawing_package *package);
/*******************************************************************************
LAST MODIFIED : July 26 2000

DESCRIPTION :
Gets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_read_order_field(struct Signal_drawing_package *package,
	struct FE_field *read_order_field);
/*******************************************************************************
LAST MODIFIED : July 26 2000

DESCRIPTION :
Sets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_signal_field(
	struct Signal_drawing_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_signal_status_field(
	struct Signal_drawing_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_signal_minimum_field(
	struct Signal_drawing_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_signal_field(struct Signal_drawing_package *package,
	struct FE_field *signal_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_signal_status_field(struct Signal_drawing_package *package,
	struct FE_field *signal_status_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_signal_minimum_field(
	struct Signal_drawing_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_signal_minimum_field(struct Signal_drawing_package *package,
	struct FE_field *signal_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_signal_maximum_field(
	struct Signal_drawing_package *package);
/*******************************************************************************
LAST MODIFIED : July 13 1999

DESCRIPTION :
Gets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_signal_maximum_field(struct Signal_drawing_package *package,
	struct FE_field *signal_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_channel_offset_field(
	struct Signal_drawing_package *package);
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
Gets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_channel_offset_field(struct Signal_drawing_package *package,
	struct FE_field *channel_offset_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
struct FE_field *get_Signal_drawing_package_channel_gain_field(
	struct Signal_drawing_package *package);
/*******************************************************************************
LAST MODIFIED : July 12 1999

DESCRIPTION :
Gets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int set_Signal_drawing_package_channel_gain_field(struct Signal_drawing_package *package,
	struct FE_field *channel_gain_field);
/*******************************************************************************
LAST MODIFIED : July 8 1999

DESCRIPTION :
Sets the field of the signal_drawing_package.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

int extract_signal_information(struct FE_node *device_node,
	struct Signal_drawing_package *signal_drawing_package,struct Device *device,
	int signal_number,int first_data,int last_data,
	int *number_of_signals_address,int *number_of_values_address,
	float **times_address,float **values_address,
	enum Event_signal_status **status_address,char **name_address,
	int *highlight_address,float *signal_minimum_address,
	float *signal_maximum_address);
/*******************************************************************************
LAST MODIFIED : 2 August 2000

DESCRIPTION :
Extracts the specified signal information.  The specification arguments are:
- <device_node>, <signal_drawing_package>, <device> identify where the information is
	stored. Either supply a <device>, with <device_node>, <signal_drawing_package> NULL
	to extract info from <device>, or supply a <device_node> and a <signal_drawing_package>
	with <device> = NULL, to extract info from <device_node>.
- <signal_number> specifies which signal (zero indicates all)	
- if device is set (device_node is NULL) <first_data>, <last_data> specify the 
  part of the signal required. If <first_data> is greater than <last_data> then 
  return the whole signal
- if device_node is set (device is NULL)<first_data>, <last_data> not used,
  <device_node> fields display_start(end)_time used instead
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
int file_read_config_FE_node_group(char *file_name,
	struct Unemap_package *unemap_package,struct Rig *rig);
/*******************************************************************************
LAST MODIFIED : 21 July 2000

DESCRIPTION :
Reads  configuration file into  a node group.
cf file_read_FE_node_group() in import_finite_element.c
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */

#if defined (UNEMAP_USE_NODES)
int file_read_signal_FE_node_group(char *file_name,
	struct Unemap_package *unemap_package,struct Rig *rig);
/*******************************************************************************
LAST MODIFIED : 21 July 2000

DESCRIPTION :
Reads  node group(s) from a signal file into rig . Signal file includes the
configuration info.
==============================================================================*/

int get_rig_node_group_map_electrode_position_min_max(struct GROUP(FE_node) *node_group,
	struct FE_field *map_electrode_position_field,FE_value *min_x,FE_value *max_x,
  FE_value *min_y,FE_value *max_y,FE_value *min_z,FE_value *max_z);
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
Finds the min and max coordinates of the  <map_electrode_position_field>
in the <node_group>
==============================================================================*/

int iterative_get_rig_node_accepted_undecided_signal_min_max(struct FE_node *node,
	void *min_max_iterator_void);
/*******************************************************************************
LAST MODIFIED : 7 August 2000

DESCRIPTION :
==============================================================================*/

int get_rig_node_signal_min_max(struct FE_node *node,
	struct FE_field *signal_field,struct FE_field *display_start_time_field,
	struct FE_field *display_end_time_field,struct FE_field *signal_status_field,
	FE_value *min,FE_value *max,enum Event_signal_status *status,int time_range);
/*******************************************************************************
LAST MODIFIED : 7 August 2000

DESCRIPTION : Determines and returns <min> and <max the minimum and maximum
value of <signal_field> at <node>. If <time_range> >0 AND <display_start_time_field>,
<display_end_time_field>  are set, then determines the min, max over this range.
If <time_range> =0, determines the min, max over entire signal range.
If <signal_status_field> and <status> set, return the node signal's 
Event_signal_status in <status>
==============================================================================*/

int iterative_unrange_rig_node_signal(struct FE_node *node,	
	void *min_max_iterator_void);
/*******************************************************************************
LAST MODIFIED : 7 August 2000

DESCRIPTION :
Set the <nodes> signal_minimum,signal_maximum by determining the signals's
actual min and max.
This function is called iteratively by analysis_unrange_all
==============================================================================*/

int iterative_set_rig_node_signal_min_max(struct FE_node *node,	
	void *min_max_iterator_void);
/*******************************************************************************
LAST MODIFIED : 7 August 2000

DESCRIPTION :
Set the <nodes> signal_minimum,signal_maximum from the <min_max_iterator>.
This function is called iteratively by analysis_set_range
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

int rig_node_group_set_map_electrode_position_lambda_r(
	struct Unemap_package *package,struct GROUP(FE_node) *rig_node_group,
	struct Region *region,FE_value sock_lambda,FE_value torso_major_r,
	FE_value torso_minor_r);
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
Sets the node group's nodal map_electrode_postions from the nodal electrode_positions, 
and changes the <rig_node_group>'s map_electrode_postions lambda or r values to 
<value>
==============================================================================*/

int rig_node_group_add_map_electrode_position_field(
	struct Unemap_package *package,struct GROUP(FE_node) *rig_node_group,
	struct FE_field *map_electrode_position_field);
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
Add an electrode_position_field  to the <rig_node_group> nodes,
in addition to the one created with create_config_template_node.

==============================================================================*/

int sort_rig_node_sorts_by_read_order(void *first,void *second);
/*******************************************************************************
LAST MODIFIED : 25 July 2000

DESCRIPTION :
Returns whether the <first> rig_node_sort has a smaller (< 0), the same (0) or a
larger (> 0) read_order than the <second> device.
==============================================================================*/

int sort_rig_node_sorts_by_event_time(void *first,void *second);
/*******************************************************************************
LAST MODIFIED : 26 July 2000

DESCRIPTION :
Returns whether the <first> rig_node_sort has a smaller (< 0), the same (0) or a
larger (> 0) event_time than the <second> device.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES) */
#endif /* #if !defined (RIG_NODE_H) */
