/*******************************************************************************
FILE : cmiss_time_sequence.c

LAST MODIFIED : 11 November 2004

DESCRIPTION :
The public interface to the Cmiss_finite_elements.
==============================================================================*/
#include <stdarg.h>
#include "api/cmiss_time_sequence.h"
#include "general/debug.h"
#include "finite_element/finite_element.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

Cmiss_time_sequence_id Cmiss_time_sequence_package_get_matching_time_sequence(
	Cmiss_time_sequence_package_id time_sequence_package,
	int number_of_times, Scalar *times)
/*******************************************************************************
LAST MODIFIED : 11 November 2004

DESCRIPTION :
Searches <cmiss_time_sequence_package> for a cmiss_time_sequence which has the time
sequence specified.  If no equivalent cmiss_time_sequence is found one is created 
and returned.
==============================================================================*/
{
	FE_value *time_values;
	int i;
	struct FE_time_sequence *time_sequence;

	ENTER(Cmiss_time_sequence_package_get_matching_time_sequence);

	if (ALLOCATE(time_values, FE_value, number_of_times))
	{
		for (i = 0 ; i < number_of_times ; i++)
		{
			time_values[i] = times[i];
		}
		time_sequence = get_FE_time_sequence_matching_time_series(
			time_sequence_package, number_of_times, time_values);
		DEALLOCATE(time_values);
	}
	LEAVE;

	return (time_sequence);
} /* Cmiss_time_sequence_package_get_matching_time_sequence */
