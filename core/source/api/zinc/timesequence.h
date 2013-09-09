/***************************************************************************//**
 * FILE : time_sequence.h
 *
 * The public interface to cmzn_time_sequence.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_TIMESEQUENCE_H__
#define CMZN_TIMESEQUENCE_H__

#include "types/fieldmoduleid.h"
#include "types/timesequenceid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Finds or creates a cmzn_time_sequence in the field module which matches the
 * sequence of times provided.
 * @param field_module  The field module to search or create in.
 * @param number_of_times  The size of the times array.
 * @param times  Array of times. Note later times must not be less than earlier
 * times.
 * @return  The time sequence matching the times array, or NULL if failed.
 */
ZINC_API cmzn_time_sequence_id cmzn_field_module_get_matching_time_sequence(
	cmzn_field_module_id field_module, int number_of_times, const double *times);

/***************************************************************************//**
 * Returns a new reference to the time sequence with reference count
 * incremented. Caller is responsible for destroying the new reference.
 *
 * @param time_sequence  The time sequence to obtain a new reference to.
 * @return  New time sequence reference with incremented reference count.
 */
ZINC_API cmzn_time_sequence_id cmzn_time_sequence_access(
	cmzn_time_sequence_id time_sequence);

/***************************************************************************//**
 * Destroys reference to the time sequence and sets pointer/handle to NULL.
 * Internally this just decrements the reference count.
 *
 * @param time_sequence_address  Address of time sequence reference.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_time_sequence_destroy(cmzn_time_sequence_id *time_sequence_address);

/***************************************************************************//**
 * Sets the time for the given time_index in the time sequence.
 * This can only be done while the time sequence is not in use by other objects.
 * If the sequence does not have as many times as the <time_index> then it will
 * be expanded and the unspecified times also set to <time>.
 * @param time_sequence  The time sequence to modify.
 * @param time_index  The index of the time to set, starting at 0.
 * @param time  The time to set.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_time_sequence_set_value(cmzn_time_sequence_id time_sequence,
	int time_index, double time);

#ifdef __cplusplus
}
#endif

#endif
