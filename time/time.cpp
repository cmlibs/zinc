/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <zinc/timekeeper.h>
#include <zinc/timenotifier.h>

#include <zinc/timekeeper.hpp>
#include <zinc/timenotifier.hpp>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

void timeNotifierCallback(cmzn_timenotifierevent_id event, void *storeTimeAddressVoid)
{
	double *storeTimeAddress = static_cast<double *>(storeTimeAddressVoid);
	EXPECT_NE(static_cast<double *>(0), storeTimeAddress);
	*storeTimeAddress = cmzn_timenotifierevent_get_time(event);
}

TEST(cmzn_timekeeper, api)
{
	ZincTestSetup zinc;

	cmzn_timekeepermodule_id tkm = cmzn_context_get_timekeepermodule(zinc.context);
	EXPECT_NE(static_cast<cmzn_timekeepermodule_id>(0), tkm);
	cmzn_timekeeper_id timekeeper = cmzn_timekeepermodule_get_default_timekeeper(tkm);
	EXPECT_NE(static_cast<cmzn_timekeeper_id>(0), timekeeper);

	int result;
	EXPECT_EQ(CMZN_OK, result = cmzn_timekeeper_set_minimum_time(timekeeper, -1.0));
	ASSERT_DOUBLE_EQ(-1.0, cmzn_timekeeper_get_minimum_time(timekeeper));
	EXPECT_EQ(CMZN_OK, result = cmzn_timekeeper_set_maximum_time(timekeeper, 5.0));
	ASSERT_DOUBLE_EQ(5.0, cmzn_timekeeper_get_maximum_time(timekeeper));
	EXPECT_EQ(CMZN_OK, result = cmzn_timekeeper_set_time(timekeeper, 2.5));
	ASSERT_DOUBLE_EQ(2.5, cmzn_timekeeper_get_time(timekeeper));

	cmzn_timenotifier_id timenotifier = cmzn_timekeeper_create_timenotifier_regular(
		timekeeper, /*update_frequency*/10, /*time_offset*/0.0);
	EXPECT_NE(static_cast<cmzn_timenotifier_id>(0), timenotifier);
	ASSERT_DOUBLE_EQ(2.5, cmzn_timenotifier_get_time(timenotifier));
	double notifiedTime = 0.0;
	EXPECT_EQ(CMZN_OK, cmzn_timenotifier_set_callback(timenotifier, timeNotifierCallback, static_cast<void *>(&notifiedTime)));

	EXPECT_EQ(CMZN_OK, result = cmzn_timekeeper_set_time(timekeeper, 3.3));
	// check callback happened
	ASSERT_DOUBLE_EQ(3.3, notifiedTime);
	EXPECT_EQ(CMZN_OK, cmzn_timenotifier_clear_callback(timenotifier));

	EXPECT_EQ(CMZN_OK, result = cmzn_timekeeper_set_time(timekeeper, 4.7));
	// check callback did not happen
	ASSERT_DOUBLE_EQ(3.3, notifiedTime);

	cmzn_timenotifier_destroy(&timenotifier);
	cmzn_timekeeper_destroy(&timekeeper);
	cmzn_timekeepermodule_destroy(&tkm);
}

class TimenotifiercallbackRecordEvent : public Timenotifiercallback
{
public:
	Timenotifierevent lastEvent;

	TimenotifiercallbackRecordEvent()
	{ }

	virtual void operator()(const Timenotifierevent &event)
	{
		this->lastEvent = event;
	}
};

TEST(ZincTimekeeper, api)
{
	ZincTestSetupCpp zinc;

	Timekeepermodule tkm = zinc.context.getTimekeepermodule();
	EXPECT_TRUE(tkm.isValid());
	Timekeeper timekeeper = tkm.getDefaultTimekeeper();
	EXPECT_TRUE(timekeeper.isValid());

	int result;
	EXPECT_EQ(CMZN_OK, result = timekeeper.setMinimumTime(-1.0));
	ASSERT_DOUBLE_EQ(-1.0, timekeeper.getMinimumTime());
	EXPECT_EQ(CMZN_OK, result = timekeeper.setMaximumTime(5.0));
	ASSERT_DOUBLE_EQ(5.0, timekeeper.getMaximumTime());
	EXPECT_EQ(CMZN_OK, result = timekeeper.setTime(2.5));
	ASSERT_DOUBLE_EQ(2.5, timekeeper.getTime());

	Timenotifier timenotifier = timekeeper.createTimenotifierRegular(
		/*update_frequency*/10, /*time_offset*/0.0);
	EXPECT_TRUE(timenotifier.isValid());
	// test cast
	TimenotifierRegular timenotifierRegular = timenotifier.castRegular();
	EXPECT_TRUE(timenotifierRegular.isValid());
	TimenotifiercallbackRecordEvent thisCallback;
	timenotifier.setCallback(thisCallback);
	EXPECT_FALSE(thisCallback.lastEvent.isValid());
	EXPECT_EQ(CMZN_OK, result = timekeeper.setTime(3.5));
	EXPECT_TRUE(thisCallback.lastEvent.isValid());
	ASSERT_DOUBLE_EQ(3.5, thisCallback.lastEvent.getTime());

	timenotifierRegular.setOffset(5.0);
	timenotifierRegular.setFrequency(1.01);
	EXPECT_EQ(CMZN_OK, result = timekeeper.setTime(3.99));
	ASSERT_DOUBLE_EQ(3.99, thisCallback.lastEvent.getTime());

	EXPECT_EQ(CMZN_OK, result = timenotifier.clearCallback());
}
