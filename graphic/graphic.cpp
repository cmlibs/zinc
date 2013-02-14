
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/region.h>
#include <zinc/fieldmodule.h>
#include <zinc/rendition.h>
#include <zinc/field.h>
#include <zinc/fieldconstant.h>
#include <zinc/graphic.h>

TEST(Cmiss_graphic_api, set_use_element_type)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);
	Cmiss_graphics_module_id gm = Cmiss_context_get_default_graphics_module(context);
	EXPECT_EQ(CMISS_OK, Cmiss_graphics_module_enable_renditions(gm, root_region));
	Cmiss_rendition_id ren = Cmiss_graphics_module_get_rendition(gm, root_region);

	EXPECT_NE(static_cast<Cmiss_field_module *>(0), fm);
	EXPECT_NE(static_cast<Cmiss_graphics_module *>(0), gm);
	EXPECT_NE(static_cast<Cmiss_rendition *>(0), ren);

	Cmiss_graphic_id is = Cmiss_rendition_create_graphic(ren, CMISS_GRAPHIC_ISO_SURFACES);

	EXPECT_NE(static_cast<Cmiss_graphic *>(0), is);

	int result = Cmiss_graphic_set_use_element_type(is, CMISS_GRAPHIC_USE_ELEMENT_FACES);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_graphic_destroy(&is);
	Cmiss_rendition_destroy(&ren);
	Cmiss_graphics_module_destroy(&gm);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}
