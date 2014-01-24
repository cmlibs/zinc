/**
 * FILE : timekeeperid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_TIMEKEEPERID_H__
#define CMZN_TIMEKEEPERID_H__

enum cmzn_timekeeper_play_direction
{
	CMZN_TIMEKEEPER_PLAY_DIRECTION_INVALID = 0,
	CMZN_TIMEKEEPER_PLAY_DIRECTION_FORWARD = 1,
	CMZN_TIMEKEEPER_PLAY_DIRECTION_REVERSE = 2
};

/**
 * A handle to a time keeper.
 * Time keeper maintains a current time which can be automatically advanced
 * with the system clock to drive animation. It sends callbacks to time notifiers
 * at the requested time or interval.
 */
	struct cmzn_timekeeper;
	typedef struct cmzn_timekeeper *cmzn_timekeeper_id;

	struct cmzn_timekeepermodule;
	typedef struct cmzn_timekeepermodule * cmzn_timekeepermodule_id;

#endif
