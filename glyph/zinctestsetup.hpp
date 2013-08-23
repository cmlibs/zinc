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
	Cmiss_context_id context;
	Cmiss_region_id root_region;
	Cmiss_field_module_id fm;
	Cmiss_graphics_module_id gm;
	Cmiss_glyph_module_id glyph_module;
	Cmiss_graphics_material_module_id material_module;
	Cmiss_scene_id scene;

	ZincTestSetup() :
		context(Cmiss_context_create("test")),
		root_region(Cmiss_context_get_default_region(context)),
		fm(Cmiss_region_get_field_module(root_region)),
		gm(Cmiss_context_get_graphics_module(context)),
		glyph_module(Cmiss_graphics_module_get_glyph_module(gm)),
		material_module(Cmiss_graphics_module_get_material_module(gm)),
		scene(0)
	{
		scene = Cmiss_graphics_module_get_scene(gm, root_region);
		EXPECT_NE(static_cast<Cmiss_field_module *>(0), fm);
		EXPECT_NE(static_cast<Cmiss_graphics_module *>(0), gm);
		EXPECT_NE(static_cast<Cmiss_glyph_module *>(0), glyph_module);
		EXPECT_EQ(CMISS_OK, Cmiss_glyph_module_define_standard_glyphs(glyph_module));
		EXPECT_EQ(CMISS_OK, Cmiss_graphics_material_module_define_standard_materials(material_module));
		EXPECT_NE(static_cast<Cmiss_scene *>(0), scene);
	}

	~ZincTestSetup()
	{
		Cmiss_scene_destroy(&scene);
		Cmiss_graphics_material_module_destroy(&material_module);
		Cmiss_glyph_module_destroy(&glyph_module);
		Cmiss_graphics_module_destroy(&gm);
		Cmiss_field_module_destroy(&fm);
		Cmiss_region_destroy(&root_region);
		Cmiss_context_destroy(&context);
	}
};

#endif // __ZINCTESTSETUP_HPP__
