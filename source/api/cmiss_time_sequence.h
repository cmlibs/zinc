/*******************************************************************************
FILE : cmiss_time_sequence.h

LAST MODIFIED : 11 November 2004

DESCRIPTION :
The public interface to Cmiss_time_sequence.
==============================================================================*/
#ifndef __CMISS_TIME_SEQUENCE_H__
#define __CMISS_TIME_SEQUENCE_H__

#include "general/object.h"
#include "api/cmiss_function_base.h"

/*
Global types
------------
*/

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_time_sequence_package FE_time_sequence_package

struct Cmiss_time_sequence_package;
/*******************************************************************************
LAST MODIFIED : 11 November 2004

DESCRIPTION :
==============================================================================*/

typedef struct Cmiss_time_sequence_package *Cmiss_time_sequence_package_id;

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_time_sequence FE_time_sequence

struct Cmiss_time_sequence;
/*******************************************************************************
LAST MODIFIED : 11 November 2004

DESCRIPTION :
==============================================================================*/

typedef struct Cmiss_time_sequence *Cmiss_time_sequence_id;

/*
Global functions
----------------
*/

Cmiss_time_sequence_package_id CREATE(Cmiss_time_sequence_package)(void);
/*******************************************************************************
LAST MODIFIED : 11 November 2004

DESCRIPTION :
Creates a Cmiss_time_sequence_package.  This object enables the actual time_sequence
objects created with respect to it to reduce storage by sharing identical lists.
==============================================================================*/

Cmiss_time_sequence_id Cmiss_time_sequence_package_get_matching_time_sequence(
	Cmiss_time_sequence_package_id time_sequence_package,
	int number_of_times, Scalar *times);
/*******************************************************************************
LAST MODIFIED : 11 November 2004

DESCRIPTION :
Searches <cmiss_time_sequence_package> for a cmiss_time_sequence which has the time
sequence specified.  If no equivalent cmiss_time_sequence is found one is created 
and returned.
==============================================================================*/
#endif /* __CMISS_TIME_SEQUENCE_H__ */
