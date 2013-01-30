
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

TEST(Cmiss_rendition_create_graphic, iso_surface)
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

	Cmiss_graphic_destroy(&is);
	Cmiss_rendition_destroy(&ren);
	Cmiss_graphics_module_destroy(&gm);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_rendition_create_graphic, cast_iso_surface)
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

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(ren, CMISS_GRAPHIC_ISO_SURFACES);
	Cmiss_graphic_iso_surface_id is = Cmiss_graphic_cast_iso_surface(gr);

	EXPECT_NE(static_cast<Cmiss_graphic_iso_surface *>(0), is);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_destroy(&is));
	Cmiss_graphic_destroy(&gr);
	Cmiss_rendition_destroy(&ren);
	Cmiss_graphics_module_destroy(&gm);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_graphic_iso_surface, base_cast)
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

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(ren, CMISS_GRAPHIC_ISO_SURFACES);
	Cmiss_graphic_iso_surface_id is = Cmiss_graphic_cast_iso_surface(gr);

	EXPECT_NE(static_cast<Cmiss_graphic_iso_surface *>(0), is);

	// No need to destroy the return handle
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), Cmiss_graphic_iso_surface_base_cast(is));

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_destroy(&is));
	Cmiss_graphic_destroy(&gr);
	Cmiss_rendition_destroy(&ren);
	Cmiss_graphics_module_destroy(&gm);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_graphic_iso_surface, set_iso_scalar_field)
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

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(ren, CMISS_GRAPHIC_ISO_SURFACES);
	Cmiss_graphic_iso_surface_id is = Cmiss_graphic_cast_iso_surface(gr);

	EXPECT_NE(static_cast<Cmiss_graphic_iso_surface *>(0), is);

	double values[] = {1.0};
	Cmiss_field_id c = Cmiss_field_module_create_constant(fm, 1, values);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_set_iso_scalar_field(is, c));

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_destroy(&is));
	Cmiss_graphic_destroy(&gr);
	Cmiss_rendition_destroy(&ren);
	Cmiss_graphics_module_destroy(&gm);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_graphic_iso_surface, set_iso_values)
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

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(ren, CMISS_GRAPHIC_ISO_SURFACES);
	Cmiss_graphic_iso_surface_id is = Cmiss_graphic_cast_iso_surface(gr);

	EXPECT_NE(static_cast<Cmiss_graphic_iso_surface *>(0), is);

	int num = 3;
	double values[] = {1.0, 1.2, 3.4};
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_set_iso_values(is, num, values));

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_destroy(&is));
	Cmiss_graphic_destroy(&gr);
	Cmiss_rendition_destroy(&ren);
	Cmiss_graphics_module_destroy(&gm);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_graphic_iso_surface, set_iso_values_null)
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

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(ren, CMISS_GRAPHIC_ISO_SURFACES);
	Cmiss_graphic_iso_surface_id is = Cmiss_graphic_cast_iso_surface(gr);

	EXPECT_NE(static_cast<Cmiss_graphic_iso_surface *>(0), is);

	int num = 3;
	double values[] = {1.0, 1.2, 3.4};
	EXPECT_NE(CMISS_OK, Cmiss_graphic_iso_surface_set_iso_values(0, num, values));
	EXPECT_NE(CMISS_OK, Cmiss_graphic_iso_surface_set_iso_values(is, 5, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_set_iso_values(is, -1, 0));

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_destroy(&is));
	Cmiss_graphic_destroy(&gr);
	Cmiss_rendition_destroy(&ren);
	Cmiss_graphics_module_destroy(&gm);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_graphic_iso_surface, set_iso_range)
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

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(ren, CMISS_GRAPHIC_ISO_SURFACES);
	Cmiss_graphic_iso_surface_id is = Cmiss_graphic_cast_iso_surface(gr);

	EXPECT_NE(static_cast<Cmiss_graphic_iso_surface *>(0), is);

	int num = 6;
	double first = 0.1, last = 0.55;
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_set_iso_range(is, num, first, last));

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_destroy(&is));
	Cmiss_graphic_destroy(&gr);
	Cmiss_rendition_destroy(&ren);
	Cmiss_graphics_module_destroy(&gm);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

