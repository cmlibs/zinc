#ifndef __ZINCTESTSETUPCPP_HPP__
#define __ZINCTESTSETUPCPP_HPP__

#include <gtest/gtest.h>

#include <zinc/status.hpp>
#include <zinc/context.hpp>
#include <zinc/region.hpp>
#include <zinc/fieldmodule.hpp>

using namespace OpenCMISS::Zinc;

class ZincTestSetupCpp
{
public:
	Context context;
	Region root_region;
	FieldModule fm;

	ZincTestSetupCpp() :
		context("test"),
		root_region(context.getDefaultRegion()),
		fm(root_region.getFieldModule())
	{
		EXPECT_EQ(true, fm.isValid());
	}

	~ZincTestSetupCpp()
	{
	}
};

#endif // __ZINCTESTSETUPCPP_HPP__
