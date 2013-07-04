#ifndef __ZINCTESTSETUPCPP_HPP__
#define __ZINCTESTSETUPCPP_HPP__

#include <gtest/gtest.h>

#include <zinc/status.h>
#include <zinc/context.hpp>
#include <zinc/region.hpp>
#include <zinc/fieldmodule.hpp>
#include <zinc/glyph.hpp>
#include <zinc/graphicsmodule.hpp>
#include <zinc/rendition.hpp>

using namespace zinc;

class ZincTestSetupCpp
{
public:
	Context context;
	Region root_region;
	FieldModule fm;
	GraphicsModule gm;
	GlyphModule glyphModule;
	Rendition ren;

	ZincTestSetupCpp() :
		context("test"),
		root_region(context.getDefaultRegion()),
		fm(root_region.getFieldModule()),
		gm(context.getDefaultGraphicsModule()),
		glyphModule(gm.getGlyphModule()),
		ren(0)
	{
		EXPECT_EQ(CMISS_OK, gm.enableRenditions(root_region));
		ren = gm.getRendition(root_region);
		EXPECT_TRUE(fm.isValid());
		EXPECT_TRUE(gm.isValid());
		EXPECT_TRUE(glyphModule.isValid());
		EXPECT_EQ(CMISS_OK, glyphModule.defineStandardGlyphs());
		EXPECT_TRUE(ren.isValid());
	}

	~ZincTestSetupCpp()
	{
	}
};

#endif // __ZINCTESTSETUPCPP_HPP__
