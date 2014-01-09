/**
 * FILE : timenotifierid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_TIMENOTIFIERID_H__
#define CMZN_TIMENOTIFIERID_H__

/**
 * A handle to zinc time notifier. This notifier provides a concept of time to
 * Cmgui, it will notify its client when time has changed if a callback is setup
 * for this notifier. time notifier normally receives its callback from a
 * time keeper. See cmzn_timekeeper_create_timenotifier_regular function.
 */
	struct cmzn_timenotifier;
	typedef struct cmzn_timenotifier *cmzn_timenotifier_id;

	struct cmzn_timenotifierevent;
	typedef struct cmzn_timenotifierevent * cmzn_timenotifierevent_id;

/**
 * A derived timenotifier type which requests notifications at regular intervals.
 */
struct cmzn_timenotifier_regular;
typedef struct cmzn_timenotifier_regular *cmzn_timenotifier_regular_id;

#endif
