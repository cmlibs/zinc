/*******************************************************************************
FILE : rig.h

LAST MODIFIED : 28 July 1999

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
#include "general/geometry.h"

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

struct Auxiliary_properties
/*******************************************************************************
LAST MODIFIED : 27 July 1999

DESCRIPTION :
If <number_of_electrodes> is non-positive then the auxiliary is an external
device measured on a single channel.  Otherwise the auxiliary is a linear
combination of electrodes.
==============================================================================*/
{
	float *electrode_coefficients;
	int number_of_electrodes;
	struct Device **electrodes;
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
LAST MODIFIED : 8 October 1997

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
LAST MODIFIED : 24 November 1993

DESCRIPTION :
???info about channel - frequency, type (AD, DA, DD)
==============================================================================*/
{
	int number;
		/*???temporary */
	float gain,offset;
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
LAST MODIFIED : 27 July 1999

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
	/* user changeable maximum and minimum values */
	float signal_maximum,signal_minimum;
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
LAST MODIFIED : 17 June 1999

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
	/* the name of the file containing the signal data (if appropriate) */
	char *signal_file_name;
#if defined (OLD_CODE)
???DB.  Only read calibration when doing acquisition */
	char *calibration_directory;
#endif /* defined (OLD_CODE) */
}; /* struct Rig */

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

struct Signal_buffer *get_Device_signal_buffer(struct Device *device);
/*******************************************************************************
LAST MODIFIED : 4 August 1999

DESCRIPTION :
Returns the signal buffer used by the <device>.
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
	int number_of_devices);
/*******************************************************************************
LAST MODIFIED : 26 June 1995

DESCRIPTION :
This function allocates memory for a region and initializes all the fields to
the specified values.  It returns a pointer to the created region if successful
and NULL if unsuccessful.
==============================================================================*/

int destroy_Region(struct Region **region);
/*******************************************************************************
LAST MODIFIED : 2 August 1992

DESCRIPTION :
This function frees the memory associated with the fields of <**region>, frees
the memory for <**region> and changes <*region> to NULL.  It does not destroy
the devices in the device list.
==============================================================================*/

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

struct Rig *create_Rig(char *name,enum Monitoring_status monitoring,
	enum Experiment_status experiment,int number_of_devices,
	struct Device **devices,struct Page_list_item *page_list,
	int number_of_regions,struct Region_list_item *region_list,
	struct Region *current_region);
/*******************************************************************************
LAST MODIFIED : 26 June 1995

DESCRIPTION :
This function allocates memory for a rig and initializes the fields to
specified values.  It returns a pointer to the created rig if successful and
NULL if unsuccessful.
???DB.  Allow different regions to have different types ?
==============================================================================*/

struct Rig *create_standard_Rig(char *name,enum Region_type region_type,
	enum Monitoring_status monitoring,enum Experiment_status experiment,
	int number_of_rows,int *electrodes_in_row,int number_of_regions,
	int number_of_auxiliary_inputs,float sock_focus);
/*******************************************************************************
LAST MODIFIED : 27 July 1999

DESCRIPTION :
This function is a specialized version of create_Rig (in rig.c).  It creates a
rig with <number_of_regions> regions with identical electrode layouts.  In each
region the electrodes are equally spaced in <number_of_rows> rows and
<number_of_columns> columns.  There are <number_of_auxiliary_inputs> auxiliary
inputs.  The auxiliaries are single channel inputs (rather than linear 
combinations of electrodes).
==============================================================================*/

int destroy_Rig(struct Rig **rig);
/*******************************************************************************
LAST MODIFIED : 3 May 1992

DESCRIPTION :
This function frees the memory associated with the fields of <**rig>, frees the
memory for <**rig> and changes <*rig> to NULL.
==============================================================================*/

struct Rig *read_configuration(FILE *input_file,enum Rig_file_type file_type,
	enum Region_type rig_type);
/*******************************************************************************
LAST MODIFIED : 27 June 1995

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

int read_configuration_file(char *file_name,void *rig_pointer);
/*******************************************************************************
LAST MODIFIED : 14 May 1992

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

struct Signal_buffer *create_Signal_buffer(enum Signal_value_type value_type,
	int number_of_signals,int number_of_samples,float frequency);
/*******************************************************************************
LAST MODIFIED : 31 August 1993

DESCRIPTION :
This function allocates memory for a signal buffer to hold the specified
<value_type>, <number_of_signals> and <number_of_samples>.  It initializes the
fields to specified values.  It returns a pointer to the created signal buffer
if successful and NULL if unsuccessful.
==============================================================================*/

int destroy_Signal_buffer(struct Signal_buffer **buffer);
/*******************************************************************************
LAST MODIFIED : 31 August 1993

DESCRIPTION :
This function frees the memory associated with the fields of <**buffer>, frees
the memory for <**buffer> and changes <*buffer> to NULL.
==============================================================================*/

int read_signal_file(FILE *input_file,struct Rig **rig_pointer);
/*******************************************************************************
LAST MODIFIED : 1 December 1993

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
#endif /* !defined (RIG_H) */
