/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ZINCTESTSETUPCPP_HPP__
#define __ZINCTESTSETUPCPP_HPP__

#include <gtest/gtest.h>

#include <cmlibs/zinc/status.hpp>
#include <cmlibs/zinc/context.hpp>
#include <cmlibs/zinc/region.hpp>
#include <cmlibs/zinc/result.hpp>
#include <cmlibs/zinc/fieldmodule.hpp>
#include <cmlibs/zinc/glyph.hpp>
#include <cmlibs/zinc/scene.hpp>

using namespace CMLibs::Zinc;

class ZincTestSetupCpp
{
public:
	Context context;
	Region root_region;
	Fieldmodule fm;
	Glyphmodule glyphmodule;
	Scene scene;

	ZincTestSetupCpp() :
        context("test_graphics"),
		root_region(context.getDefaultRegion()),
		fm(root_region.getFieldmodule()),
		glyphmodule(context.getGlyphmodule()),
		scene(0)
	{
		scene = root_region.getScene();
		EXPECT_TRUE(fm.isValid());
		EXPECT_TRUE(glyphmodule.isValid());
		EXPECT_EQ(OK, glyphmodule.defineStandardGlyphs());
		EXPECT_TRUE(scene.isValid());
	}

	~ZincTestSetupCpp()
	{
	}
};

#endif // __ZINCTESTSETUPCPP_HPP__
