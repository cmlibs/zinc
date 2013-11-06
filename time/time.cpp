#include <gtest/gtest.h>

#include <zinc/timekeeper.h>
#include <zinc/timenotifier.h>

#include <zinc/timekeeper.hpp>
#include <zinc/timenotifier.hpp>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

int timeNotifierCallback(cmzn_timenotifierevent_id event, void *storeTimeAddressVoid)
{
	double *storeTimeAddress = static_cast<double *>(storeTimeAddressVoid);
	EXPECT_NE(static_cast<double *>(0), storeTimeAddress);
	*storeTimeAddress = cmzn_timenotifierevent_get_time(event);
	return CMZN_OK;
}

TEST(cmzn_timekeeper, api)
{
	ZincTestSetup zinc;

	cmzn_timekeeper_id timekeeper = cmzn_context_get_default_timekeeper(zinc.context);
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
}

class myTimenotifier : public Timenotifiercallback
{
private:
	Timekeeper timekeeper;

	virtual int operator()(const Timenotifierevent &event)
	{
		EXPECT_EQ(timekeeper.getTime(), event.getTime());
		return CMZN_OK;
	}

public:
	myTimenotifier(Timekeeper timekeeper_in)
	{
		timekeeper = timekeeper_in;
	}
};

TEST(ZincTimekeeper, api)
{
	ZincTestSetupCpp zinc;

	Timekeeper timekeeper = zinc.context.getDefaultTimekeeper();
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
	myTimenotifier thisNotifier(timekeeper);
	timenotifier.setCallback(thisNotifier);
	EXPECT_EQ(CMZN_OK, result = timekeeper.setTime(3.5));
	EXPECT_EQ(CMZN_OK, result = timenotifier.clearCallback());
	// timenotifier callbacks not yet implemented in C++
}
