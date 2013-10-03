/**
 * FILE : timenotifier.h
 *
 * The public interface to time notifier object which maintains conditions for
 * notification of time changes from a timekeeper.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_TIMENOTIFIER_H__
#define CMZN_TIMENOTIFIER_H__

#include "types/timeid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The type used for time notifier callback. It is a pointer to a function which
 * takes the same arguments.
 *
 * @param cmzn_timenotifier_id  Handle to time notifier.
 * @param current_time  Time in the time notifier when the callback is being
 *    triggered by the time notifier.
 * @param User_data  any data user want to pass into the callback function.
 * @return  return one if such the callback function
 *    has been called successfully otherwise 0.
 */
typedef int (*cmzn_timenotifier_callback)(cmzn_timenotifier_id timenotifier,
	double current_time, void *user_data);

/**
 * Access the time notifier, increase the access count of the time notifier
 * by one.
 *
 * @param timenotifier  handle to the "to be access" zinc timenotifier.
 * @return  handle to timenotifier if successfully access timenotifier.
 */
ZINC_API cmzn_timenotifier_id cmzn_timenotifier_access(
	cmzn_timenotifier_id timenotifier);

/**
 * Destroys this reference to the time notifier (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param timenotifier_address  The address to the handle to time notifier
 * @return  Status CMZN_OK if successfully destroy(deaccess) the time notifier,
 * any other value on failure.
 */
ZINC_API int cmzn_timenotifier_destroy(cmzn_timenotifier_id *timenotifier_address);

/**
 * Adds a callback routine which is called whenever the time given to the
 * time notifier has been changed.
 *
 * @param timenotifier  Handle to time notifier.
 * @param cmzn_timenotifier_callback  callback function to be set.
 * @param user_data  Data to be past into the callback routine.
 * @return  Status CMZN_OK if successfully add callback,
 * any other value on failure.
 */
ZINC_API int cmzn_timenotifier_add_callback(cmzn_timenotifier_id timenotifier,
	cmzn_timenotifier_callback callback, void *user_data);

/**
 * Remove a callback routine which has been added to the time notifier before.
 *
 * @param timenotifier  Handle to time notifier.
 * @param cmzn_timenotifier_callback  callback function to be .
 * @param user_data  Data that was added to the callback.
 * @return  Status CMZN_OK if successfully remove callback, any other value
 * on failure.
 */
ZINC_API int cmzn_timenotifier_remove_callback(cmzn_timenotifier_id timenotifier,
  cmzn_timenotifier_callback callback, void *user_data);

/**
 * Gets the current time from the time notifier.
 *
 * @param timenotifier  Handle to time notifier.
 * @return  Current time or 0 if invalid argument.
 */
ZINC_API double cmzn_timenotifier_get_time(cmzn_timenotifier_id timenotifier);

/**
 * This controls the rate which the time depedent object is called back.
 * The default value is 10 which means time notifier will receive 10 callbacks
 * per unit of time in the time keeper.
 * i.e. If the speed of time keeper is set to be 1 and the update frequency of
 * time notifier is set to be 10, the actual interval between each callbacks is:
 * (1/speed of time keeper)/(udpate frequency) which is 0.1s.
 * Note that the time notifier does not promised to receive callback exactly
 * 0.1s after the previous callback.
 *
 * @param timenotifier  Handle to time notifier.
 * @param frequency  The number of times which time notifier will receive
 *    callback per unit of time in the time keeper.
 * @return  Status CMZN_OK if successfully set the update frequency to the value
 * provided, any other value on failure.
 */
ZINC_API int cmzn_timenotifier_regular_set_frequency(cmzn_timenotifier_id timenotifier,
	double frequency);

/**
 * This controls the exact time which the time notifier receive callbacks.
 * Time offset will set the notifier to receive callback when
 * time_offset + original callback time is reached. i.e
 *
 * @param timenotifier  Handle to time notifier.
 * @param offset  This set the time that notifier will receive callback.
 * @return  Status CMZN_OK if successfully set the update frequency to the value
 * provided, any other value on failure.
 */
ZINC_API int cmzn_timenotifier_regular_set_offset(cmzn_timenotifier_id timenotifier,
	double time_offset);

#ifdef __cplusplus
}
#endif

#endif
