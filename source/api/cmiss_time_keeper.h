/*******************************************************************************
FILE : cmiss_time_keeper.h

LAST MODIFIED : 2 Mar 2009

DESCRIPTION :
The public interface of Cmiss_time_keeper which defines a relationship between
a bunch of time notifiers, keeps them in sync and allows control such as play,
rewind and fast forward.
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
#ifndef __CMISS_TIME_KEEPER_H__
#define __CMISS_TIME_KEEPER_H__

#include "api/types/cmiss_time_id.h"
#include "api/types/cmiss_time_keeper_id.h"

/***************************************************************************//**
 * An enum type to define which direction the time keeper should go.
 */
enum Cmiss_time_keeper_play_direction
{
	CMISS_TIME_KEEPER_PLAY_INVALD = 0,
	CMISS_TIME_KEEPER_PLAY_FORWARD = 1,
	CMISS_TIME_KEEPER_PLAY_BACKWARD = 2
};

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
enum Cmiss_time_keeper_play_direction
	Cmiss_time_keeper_play_direction_enum_from_string(const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param direction  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
char *Cmiss_time_keeper_play_direction_enum_to_string(
	enum Cmiss_time_keeper_play_direction direction);

/***************************************************************************//**
 * An enum type to define the play mode of the time keeper.
 */
enum Cmiss_time_keeper_repeat_mode
{
	CMISS_TIME_KEEPER_REPEAT_MODE_INVALID = 0, /*!< Invalid play mode to handle special
																							 circumstances. */
	CMISS_TIME_KEEPER_REPEAT_MODE_PLAY_ONCE = 1, /*!< Only play once until it reaches either the
																    minimum or maximum time set on the 
																		time keeper. */  
	CMISS_TIME_KEEPER_REPEAT_MODE_PLAY_LOOP = 2, /*!< Play repeatedly in the same direction.
																    i.e The time keeper will start from the 
																		minimum time again after it reaches the 
																		maximum time during a forward playback
																		time keeper. */  
	CMISS_TIME_KEEPER_REPEAT_MODE_PLAY_SWING = 3 /*!< Play repeatedly in both direction.
																     i.e The time keeper will play forward till 
																		 it reaches the maximum time and then play 
																		 backward to minimum time and the same cycle
																		 will repeat until stopped by the user. */
};

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
enum Cmiss_time_keeper_repeat_mode Cmiss_time_keeper_repeat_mode_enum_from_string(
	const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param mode  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
char *Cmiss_time_keeper_repeat_mode_enum_to_string(
	enum Cmiss_time_keeper_repeat_mode mode);

enum Cmiss_time_keeper_frame_mode
{
	CMISS_TIME_KEEPER_FRAME_MODE_INVALID = 0,
	CMISS_TIME_KEEPER_FRAME_MODE_PLAY_REAL_TIME = 1,
	CMISS_TIME_KEEPER_FRAME_MODE_PLAY_EVERY_FRAME = 2
};

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
enum Cmiss_time_keeper_frame_mode Cmiss_time_keeper_frame_mode_enum_from_string(
	const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param mode  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
char *Cmiss_time_keeper_frame_mode_enum_to_string(
	enum Cmiss_time_keeper_frame_mode mode);

enum Cmiss_time_keeper_attribute
{
	CMISS_TIME_KEEPER_ATTRIBUTE_INVALID = 0,
	CMISS_TIME_KEEPER_ATTRIBUTE_TIME = 1,
	/*!< Current time of the time keeper. This is a real number attribute.
	 */
	CMISS_TIME_KEEPER_ATTRIBUTE_MINIMUM_TIME = 2,
	/*!< Minimum allowed time of the time keeper. The time keeper will not
	 * allow any time lower then the minimum time specified by this function.
	 * This is a real number attribute.
	 */
	CMISS_TIME_KEEPER_ATTRIBUTE_MAXIMUM_TIME = 3,
	/*!< Maximum allowed time of the time keeper. The time keeper will not allow
	 * any time larger than the value set by this function.
	 * This is a real number attribute.
	 */
	CMISS_TIME_KEEPER_ATTRIBUTE_SPEED = 4
	/*!< Rate of increment/decrement on time during playback. This will affect
	 * how often an event is generate by the time keeper.
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
enum Cmiss_time_keeper_attribute Cmiss_time_keeper_attribute_enum_from_string(
	const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param mode  attribute to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
char *Cmiss_time_keeper_attribute_enum_to_string(
	enum Cmiss_time_keeper_attribute attribute);

/***************************************************************************//**
 * Access the time_keeper, increase the access count of the time keeper by one.
 *
 * @param time_keeper  handle to the "to be access" cmiss time_keeper.
 * @return  handle to time_keeper if successfully access time_keeper.
 */
Cmiss_time_keeper_id Cmiss_time_keeper_access(Cmiss_time_keeper_id time_keeper);

/***************************************************************************//**
 * Destroys this reference to the time keeper (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param time_keeper_address  The address to the handle to time keeper
 * @return  Status CMISS_OK if successfully destroy the time keeper,
 * any other value on failure.
 */
int Cmiss_time_keeper_destroy(Cmiss_time_keeper_id *time_keeper_address);

/***************************************************************************//**
 * Get a real value of an attribute of the time keeper.
 *
 * @param time_keeper  Handle to the cmiss time_keeper.
 * @param attribute  The identifier of the real attribute to get.
 * @return  Value of the attribute.
 */
double Cmiss_time_keeper_get_attribute_real(Cmiss_time_keeper_id time_keeper,
	enum Cmiss_time_keeper_attribute attribute);

/***************************************************************************//**
 * Set a real value for an attribute of the time_keeper.
 *
 * @param time_keeper  Handle to the cmiss time_keeper.
 * @param attribute  The identifier of the real attribute to set.
 * @param value  The new value for the attribute.
 * @return  Status CMISS_OK if attribute successfully set, any other value if
 * failed or attribute not valid or unable to be set for this time_keeper object.
 */
int Cmiss_time_keeper_set_attribute_real(Cmiss_time_keeper_id time_keeper,
	enum Cmiss_time_keeper_attribute attribute, double value);

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
Cmiss_time_notifier_id Cmiss_time_keeper_create_notifier_regular(
	Cmiss_time_keeper_id time_keeper, double update_frequency, double time_offset);

/***************************************************************************//**
 * Get the current frame mode of the time keeper. 
 *
 * @param time_keeper  Handle to time keeper.
 * @return  CMISS_TIME_KEEPER_REAL_TIME or
 *    CMISS_TIME_KEEPER_EVERY_FRAME if successfully called, otherwise
 *    CMISS_TIME_KEEPER_INVALID_FRAME_MODE is returned.
 */
enum Cmiss_time_keeper_frame_mode Cmiss_time_keeper_get_frame_mode(
	Cmiss_time_keeper_id time_keeper);

/***************************************************************************//**
 * Set the time keeper to either play real time or every frame. In real time
 * mode, the time keeper may skip frames to ensure animation matches real-time 
 * as closely as possible, when the redraw time is greater than the requested 
 * interval. Every frame mode will cause the time keeper to generate every 
 * event it thinks is due and may affect the synchronisation both with the 
 * system time and between time notifiers.
 *
 * @param time_keeper  Handle to time keeper.
 * @param frame_mode  Enumerator to set the frame mode of the time keeper.
 * @return  Status CMISS_OK if successfully set the time keeper to play every
 * frame, any other value on failure.
 */
int Cmiss_time_keeper_set_frame_mode(Cmiss_time_keeper_id time_keeper,
	enum Cmiss_time_keeper_frame_mode frame_mode);

/***************************************************************************//**
 * Get the current playing direction of the time keeper. 
 *
 * @param time_keeper  Handle to time keeper.
 * @return  CMISS_TIME_KEEPER_PLAY_FORWARD or CMISS_TIME_KEEPER_PLAY_BACKWARD
 */
enum Cmiss_time_keeper_play_direction Cmiss_time_keeper_get_play_direction(
	Cmiss_time_keeper_id time_keeper);

/***************************************************************************//**
 * Set the time keeper to play forward or backward in time, synchronised by the
 * system clock. Generates time events during playback as requested by time
 * notifier.
 *
 * @param time_keeper  Handle to time keeper.
 * @param Cmiss_time_keeper_play_direction  Enumerator to tell time keeper which
 *    direction it should play.
 * @return  Status CMISS_OK if successfully set the frame mode of time keeper,
 * any other value on failure.
 */
int Cmiss_time_keeper_play(Cmiss_time_keeper_id time_keeper,
	enum Cmiss_time_keeper_play_direction play_direction);

/***************************************************************************//**
 * Check whether the time keeper is playing.
 * @param time_keeper  Handle to time keeper.
 * @return   Status CMISS_OK if successfully called and time keeper is playing,
 * any other value on failure.
 */
int Cmiss_time_keeper_is_playing(Cmiss_time_keeper_id time_keeper);

/***************************************************************************//**
 * Stops the time keeper from playing.
 *
 * @param time_keeper  Handle to time keeper.
 * @return  Status CMISS_OK if successfully to stop the time keeper,
 * any other value on failure.
 */
int Cmiss_time_keeper_stop(Cmiss_time_keeper_id time_keeper);

/***************************************************************************//**
 * Get the current repeat mode of the time keeper. 
 *
 * @param time_keeper  Handle to time keeper.
 * @return  CMISS_TIME_KEEPER_PLAY_ONCE, CMISS_TIME_KEEPER_PLAY_LOOP or
 *    CMISS_TIME_KEEPER_PLAY_SWING if successfully called, otherwise
 *    CMISS_TIME_KEEPER_INVALID_REPEAT_MODE is returned. 
 */
enum Cmiss_time_keeper_repeat_mode Cmiss_time_keeper_get_repeat_mode(
	Cmiss_time_keeper_id time_keeper);

/***************************************************************************//**
 * Set the repeat mode of the time keeper. 
 *
 * @param time_keeper  Handle to time keeper.
 * @param Cmiss_time_keeper_repeat_mode  enumerator to tell time keeper which
 *    repeat mode it should be playing.
 * @return  Status CMISS_OK if successfully set the repeat mode of time keeper,
 * any other value on failure.
 */
int Cmiss_time_keeper_set_repeat_mode(Cmiss_time_keeper_id time_keeper,
	enum Cmiss_time_keeper_repeat_mode repeat_mode);

/***************************************************************************//**
 * Add a time notifier to the time keeper. The time keeper will keep track of the
 * time. When time changes, time keeper will notify the time notifier and then 
 * the time notifier will notify its clients. time notifier can only have one set of
 * time keeper but time keeper can have multiple time notifiers. This function 
 * will increase the access count of the time notifier. 
 * 
 * @param time_keeper  Handle to time keeper.
 * @param time_notifier  Handle to time notifier.
 * @return  Status CMISS_OK if successfully set time notifier to time keeper,
 * any other value on failure.
 */
int Cmiss_time_keeper_add_time_notifier(Cmiss_time_keeper_id time_keeper, 
	Cmiss_time_notifier_id time_notifier);

/***************************************************************************//**
 * Remove the time notifier from the time keeper. This function will decrease the 
 * access count of the time notifier. 
 * @param time_keeper  Handle to time keeper.
 * @param time_notifier  Handle to time notifier.
 * @return  Status CMISS_OK if successfully set time notifier to time keeper,
 * any other value on failure.
 */
int Cmiss_time_keeper_remove_time_notifier(Cmiss_time_keeper_id time_keeper, 
	Cmiss_time_notifier_id time_notifier);

#endif /* __CMISS_TIME_KEEPER_H__ */
