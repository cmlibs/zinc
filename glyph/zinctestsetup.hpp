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
#include <zinc/graphicsmaterial.h>
#include <zinc/scene.h>

class ZincTestSetup
{
public:
	cmzn_context_id context;
	cmzn_region_id root_region;
	cmzn_fieldmodule_id fm;
	cmzn_graphics_module_id gm;
	cmzn_glyphmodule_id glyphmodule;
	cmzn_graphics_material_module_id material_module;
	cmzn_scene_id scene;

	ZincTestSetup() :
		context(cmzn_context_create("test")),
		root_region(cmzn_context_get_default_region(context)),
		fm(cmzn_region_get_fieldmodule(root_region)),
		gm(cmzn_context_get_graphics_module(context)),
		glyphmodule(cmzn_graphics_module_get_glyphmodule(gm)),
		material_module(cmzn_graphics_module_get_material_module(gm)),
		scene(0)
	{
		scene = cmzn_graphics_module_get_scene(gm, root_region);
		EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), fm);
		EXPECT_NE(static_cast<cmzn_graphics_module *>(0), gm);
		EXPECT_NE(static_cast<cmzn_glyphmodule *>(0), glyphmodule);
		EXPECT_EQ(CMZN_OK, cmzn_glyphmodule_define_standard_glyphs(glyphmodule));
		EXPECT_EQ(CMZN_OK, cmzn_graphics_material_module_define_standard_materials(material_module));
		EXPECT_NE(static_cast<cmzn_scene *>(0), scene);
	}

	~ZincTestSetup()
	{
		cmzn_scene_destroy(&scene);
		cmzn_graphics_material_module_destroy(&material_module);
		cmzn_glyphmodule_destroy(&glyphmodule);
		cmzn_graphics_module_destroy(&gm);
		cmzn_fieldmodule_destroy(&fm);
		cmzn_region_destroy(&root_region);
		cmzn_context_destroy(&context);
	}
};

#endif // __ZINCTESTSETUP_HPP__
