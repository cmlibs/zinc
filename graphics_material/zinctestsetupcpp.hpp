#ifndef __ZINCTESTSETUPCPP_HPP__
#define __ZINCTESTSETUPCPP_HPP__

#include <gtest/gtest.h>

#include <zinc/status.h>
#include <zinc/context.hpp>
#include <zinc/region.hpp>
#include <zinc/fieldmodule.hpp>
#include <zinc/glyph.hpp>
#include <zinc/scene.hpp>

using namespace OpenCMISS::Zinc;

class ZincTestSetupCpp
{
public:
	Context context;
	Region root_region;
	Fieldmodule fm;
	Glyphmodule glyphmodule;
	Scene scene;

	ZincTestSetupCpp() :
		context("test"),
		root_region(context.getDefaultRegion()),
		fm(root_region.getFieldmodule()),
		glyphmodule(context.getGlyphmodule()),
		scene(0)
	{
		scene = root_region.getScene();
		EXPECT_TRUE(fm.isValid());
		EXPECT_TRUE(glyphmodule.isValid());
		EXPECT_EQ(CMZN_OK, glyphmodule.defineStandardGlyphs());
		EXPECT_TRUE(scene.isValid());
	}

	~ZincTestSetupCpp()
	{
	}
};

#endif // __ZINCTESTSETUPCPP_HPP__
