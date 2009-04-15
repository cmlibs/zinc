/*******************************************************************************
FILE : cmiss_time_keeper.h

LAST MODIFIED : 2 Mar 2009

DESCRIPTION :
The public interface of Cmiss_time_keeper which defines a relationship between
a bunch of time objects, keeps them in sync and allows control such as play,
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

#include "general/object.h"
#include "api/cmiss_time.h"

/***************************************************************************//**
 * An enum type to define which direction the time keeper should go.
 */
enum Cmiss_time_keeper_play_direction
{
	CMISS_TIME_KEEPER_PLAY_FORWARD = 1,
	CMISS_TIME_KEEPER_PLAY_BACKWARD = 2
};

/***************************************************************************//**
 * An enum type to define the play mode of the time keeper.
 */
enum Cmiss_time_keeper_repeat_mode
{
	CMISS_TIME_KEEPER_INVALID_REPEAT_MODE = 0, /*!< Invalid play mode to handle special
																							 circumstances. */
	CMISS_TIME_KEEPER_PLAY_ONCE = 1, /*!< Only play once until it reaches either the
																    minimum or maximum time set on the 
																		time keeper. */  
	CMISS_TIME_KEEPER_PLAY_LOOP = 2, /*!< Play repeatedly in the same direction.
																    i.e The time keeper will start from the 
																		minimum time again after it reaches the 
																		maximum time during a forward playback
																		time keeper. */  
	CMISS_TIME_KEEPER_PLAY_SWING = 3 /*!< Play repeatedly in both direction.
																     i.e The time keeper will play forward till 
																		 it reaches the maximum time and then play 
																		 backward to minimum time and the same cycle
																		 will repeat until stopped by the user. */
};

enum Cmiss_time_keeper_frame_mode
{
	CMISS_TIME_KEEPER_INVALID_FRAME_MODE = 0,
	CMISS_TIME_KEEPER_PLAY_REAL_TIME = 1,
	CMISS_TIME_KEEPER_PLAY_EVERY_FRAME = 2
};

/***************************************************************************//**
 * A handle to a time keeper. 
 * Time keeper maintains a current time which can be automatically advanced
 * with the system clock to drive animation. It sends callbacks to time objects
 * at the requested time or interval.
 */
typedef struct Cmiss_time_keeper *Cmiss_time_keeper_id;

/***************************************************************************//**
 * Get the minimum allowed time in time keeper.
 *
 * @param time_keeper  Handle to time keeper.
 * @return  the minimum allowed time in time keeper if successful, otherwise 0.
 */
double Cmiss_time_keeper_get_minimum(Cmiss_time_keeper_id time_keeper);

/***************************************************************************//**
 * Set the minimum available time of the time keeper. The time keeper will not
 * allow any time lower then the minimum time specified by this function.
 *
 * @param time_keeper  Handle to time keeper.
 * @param minimum  The minimum time to be set.
 * @return  1 if successfully set minimum time, otherwise 0.
 */
int Cmiss_time_keeper_set_minimum(Cmiss_time_keeper_id time_keeper, 
	double minimum);

/***************************************************************************//**
 * Get the maximum allowed time in time keeper.
 *
 * @param time_keeper  Handle to time keeper.
 * @return  the maximum allowed time in time keeper if successful, otherwise 0.
 */
double Cmiss_time_keeper_get_maximum(Cmiss_time_keeper_id time_keeper);

/***************************************************************************//**
 * Set the maximum time of time keeper. The time keeper will not allow any time 
 * larger than the value set by this function.
 *
 * @param time_keeper  Handle to time keeper.
 * @param maximum  The maximum time to be set.
 * @return  1 if successfully set maximum time, otherwise 0.
 */
int Cmiss_time_keeper_set_maximum(Cmiss_time_keeper_id time_keeper, 
	double maximum);

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
 * system time and between time objects.
 *
 * @param time_keeper  Handle to time keeper.
 * @param frame_mode  Enumerator to set the frame mode of the time keeper.
 * @return  1 if successfully set the time keeper to play every frame, otherwise 
 *    0.
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
 * objects.
 *
 * @param time_keeper  Handle to time keeper.
 * @param Cmiss_time_keeper_play_direction  Enumerator to tell time keeper which
 *    direction it should play.
 * @return  1 if successfully set the frame mode of time keeper, otherwise 0.
 */
int Cmiss_time_keeper_play(Cmiss_time_keeper_id time_keeper,
	enum Cmiss_time_keeper_play_direction play_direction);

/***************************************************************************//**
 * Check whether the time keeper is playing.
 * @param time_keeper  Handle to time keeper.
 * @return  1 if successfully called and time keeper is playing, otherwise 0.
 */
int Cmiss_time_keeper_is_playing(Cmiss_time_keeper_id time_keeper);

/***************************************************************************//**
 * Stops the time keeper from playing.
 *
 * @param time_keeper  Handle to time keeper.
 * @return  1 if successfully to stop the time keeper, otherwise 0.
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
 * @return  1 if successfully set the repeat mode of time keeper, otherwise 0.
 */
int Cmiss_time_keeper_set_repeat_mode(Cmiss_time_keeper_id time_keeper,
	enum Cmiss_time_keeper_repeat_mode repeat_mode);

/***************************************************************************//**
 * Get the current time from the time keeper.
 *
 * @param time_keeper  Handle to time keeper.
 * @return  current time of the time keeper if successful otherwise 0.
 */
double Cmiss_time_keeper_get_time(Cmiss_time_keeper_id time_keeper);

/***************************************************************************//**
 * Set a new time on the time keeper. This new time should not be 
 * smaller than the minimum time or larger then the maximum time set on the
 * time keeper. This will notify the clients of the changes.
 *
 * @param time_keeper  Handle to time keeper.
 * @param new_time  Time to be set on the time keeper
 * @return  1 if successfully set a time on time keeper, otherwise 0.
 */
int Cmiss_time_keeper_set_time(Cmiss_time_keeper_id time_keeper, 
	double new_time);

/***************************************************************************//**
 * Get speed from the time object
 *
 * @param time_keeper  Handle to time keeper.
 * @return  Speed of the time keeper if successful otherwise 0.
 */
double Cmiss_time_keeper_get_speed(Cmiss_time_keeper_id time_keeper);

/***************************************************************************//**
 * Set the rate of increment/decrement on time during playback. This will affect 
 * how often an event is generate by the time keeper.
 * i.e If the value of speed is 2 then the amount of real time taken for the
 * time keeper to progress an unit of time is approximately (1/speed)s which is 
 * about 0.5s.
 *
 * @param time_keeper  Handle to time keeper.
 * @param speed  This will affect how quickly time keeper will progress on its 
 *    unit of time per second.
 * @return  1 if successfully set the speed on time keeper, otherwise 0.
 */
int Cmiss_time_keeper_set_speed(Cmiss_time_keeper_id time_keeper,
	double speed);

/***************************************************************************//**
 * Add a time object to the time keeper. The time keeper will keep track of the
 * time. When time changes, time keeper will notify the time object and then 
 * the time object will notify its clients. Time object can only have one set of
 * time keeper but time keeper can have multiple time objects. This function 
 * will increase the access count of the time object. 
 * 
 * @param time_keeper  Handle to time keeper.
 * @param time_object  Handle to time object.
 * @return  1 if successfully set time object to time keeper, otherwise 0.
 */
int Cmiss_time_keeper_add_time_object(Cmiss_time_keeper_id time_keeper, 
	Cmiss_time_object_id time_object);

/***************************************************************************//**
 * Remove the time object from the time keeper. This function will decrease the 
 * access count of the time object. 
 * @param time_keeper  Handle to time keeper.
 * @param time_object  Handle to time object.
 * @return  1 if successfully set time object to time keeper, otherwise 0.
 */
int Cmiss_time_keeper_remove_time_object(Cmiss_time_keeper_id time_keeper, 
	Cmiss_time_object_id time_object);

/***************************************************************************//**
 * Destroys this reference to the time keeper (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param time_keeper_address  The address to the handle to time keeper
 * @return  1 if successfully destroy(deaccess) the time keeper, otherwise 0.
 */
int Cmiss_time_keeper_destroy(Cmiss_time_keeper_id *time_keeper_address);

#endif /* __CMISS_TIME_KEEPER_H__ */
