/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ZINCTESTSETUPCPP_HPP__
#define __ZINCTESTSETUPCPP_HPP__

#include <gtest/gtest.h>

#include <opencmiss/zinc/status.hpp>
#include <opencmiss/zinc/context.hpp>
#include <opencmiss/zinc/region.hpp>
#include <opencmiss/zinc/fieldmodule.hpp>
#include <opencmiss/zinc/result.hpp>

using namespace OpenCMISS::Zinc;

class ZincTestSetupCpp
{
public:
	Context context;
	Region root_region;
	Fieldmodule fm;

	ZincTestSetupCpp() :
		context("test"),
		root_region(context.getDefaultRegion()),
		fm(root_region.getFieldmodule())
	{
		EXPECT_EQ(true, fm.isValid());
	}

	~ZincTestSetupCpp()
	{
	}
};

#endif // __ZINCTESTSETUPCPP_HPP__
