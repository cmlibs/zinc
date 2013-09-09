/***************************************************************************//**
 * time_keeper.hpp
 *
 * Declaration of time keeper classes and functions.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TIME_KEEPER_HPP
#define TIME_KEEPER_HPP

#include "general/mystring.h"
#include "general/object.h"
#include "general/debug.h"

PROTOTYPE_OBJECT_FUNCTIONS(cmzn_time_keeper);

#define Time_object cmzn_time_notifier

struct Time_object;

struct Time_object_info
{
	struct Time_object *time_object;
	double next_callback_due;
	struct Time_object_info *next;
};

struct cmzn_time_keeper
{
private:
	const char *name;
	double time;
	struct Time_object_info *time_object_info_list;
	double minimum, maximum, speed;

public:

	int access_count;

	cmzn_time_keeper();

	virtual ~cmzn_time_keeper();

	inline cmzn_time_keeper *access()
	{
		++access_count;
		return this;
	}

	static inline int deaccess(cmzn_time_keeper **time_keeper_address)
	{
		return DEACCESS(cmzn_time_keeper)(time_keeper_address);
	}

	bool setName(const char *name_in);

	char *getName();

	int addTimeObject(struct Time_object *time_object);

	int removeTimeObject(struct Time_object *time_object);

	double getTime();

	double getMinimum();

	int setMinimum(double minimum_in);

	double getMaximum();

	int setMaximum(double maximum_in);

	int setTime(double new_time);

	int hasTimeObject();

	struct Time_object_info *getObjectInfo();

	void setTimeQuiet(double new_time);
};

enum Time_keeper_play_direction
{
	TIME_KEEPER_PLAY_FORWARD,
	TIME_KEEPER_PLAY_BACKWARD
};

#endif
