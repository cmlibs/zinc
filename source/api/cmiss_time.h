/*******************************************************************************
FILE : cmiss_time.h

DESCRIPTION :
The public interface to the Cmiss_time_object which supplies a concept of time 
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
#include "api/cmiss_time_keeper.h"

/***************************************************************************//**
 * A handle to cmiss time object. This object provides a concept of time to
 * Cmgui, it will notify its client when time has changed if a callback is setup 
 * for this object. Time object normally receives its callback from a
 * time keeper. See Cmiss_time_object_set_time_keeper function.
 */
typedef struct Cmiss_time_object *Cmiss_time_object_id;

/***************************************************************************//**
 * The type used for time object callback. It is a pointer to a function which
 * takes the same arguments.
 *
 * @param Cmiss_time_object_id  Handle to time object.
 * @param current_time  Time in the time object when the callback is being 
 *    triggered by the time object.
 * @param User_data  any data user want to pass into the callback function.
 * @return  return one if such the callback function 
 *    has been called successfully otherwise 0.
 */
typedef int (*Cmiss_time_object_callback)(Cmiss_time_object_id time_object,
	double current_time, void *user_data);

/***************************************************************************//**
 * Creates and returns a time object called name. This object will immediately
 * be accessed. Time object will be returned with theaccess count = 1.
 *
 * @param name  Name to be set to the time object.
 * @return  The time object if successfully create a time object otherwise NULL.
 */
Cmiss_time_object_id Cmiss_time_object_create(const char *name);

/***************************************************************************//**
 * Destroys this reference to the time object (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param time_object_address  The address to the handle to time object
 * @return  1 if successfully destroy(deaccess) the time object, otherwise 0.
 */
int Cmiss_time_object_destroy(Cmiss_time_object_id *time_object_address);

/***************************************************************************//**
 * Adds a callback routine which is called whenever the time given to the
 * time object has been changed.
 *
 * @param time_object  Handle to time object.
 * @param Cmiss_time_object_callback  callback function to be set.
 * @param user_data  Data to be past into the callback routine.
 * @return  1 if successfully add callback, otherwise 0.
 */
int Cmiss_time_object_add_callback(Cmiss_time_object_id time_object,
	Cmiss_time_object_callback callback, void *user_data);

/***************************************************************************//**
 * Remove a callback routine which has been added to the time object before.
 *
 * @param time_object  Handle to time object.
 * @param Cmiss_time_object_callback  callback function to be .
 * @param user_data  Data that was added to the callback.
 * @return  1 if successfully add callback, otherwise 0.
 */
int Cmiss_time_object_remove_callback(Cmiss_time_object_id time_object,
  Cmiss_time_object_callback callback, void *user_data);

/***************************************************************************//**
 * Get the handle to time keeper from time object.
 *
 * @param time_object  Handle to time object.
 * @return  The handle to time keeper if successfully get time keeper from 
 *    time object, otherwise NULL.
 */
Cmiss_time_keeper_id Cmiss_time_object_get_time_keeper(
	Cmiss_time_object_id time_object);

/***************************************************************************//**
 * Set the time keeper for time object. The time keeper will keep track of the
 * time. When time changes, time keeper will notify the time object and then 
 * the time object will notify its clients. This will increase the access count
 * of the new time_keeper and decrease the access count of the original
 * time_keeper in the time_object by 1.
 *
 * @param time_object  Handle to time object.
 * @param time_keeper  Handle to time keeper. If this is null it will remove
 *    the time keeper from the time object and time object will be without
 *    a time keeper.
 * @return  1 if successfully set time keeper for time object, otherwise 0.
 */
int Cmiss_time_object_set_time_keeper(Cmiss_time_object_id time_object,
	Cmiss_time_keeper_id time_keeper);

/***************************************************************************//**
 * Get the current time of time object.
 *
 * @param time_object  Handle to time object.
 * @return  current time of the time object if successful otherwise 0.
 */
double Cmiss_time_object_get_current_time(Cmiss_time_object_id time_object);

/***************************************************************************//**
 * This controls the rate which the time depedent object is called back.
 * The default value is 10 which means time object will receive 10 callbacks
 * per unit of time in the time keeper. 
 * i.e. If the speed of time keeper is set to be 1 and the update frequency of 
 * time object is set to be 10, the actual interval between each callbacks is:
 * (1/speed of time keeper)/(udpate frequency) which is 0.1s.
 * Note that the time object is not promised to receive callback exactly 0.1s
 * after the previous callback.
 *
 * @param time_object  Handle to time object.
 * @param frequency  The number of times which time object will receive
 *    callback per unit of time in the time keeper.
 * @return  1 if successfully set the update frequency to the value provided,
 *    otherwise 0.
 */
int Cmiss_time_object_set_update_frequency(Cmiss_time_object_id time_object,
	double frequency);

#endif /* __CMISS_TIME_KEEPER_H__ */
