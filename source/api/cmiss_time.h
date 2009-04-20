/*******************************************************************************
FILE : cmiss_time.h

DESCRIPTION :
The public interface to the Cmiss_time_notifier which supplies a concept of time 
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
#ifndef __CMISS_TIME_H__
#define __CMISS_TIME_H__

#include "general/object.h"

/***************************************************************************//**
 * A handle to cmiss time notifier. This notifier provides a concept of time to
 * Cmgui, it will notify its client when time has changed if a callback is setup 
 * for this notifier. time notifier normally receives its callback from a
 * time keeper. See Cmiss_time_keeper_add_time_notifier function.
 */
typedef struct Cmiss_time_notifier *Cmiss_time_notifier_id;

/***************************************************************************//**
 * The type used for time notifier callback. It is a pointer to a function which
 * takes the same arguments.
 *
 * @param Cmiss_time_notifier_id  Handle to time notifier.
 * @param current_time  Time in the time notifier when the callback is being 
 *    triggered by the time notifier.
 * @param User_data  any data user want to pass into the callback function.
 * @return  return one if such the callback function 
 *    has been called successfully otherwise 0.
 */
typedef int (*Cmiss_time_notifier_callback)(Cmiss_time_notifier_id time_notifier,
	double current_time, void *user_data);

/***************************************************************************//**
 * Create and returns a time notifier with regular update time.
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
Cmiss_time_notifier_id Cmiss_time_notifier_create_regular(
	double update_frequency, double time_offset);

/***************************************************************************//**
 * Destroys this reference to the time notifier (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param time_notifier_address  The address to the handle to time notifier
 * @return  1 if successfully destroy(deaccess) the time notifier, otherwise 0.
 */
int Cmiss_time_notifier_destroy(Cmiss_time_notifier_id *time_notifier_address);

/***************************************************************************//**
 * Adds a callback routine which is called whenever the time given to the
 * time notifier has been changed.
 *
 * @param time_notifier  Handle to time notifier.
 * @param Cmiss_time_notifier_callback  callback function to be set.
 * @param user_data  Data to be past into the callback routine.
 * @return  1 if successfully add callback, otherwise 0.
 */
int Cmiss_time_notifier_add_callback(Cmiss_time_notifier_id time_notifier,
	Cmiss_time_notifier_callback callback, void *user_data);

/***************************************************************************//**
 * Remove a callback routine which has been added to the time notifier before.
 *
 * @param time_notifier  Handle to time notifier.
 * @param Cmiss_time_notifier_callback  callback function to be .
 * @param user_data  Data that was added to the callback.
 * @return  1 if successfully add callback, otherwise 0.
 */
int Cmiss_time_notifier_remove_callback(Cmiss_time_notifier_id time_notifier,
  Cmiss_time_notifier_callback callback, void *user_data);

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
 * @return  1 if successfully set the update frequency to the value provided,
 *    otherwise 0.
 */
int Cmiss_time_notifier_set_frequency(Cmiss_time_notifier_id time_notifier,
	double frequency);

/***************************************************************************//**
 * This controls the exact time which the time notifier receive callbacks.
 * Time offset will set the notifier to receive callback when 
 * time_offset + original callback time is reached. i.e 
 *
 * @param time_notifier  Handle to time notifier.
 * @param offset  This set the time that notifier will receive callback.
 * @return  1 if successfully set the update frequency to the value provided,
 *    otherwise 0.
 */
int Cmiss_time_notifier_set_offset(Cmiss_time_notifier_id time_notifier,
	double time_offset);

#endif /* __CMISS_TIME_H__ */
