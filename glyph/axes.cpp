
#include <gtest/gtest.h>

#include <zinc/core.h>
#include <zinc/glyph.h>
#include <zinc/graphicsmaterial.h>

#include <zinc/graphicsmaterial.hpp>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

TEST(Cmiss_glyph_axes, create)
{
	ZincTestSetup zinc;

	Cmiss_glyph_id axisGlyph = Cmiss_glyph_module_find_glyph_by_type(zinc.glyph_module, CMISS_GLYPH_AXIS);
	EXPECT_NE(static_cast<Cmiss_glyph *>(0), axisGlyph);
	Cmiss_glyph_axes_id axes = Cmiss_glyph_module_create_axes(zinc.glyph_module, axisGlyph, 0.1);
	EXPECT_NE(static_cast<Cmiss_glyph_axes *>(0), axes);

	Cmiss_glyph_destroy(&axisGlyph);
	Cmiss_glyph_axes_destroy(&axes);
}

TEST(ZincGlyphAxes, create)
{
	ZincTestSetupCpp zinc;

	Glyph axisGlyph = zinc.glyphModule.findGlyphByType(Glyph::AXIS);
	EXPECT_TRUE(axisGlyph.isValid());
	GlyphAxes axes = zinc.glyphModule.createAxes(axisGlyph, 0.1);
	EXPECT_TRUE(axes.isValid());
}

TEST(Cmiss_glyph_axes, cast)
{
	ZincTestSetup zinc;

	Cmiss_glyph_id glyph = Cmiss_glyph_module_find_glyph_by_name(zinc.glyph_module, "axes");
	EXPECT_NE(static_cast<Cmiss_glyph *>(0), glyph);
	Cmiss_glyph_axes_id axes = Cmiss_glyph_cast_axes(glyph);
	EXPECT_NE(static_cast<Cmiss_glyph_axes *>(0), axes);

	double axisWidth = Cmiss_glyph_axes_get_axis_width(axes);
	ASSERT_DOUBLE_EQ(0.1, axisWidth);

	ASSERT_EQ(glyph, Cmiss_glyph_axes_base_cast(axes));

	Cmiss_glyph_axes_destroy(&axes);
	Cmiss_glyph_destroy(&glyph);
}

TEST(ZincGlyphAxes, cast)
{
	ZincTestSetupCpp zinc;

	Glyph glyph = zinc.glyphModule.findGlyphByName("axes");
	EXPECT_TRUE(glyph.isValid());
	GlyphAxes axes = glyph;
	EXPECT_TRUE(axes.isValid());

	double axisWidth = axes.getAxisWidth();
	ASSERT_DOUBLE_EQ(0.1, axisWidth);

	// try any base class API
	EXPECT_EQ(CMISS_OK, axes.setManaged(true));
}

TEST(Cmiss_glyph_axes, valid_attributes)
{
	ZincTestSetup zinc;

	Cmiss_glyph_id axisGlyph = Cmiss_glyph_module_find_glyph_by_name(zinc.glyph_module, "axis");
	EXPECT_NE(static_cast<Cmiss_glyph *>(0), axisGlyph);
	Cmiss_glyph_axes_id axes = Cmiss_glyph_module_create_axes(zinc.glyph_module, axisGlyph, 0.1);
	EXPECT_NE(static_cast<Cmiss_glyph_axes *>(0), axes);

	double axisWidth = Cmiss_glyph_axes_get_axis_width(axes);
	ASSERT_DOUBLE_EQ(0.1, axisWidth);
	EXPECT_EQ(CMISS_OK, Cmiss_glyph_axes_set_axis_width(axes, 0.25));
	axisWidth = Cmiss_glyph_axes_get_axis_width(axes);
	ASSERT_DOUBLE_EQ(0.25, axisWidth);

	const char *axisLabels[3] = { "A", "B", "C" };
	Cmiss_graphics_material_module_id materialModule = Cmiss_graphics_module_get_material_module(zinc.gm);
	Cmiss_graphics_material_id red = Cmiss_graphics_material_module_find_material_by_name(materialModule, "red");
	EXPECT_NE(static_cast<Cmiss_graphics_material *>(0), red);

	for (int i = 1; i <= 3; ++i)
	{
		EXPECT_EQ(static_cast<char *>(0), Cmiss_glyph_axes_get_axis_label(axes, i));
		EXPECT_EQ(CMISS_OK, Cmiss_glyph_axes_set_axis_label(axes, i, axisLabels[i - 1]));
		char *label = Cmiss_glyph_axes_get_axis_label(axes, i);
		EXPECT_STREQ(axisLabels[i - 1], label);
		Cmiss_deallocate(label);

		Cmiss_graphics_material_id material = Cmiss_glyph_axes_get_axis_material(axes, i);
		EXPECT_EQ(static_cast<Cmiss_graphics_material *>(0), material);
		EXPECT_EQ(CMISS_OK, Cmiss_glyph_axes_set_axis_material(axes, i, red));
		material = Cmiss_glyph_axes_get_axis_material(axes, i);
		EXPECT_EQ(red, material);
		Cmiss_graphics_material_destroy(&material);
	}
	// check can clear label
	EXPECT_EQ(CMISS_OK, Cmiss_glyph_axes_set_axis_label(axes, 1, 0));

	Cmiss_graphics_material_destroy(&red);
	Cmiss_graphics_material_module_destroy(&materialModule);
	Cmiss_glyph_destroy(&axisGlyph);
	Cmiss_glyph_axes_destroy(&axes);
}

TEST(ZincGlyphAxes, valid_attributes)
{
	ZincTestSetupCpp zinc;

	Glyph axisGlyph = zinc.glyphModule.findGlyphByName("axis");
	EXPECT_TRUE(axisGlyph.isValid());
	GlyphAxes axes = zinc.glyphModule.createAxes(axisGlyph, 0.1);
	EXPECT_TRUE(axes.isValid());

	double axisWidth = axes.getAxisWidth();
	ASSERT_DOUBLE_EQ(0.1, axisWidth);
	EXPECT_EQ(CMISS_OK, axes.setAxisWidth(0.25));
	axisWidth = axes.getAxisWidth();
	ASSERT_DOUBLE_EQ(0.25, axisWidth);

	GraphicsMaterialModule materialModule = zinc.gm.getMaterialModule();
	GraphicsMaterial red = materialModule.findMaterialByName("red");
	EXPECT_TRUE(red.isValid());

	const char *axisLabels[3] = { "A", "B", "C" };
	for (int i = 1; i <= 3; ++i)
	{
		EXPECT_EQ(static_cast<char *>(0), axes.getAxisLabel(i));
		EXPECT_EQ(CMISS_OK, axes.setAxisLabel(i, axisLabels[i - 1]));
		char *label = axes.getAxisLabel(i);
		EXPECT_STREQ(axisLabels[i - 1], label);
		Cmiss_deallocate(label);

		GraphicsMaterial material = axes.getAxisMaterial(i);
		EXPECT_FALSE(material.isValid());
		EXPECT_EQ(CMISS_OK, axes.setAxisMaterial(i, red));
		material = axes.getAxisMaterial(i);
		EXPECT_EQ(red.getId(), material.getId());
	}
	// check can clear label
	EXPECT_EQ(CMISS_OK, axes.setAxisLabel(1, 0));
}

TEST(Cmiss_glyph_axes, invalid_attributes)
{
	ZincTestSetup zinc;

	Cmiss_glyph_id axisGlyph = Cmiss_glyph_module_find_glyph_by_name(zinc.glyph_module, "axis");
	EXPECT_NE(static_cast<Cmiss_glyph *>(0), axisGlyph);
	EXPECT_EQ(static_cast<Cmiss_glyph_axes *>(0), Cmiss_glyph_module_create_axes(0, axisGlyph, 0.1));
	EXPECT_EQ(static_cast<Cmiss_glyph_axes *>(0), Cmiss_glyph_module_create_axes(zinc.glyph_module, 0, 0.1));
	EXPECT_EQ(static_cast<Cmiss_glyph_axes *>(0), Cmiss_glyph_module_create_axes(zinc.glyph_module, axisGlyph, -0.1));
	Cmiss_glyph_axes_id axes = Cmiss_glyph_module_create_axes(zinc.glyph_module, axisGlyph, 0.1);
	EXPECT_NE(static_cast<Cmiss_glyph_axes *>(0), axes);

	double axisWidth = Cmiss_glyph_axes_get_axis_width(0);
	ASSERT_DOUBLE_EQ(0.0, axisWidth);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_glyph_axes_set_axis_width(0, 0.25));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_glyph_axes_set_axis_width(axes, -0.25));

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_glyph_axes_set_axis_label(0, 1, "X"));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_glyph_axes_set_axis_label(axes, 0, "X"));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_glyph_axes_set_axis_label(axes, 4, "X"));
	EXPECT_EQ(CMISS_OK, Cmiss_glyph_axes_set_axis_label(axes, 1, "X"));

	EXPECT_EQ(static_cast<char *>(0), Cmiss_glyph_axes_get_axis_label(0, 1));
	EXPECT_EQ(static_cast<char *>(0), Cmiss_glyph_axes_get_axis_label(axes, 0));
	EXPECT_EQ(static_cast<char *>(0), Cmiss_glyph_axes_get_axis_label(axes, 4));

	Cmiss_graphics_material_module_id materialModule = Cmiss_graphics_module_get_material_module(zinc.gm);
	Cmiss_graphics_material_id red = Cmiss_graphics_material_module_find_material_by_name(materialModule, "red");
	EXPECT_NE(static_cast<Cmiss_graphics_material *>(0), red);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_glyph_axes_set_axis_material(0, 1, red));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_glyph_axes_set_axis_material(axes, 0, red));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_glyph_axes_set_axis_material(axes, 4, red));
	EXPECT_EQ(CMISS_OK, Cmiss_glyph_axes_set_axis_material(axes, 1, red));

	EXPECT_EQ(static_cast<Cmiss_graphics_material *>(0), Cmiss_glyph_axes_get_axis_material(0, 1));
	EXPECT_EQ(static_cast<Cmiss_graphics_material *>(0), Cmiss_glyph_axes_get_axis_material(axes, 0));
	EXPECT_EQ(static_cast<Cmiss_graphics_material *>(0), Cmiss_glyph_axes_get_axis_material(axes, 4));

	Cmiss_graphics_material_destroy(&red);
	Cmiss_graphics_material_module_destroy(&materialModule);
	Cmiss_glyph_destroy(&axisGlyph);
	Cmiss_glyph_axes_destroy(&axes);
}

TEST(ZincGlyphAxes, invalid_attributes)
{
	ZincTestSetupCpp zinc;

	Glyph axisGlyph = zinc.glyphModule.findGlyphByName("axis");
	EXPECT_TRUE(axisGlyph.isValid());
	Glyph noGlyph;
	GlyphAxes axes;
	axes = zinc.glyphModule.createAxes(noGlyph, 0.1);
	EXPECT_FALSE(axes.isValid());
	axes = zinc.glyphModule.createAxes(axisGlyph, -0.1);
	EXPECT_FALSE(axes.isValid());
	axes = zinc.glyphModule.createAxes(axisGlyph, 0.1);
	EXPECT_TRUE(axes.isValid());

	GlyphAxes noAxes;
	double axisWidth = noAxes.getAxisWidth();
	ASSERT_DOUBLE_EQ(0.0, axisWidth);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, noAxes.setAxisWidth(0.25));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, axes.setAxisWidth(-0.25));

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, noAxes.setAxisLabel(1, "X"));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, axes.setAxisLabel(0, "X"));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, axes.setAxisLabel(4, "X"));
	EXPECT_EQ(CMISS_OK, axes.setAxisLabel(1, "X"));

	EXPECT_EQ(static_cast<char *>(0), noAxes.getAxisLabel(1));
	EXPECT_EQ(static_cast<char *>(0), axes.getAxisLabel(0));
	EXPECT_EQ(static_cast<char *>(0), axes.getAxisLabel(4));

	GraphicsMaterialModule materialModule = zinc.gm.getMaterialModule();
	GraphicsMaterial red = materialModule.findMaterialByName("red");
	EXPECT_TRUE(red.isValid());

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, noAxes.setAxisMaterial(1, red));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, axes.setAxisMaterial(0, red));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, axes.setAxisMaterial(4, red));
	EXPECT_EQ(CMISS_OK, axes.setAxisMaterial(1, red));

	GraphicsMaterial material;
	material = noAxes.getAxisMaterial(1);
	EXPECT_FALSE(material.isValid());
	material = axes.getAxisMaterial(0);
	EXPECT_FALSE(material.isValid());
	material = axes.getAxisMaterial(4);
	EXPECT_FALSE(material.isValid());
}
