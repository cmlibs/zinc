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

TEST(ZincTimekeeper, getNextCallback)
{
	ZincTestSetupCpp zinc;

	Timekeepermodule tkm = zinc.context.getTimekeepermodule();
	EXPECT_TRUE(tkm.isValid());
	Timekeeper timekeeper = tkm.getDefaultTimekeeper();
	EXPECT_TRUE(timekeeper.isValid());

	int result;
	EXPECT_EQ(CMZN_OK, result = timekeeper.setMinimumTime(-5.0));
	EXPECT_EQ(CMZN_OK, result = timekeeper.setMaximumTime(10.0));
	EXPECT_EQ(CMZN_OK, result = timekeeper.setTime(0.0));

	Timenotifier timenotifier = timekeeper.createTimenotifierRegular(
		/*update_frequency*/10, /*time_offset*/0.03);
	EXPECT_TRUE(timenotifier.isValid());

	Timenotifier timenotifier2 = timekeeper.createTimenotifierRegular(
		/*update_frequency*/20, /*time_offset*/0.02);
	EXPECT_TRUE(timenotifier2.isValid());

	ASSERT_DOUBLE_EQ(0.02, timekeeper.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_FORWARD));
	ASSERT_DOUBLE_EQ(0.03, timenotifier.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_FORWARD));
	ASSERT_DOUBLE_EQ(0.02, timenotifier2.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_FORWARD));

	EXPECT_EQ(CMZN_OK, result = timekeeper.setTime(0.02));
	ASSERT_DOUBLE_EQ(0.03, timekeeper.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_FORWARD));
	ASSERT_DOUBLE_EQ(0.03, timenotifier.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_FORWARD));
	ASSERT_DOUBLE_EQ(0.07, timenotifier2.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_FORWARD));

	EXPECT_EQ(CMZN_OK, result = timekeeper.setTime(0.03));
	ASSERT_DOUBLE_EQ(0.07, timekeeper.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_FORWARD));
	ASSERT_DOUBLE_EQ(0.13, timenotifier.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_FORWARD));
	ASSERT_DOUBLE_EQ(0.07, timenotifier2.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_FORWARD));

	EXPECT_EQ(CMZN_OK, result = timekeeper.setTime(0.07));
	ASSERT_DOUBLE_EQ(0.12, timekeeper.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_FORWARD));
	ASSERT_DOUBLE_EQ(0.13, timenotifier.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_FORWARD));
	ASSERT_DOUBLE_EQ(0.12, timenotifier2.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_FORWARD));

	EXPECT_EQ(CMZN_OK, result = timekeeper.setTime(9.99));
	ASSERT_DOUBLE_EQ(-4.98, timekeeper.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_FORWARD));
	ASSERT_DOUBLE_EQ(10.03, timenotifier.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_FORWARD));
	ASSERT_DOUBLE_EQ(10.02, timenotifier2.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_FORWARD));

	EXPECT_EQ(CMZN_OK, result = timekeeper.setTime(0.00));
	ASSERT_DOUBLE_EQ(-0.03, timekeeper.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_REVERSE));
	ASSERT_DOUBLE_EQ(-0.07, timenotifier.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_REVERSE));
	ASSERT_DOUBLE_EQ(-0.03, timenotifier2.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_REVERSE));

	EXPECT_EQ(CMZN_OK, result = timekeeper.setTime(-2.50));
	ASSERT_DOUBLE_EQ(-2.53, timekeeper.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_REVERSE));
	ASSERT_DOUBLE_EQ(-2.57, timenotifier.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_REVERSE));
	ASSERT_DOUBLE_EQ(-2.53, timenotifier2.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_REVERSE));

	EXPECT_EQ(CMZN_OK, result = timekeeper.setTime(-5.0));
	ASSERT_DOUBLE_EQ(9.97, timekeeper.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_REVERSE));
	ASSERT_DOUBLE_EQ(-5.07, timenotifier.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_REVERSE));
	ASSERT_DOUBLE_EQ(-5.03, timenotifier2.getNextCallbackTime(Timekeeper::PLAY_DIRECTION_REVERSE));

}
