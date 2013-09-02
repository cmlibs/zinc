/*******************************************************************************
FILE : cmiss_time.h

DESCRIPTION :
The public interface to the cmzn_time_notifier which supplies a concept of time
to Cmgui.
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
#ifndef CMZN_TIMENOTIFIER_H__
#define CMZN_TIMENOTIFIER_H__

#include "types/timeid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * The type used for time notifier callback. It is a pointer to a function which
 * takes the same arguments.
 *
 * @param cmzn_time_notifier_id  Handle to time notifier.
 * @param current_time  Time in the time notifier when the callback is being
 *    triggered by the time notifier.
 * @param User_data  any data user want to pass into the callback function.
 * @return  return one if such the callback function
 *    has been called successfully otherwise 0.
 */
typedef int (*cmzn_time_notifier_callback)(cmzn_time_notifier_id time_notifier,
	double current_time, void *user_data);

/***************************************************************************//**
 * Access the time notifier, increase the access count of the time notifier
 * by one.
 *
 * @param time_notifier  handle to the "to be access" cmiss time_notifier.
 * @return  handle to time_notifier if successfully access time_notifier.
 */
ZINC_API cmzn_time_notifier_id cmzn_time_notifier_access(
	cmzn_time_notifier_id time_notifier);

/***************************************************************************//**
 * Destroys this reference to the time notifier (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param time_notifier_address  The address to the handle to time notifier
 * @return  Status CMISS_OK if successfully destroy(deaccess) the time notifier,
 * any other value on failure.
 */
ZINC_API int cmzn_time_notifier_destroy(cmzn_time_notifier_id *time_notifier_address);

/***************************************************************************//**
 * Adds a callback routine which is called whenever the time given to the
 * time notifier has been changed.
 *
 * @param time_notifier  Handle to time notifier.
 * @param cmzn_time_notifier_callback  callback function to be set.
 * @param user_data  Data to be past into the callback routine.
 * @return  Status CMISS_OK if successfully add callback,
 * any other value on failure.
 */
ZINC_API int cmzn_time_notifier_add_callback(cmzn_time_notifier_id time_notifier,
	cmzn_time_notifier_callback callback, void *user_data);

/***************************************************************************//**
 * Remove a callback routine which has been added to the time notifier before.
 *
 * @param time_notifier  Handle to time notifier.
 * @param cmzn_time_notifier_callback  callback function to be .
 * @param user_data  Data that was added to the callback.
 * @return  Status CMISS_OK if successfully remove callback, any other value
 * on failure.
 */
ZINC_API int cmzn_time_notifier_remove_callback(cmzn_time_notifier_id time_notifier,
  cmzn_time_notifier_callback callback, void *user_data);

/***************************************************************************//**
 * Gets the current time from the time notifier.
 *
 * @param time_notifier  Handle to time notifier.
 * @return  Current time.
 */
ZINC_API double cmzn_time_notifier_get_current_time(cmzn_time_notifier_id time_notifier);

/***************************************************************************//**
 * This controls the rate which the time depedent object is called back.
 * The default value is 10 which means time notifier will receive 10 callbacks
 * per unit of time in the time keeper.
 * i.e. If the speed of time keeper is set to be 1 and the update frequency of
 * time notifier is set to be 10, the actual interval between each callbacks is:
 * (1/speed of time keeper)/(udpate frequency) which is 0.1s.
 * Note that the time notifier does not promised to receive callback exactly
 * 0.1s after the previous callback.
 *
 * @param time_notifier  Handle to time notifier.
 * @param frequency  The number of times which time notifier will receive
 *    callback per unit of time in the time keeper.
 * @return  Status CMISS_OK if successfully set the update frequency to the value
 * provided, any other value on failure.
 */
ZINC_API int cmzn_time_notifier_regular_set_frequency(cmzn_time_notifier_id time_notifier,
	double frequency);

/***************************************************************************//**
 * This controls the exact time which the time notifier receive callbacks.
 * Time offset will set the notifier to receive callback when
 * time_offset + original callback time is reached. i.e
 *
 * @param time_notifier  Handle to time notifier.
 * @param offset  This set the time that notifier will receive callback.
 * @return  Status CMISS_OK if successfully set the update frequency to the value
 * provided, any other value on failure.
 */
ZINC_API int cmzn_time_notifier_regular_set_offset(cmzn_time_notifier_id time_notifier,
	double time_offset);

#ifdef __cplusplus
}
#endif

#endif
