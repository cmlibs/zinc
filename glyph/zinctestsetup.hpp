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

#include <zinc/status.h>
#include <zinc/context.h>
#include <zinc/region.h>
#include <zinc/fieldmodule.h>
#include <zinc/glyph.h>
#include <zinc/material.h>
#include <zinc/scene.h>

class ZincTestSetup
{
public:
	cmzn_context_id context;
	cmzn_region_id root_region;
	cmzn_fieldmodule_id fm;
	cmzn_glyphmodule_id glyphmodule;
	cmzn_materialmodule_id materialmodule;
	cmzn_scene_id scene;

	ZincTestSetup() :
		context(cmzn_context_create("test")),
		root_region(cmzn_context_get_default_region(context)),
		fm(cmzn_region_get_fieldmodule(root_region)),
		glyphmodule(cmzn_context_get_glyphmodule(context)),
		materialmodule(cmzn_context_get_materialmodule(context)),
		scene(0)
	{
		scene = cmzn_region_get_scene(root_region);
		EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), fm);
		EXPECT_NE(static_cast<cmzn_glyphmodule *>(0), glyphmodule);
		EXPECT_EQ(CMZN_OK, cmzn_glyphmodule_define_standard_glyphs(glyphmodule));
		EXPECT_EQ(CMZN_OK, cmzn_materialmodule_define_standard_materials(materialmodule));
		EXPECT_NE(static_cast<cmzn_scene *>(0), scene);
	}

	~ZincTestSetup()
	{
		cmzn_scene_destroy(&scene);
		cmzn_materialmodule_destroy(&materialmodule);
		cmzn_glyphmodule_destroy(&glyphmodule);
		cmzn_fieldmodule_destroy(&fm);
		cmzn_region_destroy(&root_region);
		cmzn_context_destroy(&context);
	}
};

#endif // __ZINCTESTSETUP_HPP__
