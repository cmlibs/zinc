/*******************************************************************************
FILE : rig.h

LAST MODIFIED : 5 September 2002

DESCRIPTION :
Contains data and function descriptions for measurement rigs.

2 May 1996
----------
Added multiple signals for the same device.  This was added so that signals for
different parameter choices can be compared in dfn.
==============================================================================*/
#if !defined (RIG_H)
#define RIG_H

#include <stddef.h>
#include <stdio.h>
#if defined (UNEMAP_USE_NODES)
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#endif /* defined (UNEMAP_USE_NODES) */
#include "finite_element/finite_element.h"
#include "general/geometry.h"

/* Now using */
#define DEVICE_EXPRESSIONS

/*
Global types
------------
*/
enum Region_type
/*******************************************************************************
LAST MODIFIED : 24 June 1995

DESCRIPTION :
MIXED is for backward compatability.  Previously all regions had the same type
(the Rig_type).
==============================================================================*/
{
	MIXED= -1,
	SOCK=0,
	PATCH=1,
	TORSO=2,
	UNKNOWN=3
}; /* enum Region_type */

enum Experiment_status
/*******************************************************************************
LAST MODIFIED : 24 November 1993

DESCRIPTION :
==============================================================================*/
{
	EXPERIMENT_ON,
	EXPERIMENT_OFF
}; /* enum Experiment_status */

enum Monitoring_status
/*******************************************************************************
LAST MODIFIED : 24 November 1993

DESCRIPTION :
==============================================================================*/
{
	MONITORING_ON,
	MONITORING_OFF
}; /* enum Monitoring_status */

enum Signal_analysis_mode 
/*******************************************************************************
LAST MODIFIED : 9 February 2001

DESCRIPTION :
The type of signal analysis being performed.
==============================================================================*/
{
	ELECTRICAL_IMAGING,
	EVENT_DETECTION,
	FREQUENCY_DOMAIN,
	POWER_SPECTRA,
	CROSS_CORRELATION,
	AUTO_CORRELATION,
	FILTERING,
	BEAT_AVERAGING
}; /* enum Signal_analysis_mode */

enum Rig_file_type
/*******************************************************************************
LAST MODIFIED : 24 November 1993

DESCRIPTION :
==============================================================================*/
{
	BINARY,
	TEXT
}; /* enum Rig_file_type */

enum Device_type
/*******************************************************************************
LAST MODIFIED : 27 July 1999

DESCRIPTION :
An ELECTRODE is an input that has a position and is measured on a single
channel.

An AUXILIARY is an input that does not have position.  It can be either
- an external device (eg blood pressure monitor) measured on a single channel
- a linear combination of ELECTRODEs (eg an ECG lead)
==============================================================================*/
{
	ELECTRODE,
	AUXILIARY
}; /* enum Device_type */

struct Position
/*******************************************************************************
LAST MODIFIED : 27 June 1995

DESCRIPTION :
==============================================================================*/
{
	float x,y,z;
}; /* struct Position */

struct Electrode_properties
/*******************************************************************************
LAST MODIFIED : 27 June 1995

DESCRIPTION :
==============================================================================*/
{
	struct Position position;
}; /* struct Electrode_properties */

/*???DB.  Forward declaration */
struct Device;

#if defined (DEVICE_EXPRESSIONS)
/*???DB.  Forward declaration */
struct Device_expression;

enum Device_expression_operator_type
/*******************************************************************************
LAST MODIFIED : 17 June 2002

DESCRIPTION :
==============================================================================*/
{
	DEVICE_EXPRESSION_ADD_OPERATOR,
	DEVICE_EXPRESSION_DIVIDE_OPERATOR,
	DEVICE_EXPRESSION_MULTIPLY_OPERATOR,
	DEVICE_EXPRESSION_SUBTRACT_OPERATOR
}; /* enum Device_expression_operator_type */

struct Device_binary_operation
/*******************************************************************************
LAST MODIFIED : 19 June 2002

DESCRIPTION :
==============================================================================*/
{
	enum Device_expression_operator_type type;
	struct Device_expression *first,*second;
}; /* struct Device_binary_operation */

enum Device_expression_type
/*******************************************************************************
LAST MODIFIED : 19 June 2002

DESCRIPTION :
==============================================================================*/
{
	DEVICE_EXPRESSION_DEVICE,
	DEVICE_EXPRESSION_EXPRESSION,
	DEVICE_EXPRESSION_REAL
}; /* enum Device_expression_type */

struct Device_expression
/*******************************************************************************
LAST MODIFIED : 19 June 2002

DESCRIPTION :
An arithmetic expression involving devices and real coefficients.
==============================================================================*/
{
	enum Device_expression_type type;
	union
	{
		float coefficient;
		struct Device *device;
		struct Device_binary_operation *binary;
	} expression;
}; /* struct Device_expression */

enum Auxiliary_device_type
/*******************************************************************************
LAST MODIFIED : 19 June 2002

DESCRIPTION :
==============================================================================*/
{
	AUXILIARY_DEVICE_CHANNEL,
	AUXILIARY_DEVICE_EXPRESSION,
	AUXILIARY_DEVICE_SUM
}; /* enum Auxiliary_device_type */
#endif /* defined (DEVICE_EXPRESSIONS) */

struct Auxiliary_properties
/*******************************************************************************
LAST MODIFIED : 24 July 2002

DESCRIPTION :
If <number_of_electrodes> is non-positive then the auxiliary is an external
device measured on a single channel.  Otherwise the auxiliary is a linear
combination of electrodes.
???DB.  What about the binary configuration file?
==============================================================================*/
{
#if defined (DEVICE_EXPRESSIONS)
	enum Auxiliary_device_type type;
	union
	{
		struct
		{
			char *device_expression_string;
			struct Device_expression *device_expression;
		} expression;
		struct
		{
#endif /* defined (DEVICE_EXPRESSIONS) */
			float *electrode_coefficients;
			int number_of_electrodes;
			struct Device **electrodes;
#if defined (DEVICE_EXPRESSIONS)
		} sum;
	} combination;
#endif /* defined (DEVICE_EXPRESSIONS) */
}; /* struct Auxiliary_properties */

struct Interpolation_function
/*******************************************************************************
LAST MODIFIED : 26 December 1992

DESCRIPTION :
==============================================================================*/
{
	enum Region_type region_type;
	/* mesh */
	int number_of_columns,number_of_rows;
	float *x_mesh,*y_mesh;
	/* function */
	int number_of_nodes;
	float *dfdx,*dfdy,*d2fdxdy,*f;
	float f_max,f_min;
}; /* struct Interpolation_function */

struct Region
/*******************************************************************************
LAST MODIFIED : 19 January 2001

DESCRIPTION :
==============================================================================*/
{
	/* region name */
	char *name;
	int number;
	enum Region_type type;
	union
	{
		struct
		{
			float focus;
			Linear_transformation *linear_transformation;
		} sock;
	} properties;
	int number_of_devices;
#if defined (UNEMAP_USE_3D)
	struct Unemap_package *unemap_package;
	struct GROUP(FE_node) *rig_node_group; /* all the nodes */	
	struct GROUP(FE_node) *unrejected_node_group;/* the unrejected  nodes */
	struct Map_3d_package *map_3d_package;
	/* these fields stored here as each region can have a different one,*/ 
	/* unlike fields in unemap_package,which are constant per rig  */
	struct FE_field *electrode_position_field;
	/*map_electrode_position_field is of rig node group, not map node group*/
	/* so its here */
	struct FE_field *map_electrode_position_field;
#endif /* defined (UNEMAP_USE_3D) */
}; /* struct Region */

struct Region_list_item
/*******************************************************************************
LAST MODIFIED : 2 August 1992

DESCRIPTION :
==============================================================================*/
{
	/* region */
	struct Region *region;
	/* next item */
	struct Region_list_item *next;
}; /* struct Region_list_item */

struct Device_description
/*******************************************************************************
LAST MODIFIED : 2 August 1992

DESCRIPTION :
==============================================================================*/
{
	char *name;
	enum Device_type type;
	union
	{
		struct Electrode_properties electrode;
		struct Auxiliary_properties auxiliary;
	} properties;
	/* the region to which the device belongs */
	struct Region *region;
}; /* struct Device_description */

struct Channel
/*******************************************************************************
LAST MODIFIED : 29 July 2000

DESCRIPTION :
???info about channel - frequency, type (AD, DA, DD)
???DB.  For data acquisition, would like to be able to specify the starting
	gain, but also need a gain correction (approximately 1) to allow for the
	hardware not being exact.  This will mean a modification to calibrate.dat or
	a new file (have calibrate.dat and the gain corrections inside the unemap
	hardware?), but currently I'm only fixing a problem to do with displaying a
	scrolling signal and save a signal file at the same time.
==============================================================================*/
{
	int number;
		/*???temporary */
	float gain,gain_correction,offset;
}; /* struct Channel */

enum Signal_value_type
/*******************************************************************************
LAST MODIFIED : 31 August 1993

DESCRIPTION :
The storage type for the signal values.
==============================================================================*/
{
	SHORT_INT_VALUE,
	FLOAT_VALUE
}; /* enum Signal_value_type */

struct Signal_buffer
/*******************************************************************************
LAST MODIFIED : 31 August 1993

DESCRIPTION :
Hardware specific.
***Serge*** I see this working as follows, but I am open to suggestions.

I think that this should have 2 blocks of memory, each (maximum length for
acquired signal)/(frequency)*((number of signals)+1) 16 bit integers long,
and some other pieces of information (time since the start of the experiment,
pointers to the first time and the last time for each buffer).

Each block will be a circular buffer.  The UNIMA system will write into one
(while monitoring is in progress).  When acquisition is triggered and the
requested time interval is in the buffer, UNIMA will swap to writing to the
other buffer and another process will write the data to disk.

For each measurement time, the UNIMA system will write the time and the values
into a contiguous part of the buffer, update the first time and last time
pointers, update the time since the start of the experiment and update the
address for the DMA controller so that it writes the next data time's values
"after" this data time's.

Note:
This will have to be accessed by several processes (UNIMA, process for writing
data to disk, process for drawing scrolling display).  I am thinking of using
the UNIX shared memory Inter Process Communication.
==============================================================================*/
{
	int number_of_signals,number_of_samples;
	float frequency;
	int *times;
	enum Signal_value_type value_type;
	/* buffer containing the signals */
	union
	{
		short int *short_int_values;
		float *float_values;
	} signals;
	/* indicies for the start and end of the data in the buffer.  Used during
		acquisition (circular buffer) and analysis (saving sub-intervals) */
	int start,end;
}; /* struct Signal_buffer */

enum Event_signal_status
/*******************************************************************************
LAST MODIFIED : 24 November 1993

DESCRIPTION :
==============================================================================*/
{
	ACCEPTED,
	REJECTED,
	UNDECIDED
}; /* enum Event_signal_status */

struct Event
/*******************************************************************************
LAST MODIFIED : 25 November 1993

DESCRIPTION :
==============================================================================*/
{
	int time;
	int number;
	enum Event_signal_status status;
	struct Event *next,*previous;
}; /* struct Event */

struct Signal
/*******************************************************************************
LAST MODIFIED : 2 May 1996

DESCRIPTION :
This is the type for an object which is a single signal.  The operations that
can be performed on an object of this type are:
a) Query first time
b) Query last time
c) Extract data (times and values) for a specified time segment
???More ?
The idea of this type is to separate the program from the hardware
considerations - the program can think of the signals as separate even if they
are not.
==============================================================================*/
{
	enum Event_signal_status status;
	/* buffer containing all the signals */
	struct Signal_buffer *buffer;
	/* index for this signal in the buffer */
	int index;
	/* a list of events */
	struct Event *first_event;
	/* to allow multiple signals for the same device */
	struct Signal *next;
	/* to "align" with signals for other devices */
	int number;
}; /* struct Signal */

struct Device
/*******************************************************************************
LAST MODIFIED : 23 February 2001

DESCRIPTION :
For an auxiliary that is a linear combination of electrodes, <channel> and
<signal> should be NULL.

???DB.  Is the description really the device ?
==============================================================================*/
{
	/* internal number for the device which is used when reading and writing
		binary configuration files */
	int number;
	/* description of the device */
	struct Device_description *description;
	/* I/O channel the device is connected to */
	struct Channel *channel;
	/* measured signal(s) */
	struct Signal *signal;
	/* user changeable maximum and minimum values  for display */
	float signal_display_maximum,signal_display_minimum;
	/* for display */
	int highlight;
}; /* struct Device */

struct Device_list_item
/*******************************************************************************
LAST MODIFIED : 24 November 1993

DESCRIPTION :
==============================================================================*/
{
	/* device */
	struct Device *device;
	/* previous item */
	struct Device_list_item *previous;
	/* next item */
	struct Device_list_item *next;
}; /* struct Device_list_item */

struct Page
/*******************************************************************************
LAST MODIFIED : 24 November 1993

DESCRIPTION :
==============================================================================*/
{
	/* page name */
	char *name;
	/* list of devices */
	struct Device_list_item *device_list;
}; /* struct Page */

struct Page_list_item
/*******************************************************************************
LAST MODIFIED : 24 November 1993

DESCRIPTION :
==============================================================================*/
{
	/* page */
	struct Page *page;
	/* next item */
	struct Page_list_item *next;
}; /* struct Page_list_item */

struct Rig
/*******************************************************************************
LAST MODIFIED : 26 June 2000

DESCRIPTION :
==============================================================================*/
{
	/* rig properties */
	char *name;
	enum Experiment_status experiment;
	enum Monitoring_status monitoring;
	/* list of devices */
	int number_of_devices;
	struct Device **devices;
	/* list of display pages */
	struct Page_list_item *page_list;
	/* list of regions */
	int number_of_regions;
	struct Region_list_item *region_list;
	struct Region *current_region;
#if defined (UNEMAP_USE_3D)
	struct Unemap_package *unemap_package;
	struct GROUP(FE_node) *all_devices_rig_node_group;
#endif /* defined (UNEMAP_USE_3D) */
	/* the name of the file containing the signal data (if appropriate) */
	char *signal_file_name;
#if defined (OLD_CODE)
???DB.  Only read calibration when doing acquisition */
	char *calibration_directory;
#endif /* defined (OLD_CODE) */
}; /* struct Rig */

struct Electrical_imaging_event
/*******************************************************************************
LAST MODIFIED : 31 May 2001

DESCRIPTION :
Events (times) on the cardiac_intervals_device signal.
Used to generate maps. Merge with Struct Event ??JW
==============================================================================*/
{
	/* time an array index,is_current_event a flag  */
	int time,device_time,is_current_event; 
	struct Electrical_imaging_event *next,*previous;
}; /* Electrical_imaging_event */

/*
Global functions
----------------
*/
struct Device_description *create_Device_description(
	char *name,enum Device_type type,struct Region *region);
/*******************************************************************************
LAST MODIFIED : 2 August 1992

DESCRIPTION :
This function allocates memory for a device description and initializes the
fields to the specified values.  It does not initialize the fields which depend
on the <type>.  It returns a pointer to the created device description if
successful and NULL if unsuccessful.
==============================================================================*/

int destroy_Device_description(struct Device_description **description);
/*******************************************************************************
LAST MODIFIED : 4 May 1992

DESCRIPTION :
This function frees the memory associated with the fields of <**description>,
frees the memory for <**description> and changes <*description> to NULL.
==============================================================================*/

char *get_Device_description_name(struct Device_description *description);
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Returns the name used by the Device_description <description>.
==============================================================================*/

struct Channel *create_Channel(int number,float offset,float gain);
/*******************************************************************************
LAST MODIFIED : 9 September 1992

DESCRIPTION :
This function allocates memory for a channel and initializes the fields to the
specified values.  It returns a pointer to the created channel if successful and
NULL if unsuccessful.
==============================================================================*/

int destroy_Channel(struct Channel **channel);
/*******************************************************************************
LAST MODIFIED : 4 May 1992

DESCRIPTION :
This function frees the memory associated with the fields of <**channel>, frees
the memory for <**channel> and changes <*channel> to NULL.
==============================================================================*/

struct Device *create_Device(int number,struct Device_description *description,
	struct Channel *channel,struct Signal *signal);
/*******************************************************************************
LAST MODIFIED : 8 May 1992

DESCRIPTION :
This function allocates memory for a device and initializes the fields to
specified values.  It returns a pointer to the created measurement device if
successful and NULL if unsuccessful.
==============================================================================*/

int destroy_Device(struct Device **device);
/*******************************************************************************
LAST MODIFIED : 4 May 1992

DESCRIPTION :
This function frees the memory associated with the fields of <**device>, frees
the memory for <**device> and changes <*device> to NULL.
==============================================================================*/

struct Device_expression *parse_device_expression_string(
	char *device_expression_string,struct Device_list_item *device_list,
	int number_of_devices,struct Device **devices);
/*******************************************************************************
LAST MODIFIED : 15 August 2002

DESCRIPTION :
Parses the <device_expression_string> to create a device expression and returns
it.  <device_list> and (<number_of_devices>,<devices>) are exclusive
alternatives for specifying the list of devices to use.
==============================================================================*/

int destroy_Device_expression(struct Device_expression **expression_address);
/*******************************************************************************
LAST MODIFIED : 18 June 2002

DESCRIPTION :
Destroys a device expression.
==============================================================================*/

int calculate_device_channels(struct Device *device,
	int *number_of_channels_address,struct Device ***channel_devices_address);
/*******************************************************************************
LAST MODIFIED : 3 September 2002

DESCRIPTION :
From the <device>, calculate the single channel devices whose values are needed
in order to evaluate the <device>.  <*number_of_channels_address> and
<*channel_devices_address> are "incremented" to include these devices.  The
values passed to <evaluate_device> need to be in the same order as the single
channel devices calculated by this routine.
==============================================================================*/

int evaluate_device(struct Device *device,float **channel_values_address,
	float *result);
/*******************************************************************************
LAST MODIFIED : 24 July 2002

DESCRIPTION :
Evaluates the <device> using the <*channel_values_address>.  The values
should be for and in the order of the channels returned by
<calculate_device_channels>.  Increments <*channel_values_address> as it steps
through.
==============================================================================*/

int evaluate_device_extrema(struct Device *device,float *minimum,
	float *maximum);
/*******************************************************************************
LAST MODIFIED : 5 September 2002

DESCRIPTION :
Evaluates the <device>'s <minimum> and <maximum>.
==============================================================================*/

struct Signal *get_Device_signal(struct Device *device);
/*******************************************************************************
LAST MODIFIED : 24 August 200

DESCRIPTION :
Returns the signal used by the <device>.
==============================================================================*/

struct Device_description *get_Device_description(struct Device *device);
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Returns the Device_description used by the <device>.
==============================================================================*/

struct Signal_buffer *get_Device_signal_buffer(struct Device *device);
/*******************************************************************************
LAST MODIFIED : 4 August 1999

DESCRIPTION :
Returns the signal buffer used by the <device>.
==============================================================================*/

int extract_Device_signal_information(struct Device *device,
	int signal_number,int first_data,int last_data,float **times_address,
	float **values_address,enum Event_signal_status **status_address,
	int *number_of_signals_address,int *number_of_signal_values_address,
	char **name_address,int *highlight_address,float *signal_minimum_address,
	float *signal_maximum_address);
/*******************************************************************************
LAST MODIFIED : 6 September 2002

DESCRIPTION :
Extracts the specified signal information.  The specification arguments are:
- the <device> where the information is stored.
- <signal_number> specifies which signal (zero indicates all)	
- <first_data> and <last_data> specify the part of the signal required.  If
	<first_data> is greater than <last_data> then return the whole signal
If the return_code is nonzero, the extraction arguments are:
- if <times_address> not NULL then
	- if <*times_address> not NULL then
		- <number_of_signal_values_address> should be non NULL
		- on entry, <*number_of_signal_values_address> is the maximum number of
			times that can be extracted into <*times_address>
		else
		- <*times_address> will be allocated for the number of signal values/times
			(either <last_data>-<first_data>+1 or the whole signal) and the times
			extracted
	else
	- no times are extracted
- if <values_address> not NULL then
	- for the extracted values, signal varies fastest ie the values for all
		signals at a particular time are sequential
	- if <*values_address> not NULL then
		- <number_of_signal_values_address> and <number_of_signals_address> should
			both be non NULL
		- on entry, <*number_of_signal_values_address> is the maximum number of
			values that can be extracted for each signal and
			<*number_of_signals_address> is the maximum number of signals that can be
			extracted
		- the stride between values for a particular signal is the entry value of
			<*number_of_signals_address>
		else
		- <*values_address> will be allocated for the number of signals and the
			number of values per signal and the values extracted
	else
	- no values are extracted
- <status_address> if not NULL then
	- if <*status_address> not NULL then
		- <number_of_signals_address> should be non NULL
		- on entry, <*number_of_signals_address> is the maximum number of signals
			that can be extracted
		else
		- an array with number of signals entries is allocated, filled in with the
			signal statuses and assigned to <*status_address>
	else
	- no statuses are extracted
- if <number_of_signals_address> not NULL then
	- if <values_address> or <status_address> are not NULL then
		- <*number_of_signals_address> is the number of signals extracted
		else
		- <*number_of_signals_address> is the number of signals available
- if <number_of_signal_values_address> not NULL then
	- if <times_address> or <values_address> are not NULL then
		- <*number_of_signal_values_address> is the number of values per signal
			extracted
		else
		- <*number_of_signal_values_address> is the number of values per signal
- <name_address> if not NULL, a copy of the name is made and assigned to
	<*name_address>
- <highlight_address> if not NULL <*highlight_address> is set to zero if the
	signals are not highlighted and a non-zero if they are highlighted
- <signal_minimum_address> if not NULL <*signal_minimum_address> is set to
	the minimum value to be displayed (not necessarily the minimum of the values)
- <signal_maximum_address> if not NULL <*signal_maximum_address> is set to
	the maximum value to be displayed (not necessarily the maximum of the values)
==============================================================================*/

struct Device_list_item *create_Device_list_item(struct Device *device,
	struct Device_list_item *previous,struct Device_list_item *next);
/*******************************************************************************
LAST MODIFIED : 13 June 1992

DESCRIPTION :
This function allocates memory for a device list item and initializes all the
fields to specified values.  It returns a pointer to the created device list
item if successful and NULL if unsuccessful.
==============================================================================*/

int destroy_Device_list(struct Device_list_item **list,int destroy_devices);
/*******************************************************************************
LAST MODIFIED : 5 May 1992

DESCRIPTION :
This function recursively frees the memory for the items in the <list>.  If
<destroy_devices> is non-zero then the devices in the list are destroyed.
<**list> is set to NULL.
==============================================================================*/

int number_in_Device_list(struct Device_list_item *list);
/*******************************************************************************
LAST MODIFIED : 7 May 1992

DESCRIPTION :
Counts the number of items in a device list.
==============================================================================*/

int sort_devices_by_event_time(void *first,void *second);
/*******************************************************************************
LAST MODIFIED : 24 November 1993

DESCRIPTION :
Returns whether the <first> device has an earlier (< 0), the same (0) or a
later (> 0) first event time than the <second> device.
==============================================================================*/

int sort_devices_by_number(void *first,void *second);
/*******************************************************************************
LAST MODIFIED : 24 November 1993

DESCRIPTION :
Returns whether the <first> device has a smaller (< 0), the same (0) or a
larger (> 0) number than the <second> device.
==============================================================================*/

struct Page *create_Page(char *name,struct Device_list_item *device_list);
/*******************************************************************************
LAST MODIFIED : 4 May 1992

DESCRIPTION :
This function allocates memory for a page and initializes all the fields to the
speciified values.  It returns a pointer to the created page if successful and
NULL if unsuccessful.
==============================================================================*/

int destroy_Page(struct Page **page);
/*******************************************************************************
LAST MODIFIED : 5 May 1992

DESCRIPTION :
This function frees the memory associated with the fields of <**page>, frees the
memory for <**page> and changes <*page> to NU list item
if successful and NULL if unsuccessful.
==============================================================================*/

int destroy_Page_list(struct Page_list_item **list);
/*******************************************************************************
LAST MODIFIED : 5 May 1992

DESCRIPTION :
This function recursively frees the memory for the items in the <list>.  It
destroys the pages in the list.  <**list> is set to NULL.
==============================================================================*/

int number_in_Page_list(struct Page_list_item *list);
/*******************************************************************************
LAST MODIFIED : 7 May 1992

DESCRIPTION :
Counts the number of items in a page list.
==============================================================================*/

struct Region *create_Region(char *name,enum Region_type type,int number,
	int number_of_devices
#if defined (UNEMAP_USE_3D)
	,struct Unemap_package *unemap_package
#endif /* defined (UNEMAP_USE_3D)*/
							 );
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
This function allocates memory for a region and initializes all the fields to
the specified values.  It returns a pointer to the created region if successful
and NULL if unsuccessful.
==============================================================================*/

int destroy_Region(struct Region **region);
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
This function frees the memory associated with the fields of <**region>, frees
the memory for <**region> and changes <*region> to NULL.  It does not destroy
the devices in the device list.
==============================================================================*/

#if defined (UNEMAP_USE_3D)
struct Unemap_package *get_Region_unemap_package(struct Region *region);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Gets  unemap_package of <region> 
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
struct GROUP(FE_node) *get_Region_rig_node_group(struct Region *region);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Gets  rig_node_group of <region> 
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int set_Region_rig_node_group(struct Region *region,
	struct GROUP(FE_node) *rig_node_group);
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Sets (and accesses) rig_node_group of <region> to <rig_node_group>
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct GROUP(FE_node) *get_Region_unrejected_node_group(struct Region *region);
/*******************************************************************************
LAST MODIFIED : 19 January 2001

DESCRIPTION :
Gets  unrejected_node_group of <region> 
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int set_Region_unrejected_node_group(struct Region *region,
	struct GROUP(FE_node) *unrejected_node_group);
/*******************************************************************************
LAST MODIFIED : 19 January 2001

DESCRIPTION :
Sets (and accesses) unrejected_node_group of <region> to <unrejected_node_group>
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

struct Region_list_item *create_Region_list_item(struct Region *region,
	struct Region_list_item *next);
/*******************************************************************************
LAST MODIFIED : 2 August 1992

DESCRIPTION :
This function allocates memory for a region list item and initializes all the
fields to specified values.  It returns a pointer to the created region list
item if successful and NULL if unsuccessful.
==============================================================================*/

int destroy_Region_list(struct Region_list_item **list);
/*******************************************************************************
LAST MODIFIED : 2 August 1992

DESCRIPTION :
This function recursively frees the memory for the items in the <list>.  It
destroys the regions in the list.  <**list> is set to NULL.
==============================================================================*/

struct Region *get_Region_list_item_region(
	struct Region_list_item *region_list_item);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Gets  Region  of <region_list_item> 
==============================================================================*/

int set_Region_list_item_region(struct Region_list_item *region_list_item,
	struct Region *region);
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Sets region of <region_list_item> 
==============================================================================*/

struct Region_list_item *get_Region_list_item_next(
	struct Region_list_item *region_list_item);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Gets next  of <region_list_item> 
==============================================================================*/

int set_Region_list_item_next(struct Region_list_item *region_list_item,
	struct Region_list_item *next);
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Sets next of <region_list_item> to 
==============================================================================*/

struct Rig *create_Rig(char *name,enum Monitoring_status monitoring,
	enum Experiment_status experiment,int number_of_devices,
	struct Device **devices,struct Page_list_item *page_list,
	int number_of_regions,struct Region_list_item *region_list,
	struct Region *current_region
#if defined (UNEMAP_USE_3D)
	,struct Unemap_package *unemap_package
#endif /* defined (UNEMAP_USE_3D)*/
											 );
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
This function allocates memory for a rig and initializes the fields to
specified values.  It returns a pointer to the created rig if successful and
NULL if unsuccessful.
???DB.  Allow different regions to have different types ?
==============================================================================*/

struct Rig *create_standard_Rig(char *name,enum Region_type region_type,
	enum Monitoring_status monitoring,enum Experiment_status experiment,
	int number_of_rows,int *electrodes_in_row,
	int number_of_regions_and_device_numbering,
	int number_of_auxiliary_inputs,float sock_focus
#if defined (UNEMAP_USE_3D)
	,struct Unemap_package *unemap_package
#endif /* defined (UNEMAP_USE_3D)*/
	);
/*******************************************************************************
LAST MODIFIED : 10 February 2002

DESCRIPTION :
This function is a specialized version of create_Rig (in rig.c).  It creates a
rig with abs(<number_of_regions_and_device_numbering>) regions with identical
electrode layouts.  In each region the electrodes are equally spaced in
<number_of_rows> rows and <electrodes_in_row> columns.  If
<number_of_regions_and_device_numbering> is positive then the device/channel
number offset between electrodes in a region is
<number_of_regions_and_device_numbering>, otherwise the offset is 1.  There are
<number_of_auxiliary_inputs> auxiliary inputs.  The auxiliaries are single
channel inputs (rather than linear combinations of electrodes).
==============================================================================*/

int destroy_Rig(struct Rig **rig);
/*******************************************************************************
LAST MODIFIED : 3 May 1992

DESCRIPTION :
This function frees the memory associated with the fields of <**rig>, frees the
memory for <**rig> and changes <*rig> to NULL.
==============================================================================*/

#if defined (UNEMAP_USE_3D)
struct Unemap_package *get_Rig_unemap_package(struct Rig *rig);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Gets  unemap_package of <rig> 
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct GROUP(FE_node) *get_Rig_all_devices_rig_node_group(struct Rig *rig);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Gets  all_devices_rig_node_group of <rig> 
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D) */

#if defined (UNEMAP_USE_3D)
int set_Rig_all_devices_rig_node_group(struct Rig *rig,
	struct GROUP(FE_node) *rig_node_group);
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Sets (and accesses) all_devices_rig_node_group of <rig> to <rig_node_group>
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

struct Region *get_Rig_current_region(struct Rig *rig);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Gets  current_region of <rig> 
==============================================================================*/

int set_Rig_current_region(struct Rig *rig,struct Region *region);
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Sets  current_region of <rig> to <region>
==============================================================================*/

struct Region_list_item *get_Rig_region_list(struct Rig *rig);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Gets  region_list of <rig> struct Region_list_item *
==============================================================================*/

int set_Rig_region_list(struct Rig *rig,struct Region_list_item *region_list);
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Sets  region_list of <rig> to <region>
==============================================================================*/

#if defined (UNEMAP_USE_3D)
struct Map_3d_package *get_Region_map_3d_package(struct Region *region);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Gets  Map_3d_package  of <region> 
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int set_Region_map_3d_package(struct Region *region,struct Map_3d_package *map_3d_package);
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
sets (and accesses) map_info_3 of <region> to <map_3d_package>
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_Region_electrode_position_field(struct Region *region);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Gets  electrode_position_field  of <region> 
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int set_Region_electrode_position_field(struct Region *region,
	struct FE_field *electrode_position_field);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
sets (and accesses) electrode_position_field of <region> to <electrode_position_field>
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
struct FE_field *get_Region_map_electrode_position_field(struct Region *region);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Gets  map_electrode_position_field  of <region> 
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int set_Region_map_electrode_position_field(struct Region *region,
	struct FE_field *map_electrode_position_field);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
sets (and accesses) map_electrode_position_field of <region> to 
<map_electrode_position_field>
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

struct Rig *read_configuration(FILE *input_file,enum Rig_file_type file_type,
	enum Region_type rig_type
#if defined (UNEMAP_USE_3D)
	,struct Unemap_package *unemap_package
#endif /* defined (UNEMAP_USE_3D)*/
	);
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Assumes that the <input_file> has been opened, the <file_type> (binary or text)
has been determined, the <rig_type> has been determined and the file has been
positioned at the beginning of the rig name.  This function creates a rig and
then initializes the fields as specified in the <input_file>.  It returns a
pointer to the rig if successful and NULL if unsuccessful.
==============================================================================*/

int read_calibration_file(char *file_name,void *rig);
/*******************************************************************************
LAST MODIFIED : 9 September 1992

DESCRIPTION :
This function reads in the characteristics of the acquisition channels for the
<rig> from the specified file.
==============================================================================*/

int read_configuration_file(char *file_name,void *rig_pointer
#if defined (UNEMAP_USE_3D)					
		 ,struct Unemap_package *unemap_package
#endif /* defined (UNEMAP_USE_3D) */
	   );
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
This function reads in a rig configuration from the named file, automatically
determining whether it was written as binary or text.  It creates a rig with
the specified configuration and returns a pointer to it.
==============================================================================*/

int write_configuration(struct Rig *rig,FILE *output_file,
	enum Rig_file_type file_type);
/*******************************************************************************
LAST MODIFIED : 1 March 1993

DESCRIPTION :
Assumes that the <output_file> has been opened with the specified <file_type>
(binary or text).  This function writes a description of the <rig> to the
<output_file>.  It returns a non-zero if successful and zero if unsuccessful.
==============================================================================*/

int write_configuration_file(char *file_name,void *rig_pointer);
/*******************************************************************************
LAST MODIFIED : 14 May 1992

DESCRIPTION :
Writes the configuration of the <rig> to the named file in text format.  It
returns a non-zero if successful and zero if unsuccessful.
==============================================================================*/

struct Signal *create_Signal(int index,struct Signal_buffer *buffer,
	enum Event_signal_status status,int number);
/*******************************************************************************
LAST MODIFIED : 3 May 1996

DESCRIPTION :
==============================================================================*/

int destroy_Signal(struct Signal **signal);
/*******************************************************************************
LAST MODIFIED : 30 May 1992

DESCRIPTION :
==============================================================================*/

int Signal_get_min_max(struct Signal *signal,float *min,float *max,int time_range);
/*******************************************************************************
LAST MODIFIED : 4 August 2000

DESCRIPTION :
Get the minimum and maximun of <signal>, returned in <min> <max>.
If <time_range> is set, determine min, max over the selected time range.
If <time_range> is 0, determine min, max over the signal's entire time range.
==============================================================================*/

struct Signal_buffer *create_Signal_buffer(enum Signal_value_type value_type,
	int number_of_signals,int number_of_samples,float frequency);
/*******************************************************************************
LAST MODIFIED : 31 August 1993

DESCRIPTION :
This function allocates memory for a signal buffer to hold the specified
<value_type>, <number_of_signals> and <number_of_samples>.  It returns a pointer
to the created signal buffer if successful and NULL if unsuccessful.
==============================================================================*/

struct Signal_buffer *reallocate_Signal_buffer(
	struct Signal_buffer *signal_buffer,enum Signal_value_type value_type,
	int number_of_signals,int number_of_samples,float frequency);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
This function reallocates memory for a signal <buffer> to hold the specified
<value_type>, <number_of_signals> and <number_of_samples>.  It returns a pointer
to the created signal buffer if successful and NULL if unsuccessful.
==============================================================================*/

int destroy_Signal_buffer(struct Signal_buffer **buffer);
/*******************************************************************************
LAST MODIFIED : 31 August 1993

DESCRIPTION :
This function frees the memory associated with the fields of <**buffer>, frees
the memory for <**buffer> and changes <*buffer> to NULL.
==============================================================================*/

int read_signal_file(FILE *input_file,struct Rig **rig_pointer
#if defined (UNEMAP_USE_3D)
			,struct Unemap_package *unemap_package
#endif /* defined (UNEMAP_USE_NODES)*/
		 );
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
This function reads in a rig configuration and an interval of signal data from
the <input_file>.
==============================================================================*/

int write_signal_file(FILE *output_file,struct Rig *rig);
/*******************************************************************************
LAST MODIFIED : 1 December 1993

DESCRIPTION :
This function writes the <rig> configuration and interval of signal data to the
<output_file>.
==============================================================================*/

struct Event *create_Event(int time,int number,enum Event_signal_status status,
	struct Event *previous,struct Event *next);
/*******************************************************************************
LAST MODIFIED : 25 November 1993

DESCRIPTION :
This function allocates memeory for an event and initializes the fields to the
specified values.  It returns a pointer to the created event if successful and
NULL if unsuccessful.
==============================================================================*/

int destroy_Event_list(struct Event **first_event);
/*******************************************************************************
LAST MODIFIED : 24 November 1993

DESCRIPTION :
This function frees the memory associated with the event list starting at
<**first_event> and sets <*first_event> to NULL.
==============================================================================*/

int destroy_all_events(struct Rig *rig);
/*******************************************************************************
LAST MODIFIED : 22 June 1992

DESCRIPTION :
==============================================================================*/

struct Electrical_imaging_event *create_Electrical_imaging_event(int time);
/*******************************************************************************
LAST MODIFIED : 31 May 2001

DESCRIPTION : create a Electrical_imaging_event at <time>
==============================================================================*/

int print_Electrical_imaging_event_list(
	struct Electrical_imaging_event *first_list_event);
/*******************************************************************************
LAST MODIFIED :  31 May 2001

DESCRIPTION : Debugging function.
==============================================================================*/

int count_Electrical_imaging_events(
	struct Electrical_imaging_event *first_list_event);
/*******************************************************************************
LAST MODIFIED :  4 July 2001

DESCRIPTION : Count and return the number of evens in the list beginning at 
<first_list_event>
==============================================================================*/

int add_Electrical_imaging_event_to_sorted_list(
	struct Electrical_imaging_event **first_list_event,
	struct Electrical_imaging_event *new_event);
/*******************************************************************************
LAST MODIFIED :  31 May 2001

DESCRIPTION : adds <new_event> to the event list <first_list_event>,
inserting it so that the list is in order of event->time.
If 2 events have the same time, they will both exist in the list, stored 
consecutively. 
==============================================================================*/

int remove_Electrical_imaging_event_from_list(
	struct Electrical_imaging_event **first_list_event,
	struct Electrical_imaging_event *event_to_remove);
/*******************************************************************************
LAST MODIFIED :  14 June 2001

DESCRIPTION : remove the <event_to_remove> from the list of events whose first 
event is <first_list_event>.
==============================================================================*/

int add_Electrical_imaging_event_to_unsorted_list(
	struct Electrical_imaging_event **first_list_event,
	struct Electrical_imaging_event *new_event);
/*******************************************************************************
LAST MODIFIED : 31 May 2001

DESCRIPTION :adds <new_event> to the end of the event list 
<first_list_event>. See also add_Electrical_imaging_event_to_sorted_list
==============================================================================*/

int destroy_Electrical_imaging_event_list(
	struct Electrical_imaging_event **first_event);
/*******************************************************************************
LAST MODIFIED : 31 May 2001

DESCRIPTION :
This function frees the memory associated with the event list starting at
<**first_event> and sets <*first_event> to NULL.
==============================================================================*/


#endif /* !defined (RIG_H) */
