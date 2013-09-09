/*******************************************************************************
FILE : timekeeper.h

LAST MODIFIED : 2 Mar 2009

DESCRIPTION :
The public interface of cmzn_time_keeper which defines a relationship between
a bunch of time notifiers, keeps them in sync and allows control such as play,
rewind and fast forward.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_TIMEKEEPER_H__
#define CMZN_TIMEKEEPER_H__

#include "types/timeid.h"
#include "types/timekeeperid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

enum cmzn_time_keeper_attribute
{
	CMZN_TIME_KEEPER_ATTRIBUTE_INVALID = 0,
	CMZN_TIME_KEEPER_ATTRIBUTE_TIME = 1,
	/*!< Current time of the time keeper. This is a real number attribute.
	 */
	CMZN_TIME_KEEPER_ATTRIBUTE_MINIMUM_TIME = 2,
	/*!< Minimum allowed time of the time keeper. The time keeper will not
	 * allow any time lower then the minimum time specified by this function.
	 * This is a real number attribute.
	 */
	CMZN_TIME_KEEPER_ATTRIBUTE_MAXIMUM_TIME = 3
	/*!< Maximum allowed time of the time keeper. The time keeper will not allow
	 * any time larger than the value set by this function.
	 * This is a real number attribute.
	 */
};

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum cmzn_time_keeper_attribute cmzn_time_keeper_attribute_enum_from_string(
	const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param mode  attribute to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_time_keeper_attribute_enum_to_string(
	enum cmzn_time_keeper_attribute attribute);

/***************************************************************************//**
 * Access the time_keeper, increase the access count of the time keeper by one.
 *
 * @param time_keeper  handle to the "to be access" zinc time_keeper.
 * @return  handle to time_keeper if successfully access time_keeper.
 */
ZINC_API cmzn_time_keeper_id cmzn_time_keeper_access(cmzn_time_keeper_id time_keeper);

/***************************************************************************//**
 * Destroys this reference to the time keeper (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param time_keeper_address  The address to the handle to time keeper
 * @return  Status CMZN_OK if successfully destroy the time keeper,
 * any other value on failure.
 */
ZINC_API int cmzn_time_keeper_destroy(cmzn_time_keeper_id *time_keeper_address);

/***************************************************************************//**
 * Get a real value of an attribute of the time keeper.
 *
 * @param time_keeper  Handle to the zinc time_keeper.
 * @param attribute  The identifier of the real attribute to get.
 * @return  Value of the attribute.
 */
ZINC_API double cmzn_time_keeper_get_attribute_real(cmzn_time_keeper_id time_keeper,
	enum cmzn_time_keeper_attribute attribute);

/***************************************************************************//**
 * Set a real value for an attribute of the time_keeper.
 *
 * @param time_keeper  Handle to the zinc time_keeper.
 * @param attribute  The identifier of the real attribute to set.
 * @param value  The new value for the attribute.
 * @return  Status CMZN_OK if attribute successfully set, any other value if
 * failed or attribute not valid or unable to be set for this time_keeper object.
 */
ZINC_API int cmzn_time_keeper_set_attribute_real(cmzn_time_keeper_id time_keeper,
	enum cmzn_time_keeper_attribute attribute, double value);

/***************************************************************************//**
 * Create and returns a time notifier with regular update time in time keeper.
 * The returned time notifier will automatically be added to the time keeper.
 *
 * @param update_frequency  The number of times which time notifier will receive
 *    callback per unit of time in the time keeper.
 * @param time_offset  This value will set the exact time the notification
 *    happenes and allow setting the callback time other than t=0.
 *    Time notifier will receive/send out notification when
 *    time_offset + original callback time is reached.
 * @return  The time notifier if successfully create a time notifier otherwise
 *    NULL.
 */
ZINC_API cmzn_time_notifier_id cmzn_time_keeper_create_notifier_regular(
	cmzn_time_keeper_id time_keeper, double update_frequency, double time_offset);

/***************************************************************************//**
 * Add a time notifier to the time keeper. The time keeper will keep track of the
 * time. When time changes, time keeper will notify the time notifier and then
 * the time notifier will notify its clients. time notifier can only have one set of
 * time keeper but time keeper can have multiple time notifiers. This function
 * will increase the access count of the time notifier.
 *
 * @param time_keeper  Handle to time keeper.
 * @param time_notifier  Handle to time notifier.
 * @return  Status CMZN_OK if successfully set time notifier to time keeper,
 * any other value on failure.
 */
ZINC_API int cmzn_time_keeper_add_time_notifier(cmzn_time_keeper_id time_keeper,
	cmzn_time_notifier_id time_notifier);

/***************************************************************************//**
 * Remove the time notifier from the time keeper. This function will decrease the
 * access count of the time notifier.
 * @param time_keeper  Handle to time keeper.
 * @param time_notifier  Handle to time notifier.
 * @return  Status CMZN_OK if successfully set time notifier to time keeper,
 * any other value on failure.
 */
ZINC_API int cmzn_time_keeper_remove_time_notifier(cmzn_time_keeper_id time_keeper,
	cmzn_time_notifier_id time_notifier);

#ifdef __cplusplus
}
#endif

#endif
