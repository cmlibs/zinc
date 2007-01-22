/*******************************************************************************
FILE : cmiss_time_sequence.c

LAST MODIFIED : 11 November 2004

DESCRIPTION :
The public interface to the Cmiss_finite_elements.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
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

int Cmiss_time_sequence_set_value(
	struct Cmiss_time_sequence *time_sequence, int time_index, double time)
/*******************************************************************************
LAST MODIFIED : 18 November 2004

DESCRIPTION :
Sets the <time> for the given <time_index> in the <time_sequence>.  This 
should only be done for unmanaged time sequences (as otherwise this sequence
may be shared by many other objects which are not expecting changes).
If the sequence does not have as many times as the <time_index> then it will
be expanded and the unspecified times also set to <time>.
==============================================================================*/
{
	return FE_time_sequence_set_time_and_index(time_sequence, time_index, time);
}
