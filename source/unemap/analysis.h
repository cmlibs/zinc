/*******************************************************************************
FILE : analysis.h

LAST MODIFIED : 13 November 2002

DESCRIPTION :
==============================================================================*/
#if !defined (ANALYSIS_H)
#define ANALYSIS_H

#include <stddef.h>
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xlib.h>
#include <X11/Composite.h>
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#include "finite_element/finite_element.h"
#include "unemap/analysis_calculate.h"
#include "unemap/rig.h"
#include "unemap/rig_node.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
enum Datum_type
/*******************************************************************************
LAST MODIFIED : 13 June 1992

DESCRIPTION :
The method for determining the datum from which the event times are calculated.
FIXED = specified by the user
AUTOMATIC = calculated by the system as the first event
==============================================================================*/
{
	AUTOMATIC_DATUM,
	FIXED_DATUM
}; /* enum Datum_type */

enum Edit_order
/*******************************************************************************
LAST MODIFIED : 13 June 1992

DESCRIPTION :
The order in which the events are traversed while editing.
==============================================================================*/
{
	DEVICE_ORDER,
	BEAT_ORDER
}; /* enum Edit_order */

enum Signal_order
/*******************************************************************************
LAST MODIFIED : 13 June 1992

DESCRIPTION :
The order in which the signals are drawn and edited.
==============================================================================*/
{
	EVENT_ORDER,
	CHANNEL_ORDER
}; /* enum Signal_order */

/*
Global functions
----------------
*/
int calculate_device_objective(struct Device *device,
	enum Event_detection_algorithm detection,
	enum Event_detection_objective objective,float *objective_values,
	int number_of_objective_values,int objective_values_step,int average_width);
/*******************************************************************************
LAST MODIFIED : 15 February 2000

DESCRIPTION :
Calculates the specified <objective>/<detection> function for the <device>.
Storing the values in the array (<objective_values> every
<objective_values_step>) provided.
==============================================================================*/

int calculate_device_event_markers(struct Device *device,
	int start_search,int end_search,enum Event_detection_algorithm detection,
	float *objective_values,int number_of_objective_values,
	int objective_values_step,int number_of_events,int threshold_percentage,
	int minimum_separation_milliseconds,float level);
/*******************************************************************************
LAST MODIFIED : 12 September 2000

DESCRIPTION :
Calculate the positions of the event markers for a signal/<device>/<device_node>
based upon the the start and end times, the number of events, the <detection>
algorithm and the <objective_values>.
==============================================================================*/

int analysis_write_signal_file(char *file_name,struct Rig *rig,int datum,
	int potential_time,int start_search_interval,int end_search_interval,
	char calculate_events,enum Event_detection_algorithm detection,
	int event_number,int number_of_events,int minimum_separation,int threshold,
	enum Datum_type datum_type,enum Edit_order edit_order,
	enum Signal_order signal_order,float level,int average_width);
/*******************************************************************************
LAST MODIFIED : 19 November 2000

DESCRIPTION :
This function writes the rig configuration and interval of signal data to the
named file.
==============================================================================*/

int analysis_read_signal_file(char *file_name,struct Rig **rig_address,
	int *analysis_information,int *datum_address,char *calculate_events_address,
	enum Event_detection_algorithm *detection_address,int *event_number_address,
	int *number_of_events_address,int *potential_time_address,
	int *minimum_separation_address,int *threshold_address,
	enum Datum_type *datum_type_address,enum Edit_order *edit_order_address,
	enum Signal_order *signal_order_address,int *start_search_interval_address,
	int *end_search_interval_address,float *level_address,
	int *average_width_address
#if defined (UNEMAP_USE_3D)
	,struct Unemap_package *unemap_package
#endif /* defined (UNEMAP_USE_NODES) */
	);
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :
Reads a signal file and the analysis information.  If there is analysis
information then <*analysis_information> set non-zero and the information is
set.
==============================================================================*/
#endif /* !defined (ANALYSIS_H) */
