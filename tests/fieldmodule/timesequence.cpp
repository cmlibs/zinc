/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <cmlibs/zinc/timesequence.hpp>
#include "zinctestsetupcpp.hpp"

TEST(ZincTimesequence, api)
{
	ZincTestSetupCpp zinc;

	double times[2] = { 1.0, 5.0 };
	Timesequence seq1 = zinc.fm.getMatchingTimesequence(2, times);
	EXPECT_TRUE(seq1.isValid());
	Timesequence seq2 = zinc.fm.getMatchingTimesequence(2, times);
	EXPECT_EQ(seq1, seq2);
	ASSERT_EQ(2, seq1.getNumberOfTimes());
	double outValue;
	ASSERT_DOUBLE_EQ(1.0, outValue = seq1.getTime(1));
	ASSERT_DOUBLE_EQ(5.0, outValue = seq1.getTime(2));
	ASSERT_DOUBLE_EQ(0.0, outValue = seq1.getTime(3));
	ASSERT_DOUBLE_EQ(0.0, outValue = seq1.getTime(0));

	double moreTimes[3] = { 0.1, 0.2, 0.3 };
	Timesequence seq3 = zinc.fm.getMatchingTimesequence(3, moreTimes);
	EXPECT_FALSE(seq1 == seq3);
	int result;
	ASSERT_DOUBLE_EQ(0.1, outValue = seq3.getTime(1));
	EXPECT_EQ(OK, result = seq3.setTime(1, 0.05));
	ASSERT_DOUBLE_EQ(0.05, outValue = seq3.getTime(1));
	EXPECT_EQ(ERROR_ARGUMENT, result = seq3.setTime(0, 0.05));
	EXPECT_EQ(OK, result = seq3.setTime(4, 5.5));
	ASSERT_DOUBLE_EQ(5.5, outValue = seq3.getTime(4));
	ASSERT_EQ(4, seq3.getNumberOfTimes());
}
