/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ZINCTESTSETUP_HPP__
#define __ZINCTESTSETUP_HPP__

#include <gtest/gtest.h>

#include <opencmiss/zinc/status.h>
#include <opencmiss/zinc/context.h>
#include <opencmiss/zinc/region.h>
#include <opencmiss/zinc/fieldmodule.h>
#include <opencmiss/zinc/glyph.h>
#include <opencmiss/zinc/scene.h>

class ZincTestSetup
{
public:
	cmzn_context_id context;
	cmzn_region_id root_region;
	cmzn_fieldmodule_id fm;
	cmzn_glyphmodule_id glyphmodule;
	cmzn_scene_id scene;

	ZincTestSetup() :
		context(cmzn_context_create("test")),
		root_region(cmzn_context_get_default_region(context)),
		fm(cmzn_region_get_fieldmodule(root_region)),
		glyphmodule(cmzn_context_get_glyphmodule(context)),
		scene(0)
	{
		scene = cmzn_region_get_scene(root_region);
		EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), fm);
		EXPECT_NE(static_cast<cmzn_glyphmodule *>(0), glyphmodule);
		EXPECT_EQ(CMZN_OK, cmzn_glyphmodule_define_standard_glyphs(glyphmodule));
		EXPECT_NE(static_cast<cmzn_scene *>(0), scene);
	}

	~ZincTestSetup()
	{
		cmzn_scene_destroy(&scene);
		cmzn_glyphmodule_destroy(&glyphmodule);
		cmzn_fieldmodule_destroy(&fm);
		cmzn_region_destroy(&root_region);
		cmzn_context_destroy(&context);
	}
};

#endif // __ZINCTESTSETUP_HPP__
