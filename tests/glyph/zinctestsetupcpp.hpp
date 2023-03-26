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

#include <opencmiss/zinc/context.hpp>
#include <opencmiss/zinc/fieldmodule.hpp>
#include <opencmiss/zinc/glyph.hpp>
#include <opencmiss/zinc/region.hpp>
#include <opencmiss/zinc/result.hpp>
#include <opencmiss/zinc/scene.hpp>
#include <opencmiss/zinc/status.hpp>

using namespace OpenCMISS::Zinc;

class ZincTestSetupCpp
{
public:
	Context context;
	Region root_region;
	Fieldmodule fm;
	Glyphmodule glyphmodule;
	Materialmodule materialModule;
	Scene scene;

	ZincTestSetupCpp() :
		context("test"),
		root_region(context.getDefaultRegion()),
		fm(root_region.getFieldmodule()),
		glyphmodule(context.getGlyphmodule()),
		materialModule(context.getMaterialmodule()),
		scene(0)
	{
		scene = root_region.getScene();
		EXPECT_TRUE(fm.isValid());
		EXPECT_TRUE(glyphmodule.isValid());
		EXPECT_EQ(OK, glyphmodule.defineStandardGlyphs());
		EXPECT_EQ(OK, materialModule.defineStandardMaterials());
		EXPECT_TRUE(scene.isValid());
	}

	~ZincTestSetupCpp()
	{
	}
};

#endif // __ZINCTESTSETUPCPP_HPP__
