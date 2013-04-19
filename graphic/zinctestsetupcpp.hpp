#ifndef __ZINCTESTSETUPCPP_HPP__
#define __ZINCTESTSETUPCPP_HPP__

#include <gtest/gtest.h>

#include <zinc/status.h>
#include <zinc/context.hpp>
#include <zinc/region.hpp>
#include <zinc/fieldmodule.hpp>
#include <zinc/rendition.hpp>

using namespace zinc;

class ZincTestSetupCpp
{
public:
	Context context;
	Region root_region;
	FieldModule fm;
	GraphicsModule gm;
	Rendition ren;

	ZincTestSetupCpp() :
		context("test"),
		root_region(context.getDefaultRegion()),
		fm(root_region.getFieldModule()),
		gm(context.getDefaultGraphicsModule(context)),
		ren(0)
	{
		EXPECT_EQ(CMISS_OK, gm.enableRenditions(root_region));
		ren = gm.getRendition(root_region);
		EXPECT_EQ(true, fm.isValid());
		EXPECT_EQ(true, gm.isValid());
		EXPECT_EQ(true, ren.isValid());
	}

	~ZincTestSetupCpp()
	{
	}
};

#endif // __ZINCTESTSETUPCPP_HPP__
