/*******************************************************************************
FILE : finite_element.h

LAST MODIFIED : 27 November 2002

DESCRIPTION :
Representing time in finite elements.
==============================================================================*/
#if !defined (FINITE_ELEMENT_TIME_H)
#define FINITE_ELEMENT_TIME_H

#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"

/*
Global types
------------
*/

enum FE_time_version_type
/******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
==============================================================================*/
{
	FE_TIME_SERIES
};

struct FE_time;
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
struct FE_time is private.
==============================================================================*/

struct FE_time_version;
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
struct FE_time_version is private.
==============================================================================*/

DECLARE_LIST_TYPES(FE_time_version);

DECLARE_MANAGER_TYPES(FE_time_version);

/*
Global functions
----------------
*/
struct FE_time *CREATE(FE_time)(void);
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Creates FE_time.
==============================================================================*/

int DESTROY(FE_time)(struct FE_time **fe_time_address);
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Frees memory/deaccess objects in FE_time at <*field_address>.
==============================================================================*/

struct FE_time_version *CREATE(FE_time_version)(void);
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Creates a basic FE_time_version.
==============================================================================*/

int DESTROY(FE_time_version)(struct FE_time_version **fe_time_version_address);
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Frees memory/deaccess objects in FE_time_version at <*field_address>.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(FE_time);

PROTOTYPE_OBJECT_FUNCTIONS(FE_time_version);

PROTOTYPE_LIST_FUNCTIONS(FE_time_version);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(FE_time_version,name,char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(FE_time_version,name,char *);
PROTOTYPE_MANAGER_FUNCTIONS(FE_time_version);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(FE_time_version,name,char *);

struct FE_time_version *get_FE_time_version_matching_FE_time_version(
	struct FE_time *fe_time, struct FE_time_version *source_fe_time_version);
/*******************************************************************************
LAST MODIFIED : 27 November 2002

DESCRIPTION :
Searches <fe_time> for a FE_time_version matching <source_fe_time_version>.
If no equivalent fe_time_version is found one is created in <fe_time> and
returned.
==============================================================================*/

struct FE_time_version *get_FE_time_version_matching_time_series(
	struct FE_time *fe_time, int number_of_times, FE_value *times);
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Searches <fe_time> for a fe_time_version which has the time series specified. 
If no equivalent fe_time_version is found one is created and returned.
==============================================================================*/

struct FE_time_version *get_FE_time_version_merging_two_time_series(
	struct FE_time *fe_time, struct FE_time_version *time_version_one,
	struct FE_time_version *time_version_two);
/*******************************************************************************
LAST MODIFIED : 20 November 2001

DESCRIPTION :
Searches <fe_time> for a fe_time_version which has the list of times formed
by merging the two time_versions supplied.
==============================================================================*/

int FE_time_version_get_number_of_times(
	struct FE_time_version *fe_time_version);
/*******************************************************************************
LAST MODIFIED : 15 November 2001

DESCRIPTION :
Returns the number of times that a particular FE_time_version references to.
==============================================================================*/

int FE_time_version_get_index_for_time(
	struct FE_time_version *fe_time_version, FE_value time, int *time_index);
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
Returns the integer <time_index> into the time array contained in this version 
that corresponds to the <time>.  Returns 0 if that exact time is not found
and 1 if it is.
==============================================================================*/

int FE_time_version_get_time_for_index(
	struct FE_time_version *fe_time_version, int time_index, FE_value *time);
/*******************************************************************************
LAST MODIFIED : 15 November 2001

DESCRIPTION :
If the <time_index> is valid returns the corresponding <time>.
==============================================================================*/

int FE_time_version_get_interpolation_for_time(
	struct FE_time_version *fe_time_version, FE_value time, int *time_index_one,
	int *time_index_two, FE_value *xi);
/*******************************************************************************
LAST MODIFIED : 20 November 2001

DESCRIPTION :
Returns the two integers <time_index_one> and <time_index_two> which index into
the time array bracketing the supplied <time>, the <xi> value is set between 0
and 1 to indicate what fraction of the way between <time_index_one> and 
<time_index_two> the value is found.  Returns 0 if time is outside the range
of the time index array.
==============================================================================*/

int FE_time_has_FE_time_version(struct FE_time *fe_time,
	struct FE_time_version *fe_time_version);
/*******************************************************************************
LAST MODIFIED : 15 October 2002

DESCRIPTION :
Returns true if <fe_time> contains the <fe_time_version>.
==============================================================================*/

#endif /* !defined (FINITE_ELEMENT_TIME_H) */
