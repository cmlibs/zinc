/*******************************************************************************
FILE : analysis_calculate.h

LAST MODIFIED : 20 July 2004

DESCRIPTION :
The routines for calculating event times.  Separated out and without structures
so that cm can use them.
==============================================================================*/
#if !defined (ANALYSIS_CALCULATE_H)
#define ANALYSIS_CALCULATE_H

/*
Module types
------------
*/
enum Event_detection_algorithm
/*******************************************************************************
LAST MODIFIED : 8 December 1999

DESCRIPTION :
The algorithm used for detecting an event from a signal.
EDA_INTERVAL = divide the search interval into a user specified number of 
	sub-intervals and select the maximum objective within each sub-interval
EDA_LEVEL = find the first time in the search interval at which the absolute
	value of the signal exceeds the user specified level
EDA_THRESHOLD = select all times within the search interval whose objectives
	exceed a user specified percentage of the maximum objective for the search
	interval, subject to user specifed minimum event separation.
NB.  New algorithms need to be added to the end because the detection algorithm
	is written to the signals file (when saving analysis)
==============================================================================*/
{
	EDA_INTERVAL,
	EDA_THRESHOLD,
	EDA_LEVEL
}; /* enum Event_detection_algorithm */

enum Event_detection_objective
/*******************************************************************************
LAST MODIFIED :  April 2001

DESCRIPTION :
The objective used when detecting an event.
==============================================================================*/
{
	ABSOLUTE_SLOPE,
	NEGATIVE_SLOPE,
	POSITIVE_SLOPE,
	ABSOLUTE_VALUE,
	NEGATIVE_VALUE,
	POSITIVE_VALUE
}; /* enum Event_detection_objective */

/*
Module functions
----------------
*/
int calculate_time_series_objective(enum Event_detection_algorithm detection,
  enum Event_detection_objective objective,int average_width,float gain,
	float offset,int number_of_objective_values,int objective_values_step,
	float *objective_values);
/*******************************************************************************
LAST MODIFIED : 6 March 2002

DESCRIPTION :
Calculates the specified <objective>/<detection> function for the time
series initially stored in <objective_values>.  Storing the values in the array
(<objective_values> every <objective_values_step>) provided.  <objective_values>
is assumed to have storage for at least <number_of_objective_values>*
<objective_values_step> values.

Split from function calculate_device_objective on 19 February 2002
==============================================================================*/

int calculate_time_series_event_markers(int start_search,int end_search,
  enum Event_detection_algorithm detection,float *objective_values,
	int number_of_objective_values,int objective_values_step,
	int number_of_interval_events,int *interval_divisions,
	int threshold_percentage,int minimum_separation_milliseconds,float level,
	float frequency,int *number_of_events_address,int **events_address);
/*******************************************************************************
LAST MODIFIED : 20 July 2004

DESCRIPTION :
Calculate the event times for a signal (<objective_values>) based upon the the
<start_search> and <end_search> times, and the <detection> algorithm.
<objective_values> is assumed to have storage for at least
<number_of_objective_values>*<objective_values_step> values.  Allocates storage
for <*event_address>.

<interval_divisions> are the <number_of_interval_events>-1 divisions between the
sub-intervals.  If missing then sub-intervals are all the same length.

Split from the function calculate_device_event_markers on 19 February 2002
==============================================================================*/
#endif /* !defined (ANALYSIS_CALCULATE_H) */
