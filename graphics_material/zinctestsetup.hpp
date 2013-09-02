#ifndef __ZINCTESTSETUP_HPP__
#define __ZINCTESTSETUP_HPP__

#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/context.h>
#include <zinc/region.h>
#include <zinc/fieldmodule.h>
#include <zinc/glyph.h>
#include <zinc/graphicsmodule.h>
#include <zinc/scene.h>

class ZincTestSetup
{
public:
	cmzn_context_id context;
	cmzn_region_id root_region;
	cmzn_field_module_id fm;
	cmzn_graphics_module_id gm;
	cmzn_glyph_module_id glyph_module;
	cmzn_scene_id scene;

	ZincTestSetup() :
		context(cmzn_context_create("test")),
		root_region(cmzn_context_get_default_region(context)),
		fm(cmzn_region_get_field_module(root_region)),
		gm(cmzn_context_get_graphics_module(context)),
		glyph_module(cmzn_graphics_module_get_glyph_module(gm)),
		scene(0)
	{
		scene = cmzn_graphics_module_get_scene(gm, root_region);
		EXPECT_NE(static_cast<cmzn_field_module *>(0), fm);
		EXPECT_NE(static_cast<cmzn_graphics_module *>(0), gm);
		EXPECT_NE(static_cast<cmzn_glyph_module *>(0), glyph_module);
		EXPECT_EQ(CMISS_OK, cmzn_glyph_module_define_standard_glyphs(glyph_module));
		EXPECT_NE(static_cast<cmzn_scene *>(0), scene);
	}

	~ZincTestSetup()
	{
		cmzn_scene_destroy(&scene);
		cmzn_glyph_module_destroy(&glyph_module);
		cmzn_graphics_module_destroy(&gm);
		cmzn_field_module_destroy(&fm);
		cmzn_region_destroy(&root_region);
		cmzn_context_destroy(&context);
	}
};

#endif // __ZINCTESTSETUP_HPP__
