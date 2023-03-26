/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/core.h>
#include <opencmiss/zinc/glyph.h>
#include <opencmiss/zinc/material.h>

#include <opencmiss/zinc/material.hpp>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

TEST(cmzn_glyph_axes, create)
{
	ZincTestSetup zinc;

	cmzn_glyph_id axisGlyph = cmzn_glyphmodule_find_glyph_by_glyph_shape_type(zinc.glyphmodule, CMZN_GLYPH_SHAPE_TYPE_AXIS);
	EXPECT_NE(static_cast<cmzn_glyph *>(0), axisGlyph);
	cmzn_glyph_id axes = cmzn_glyphmodule_create_glyph_axes(zinc.glyphmodule, axisGlyph, 0.1);
	EXPECT_NE(static_cast<cmzn_glyph_id>(0), axes);

	cmzn_glyph_destroy(&axes);
	cmzn_glyph_destroy(&axisGlyph);
}

TEST(ZincGlyphAxes, create)
{
	ZincTestSetupCpp zinc;

	Glyph axisGlyph = zinc.glyphmodule.findGlyphByGlyphShapeType(Glyph::SHAPE_TYPE_AXIS);
	EXPECT_TRUE(axisGlyph.isValid());
	GlyphAxes axes = zinc.glyphmodule.createGlyphAxes(axisGlyph, 0.1);
	EXPECT_TRUE(axes.isValid());
}

TEST(cmzn_glyph_axes, cast)
{
	ZincTestSetup zinc;

	cmzn_glyph_id glyph = cmzn_glyphmodule_find_glyph_by_name(zinc.glyphmodule, "axes");
	EXPECT_NE(static_cast<cmzn_glyph *>(0), glyph);
	cmzn_glyph_axes_id axes = cmzn_glyph_cast_axes(glyph);
	EXPECT_NE(static_cast<cmzn_glyph_axes *>(0), axes);

	double axisWidth = cmzn_glyph_axes_get_axis_width(axes);
	ASSERT_DOUBLE_EQ(0.1, axisWidth);

	ASSERT_EQ(glyph, cmzn_glyph_axes_base_cast(axes));

	cmzn_glyph_axes_destroy(&axes);
	cmzn_glyph_destroy(&glyph);
}

TEST(ZincGlyphAxes, cast)
{
	ZincTestSetupCpp zinc;

	Glyph glyph = zinc.glyphmodule.findGlyphByName("axes");
	EXPECT_TRUE(glyph.isValid());

	GlyphAxes axes = glyph.castAxes();
	EXPECT_TRUE(axes.isValid());

	double axisWidth = axes.getAxisWidth();
	ASSERT_DOUBLE_EQ(0.1, axisWidth);

	// try any base class API
	EXPECT_EQ(CMZN_OK, axes.setManaged(true));
}

TEST(cmzn_glyph_axes, valid_attributes)
{
	ZincTestSetup zinc;

	cmzn_glyph_id axisGlyph = cmzn_glyphmodule_find_glyph_by_name(zinc.glyphmodule, "axis");
	EXPECT_NE(static_cast<cmzn_glyph *>(0), axisGlyph);
	cmzn_glyph_id glyph = cmzn_glyphmodule_create_glyph_axes(zinc.glyphmodule, axisGlyph, 0.1);
	cmzn_glyph_axes_id axes = cmzn_glyph_cast_axes(glyph);
	EXPECT_NE(static_cast<cmzn_glyph_axes *>(0), axes);

	double axisWidth = cmzn_glyph_axes_get_axis_width(axes);
	ASSERT_DOUBLE_EQ(0.1, axisWidth);
	EXPECT_EQ(CMZN_OK, cmzn_glyph_axes_set_axis_width(axes, 0.25));
	axisWidth = cmzn_glyph_axes_get_axis_width(axes);
	ASSERT_DOUBLE_EQ(0.25, axisWidth);

	const char *axisLabels[3] = { "A", "B", "C" };
	cmzn_materialmodule_id materialModule = cmzn_context_get_materialmodule(zinc.context);
	cmzn_material_id red = cmzn_materialmodule_find_material_by_name(materialModule, "red");
	EXPECT_NE(static_cast<cmzn_material *>(0), red);

	for (int i = 1; i <= 3; ++i)
	{
		EXPECT_EQ(static_cast<char *>(0), cmzn_glyph_axes_get_axis_label(axes, i));
		EXPECT_EQ(CMZN_OK, cmzn_glyph_axes_set_axis_label(axes, i, axisLabels[i - 1]));
		char *label = cmzn_glyph_axes_get_axis_label(axes, i);
		EXPECT_STREQ(axisLabels[i - 1], label);
		cmzn_deallocate(label);

		cmzn_material_id material = cmzn_glyph_axes_get_axis_material(axes, i);
		EXPECT_EQ(static_cast<cmzn_material *>(0), material);
		EXPECT_EQ(CMZN_OK, cmzn_glyph_axes_set_axis_material(axes, i, red));
		material = cmzn_glyph_axes_get_axis_material(axes, i);
		EXPECT_EQ(red, material);
		cmzn_material_destroy(&material);
	}
	// check can clear label
	EXPECT_EQ(CMZN_OK, cmzn_glyph_axes_set_axis_label(axes, 1, 0));

	cmzn_material_destroy(&red);
	cmzn_materialmodule_destroy(&materialModule);
	cmzn_glyph_axes_destroy(&axes);
	cmzn_glyph_destroy(&glyph);
	cmzn_glyph_destroy(&axisGlyph);
}

TEST(ZincGlyphAxes, valid_attributes)
{
	ZincTestSetupCpp zinc;

	Glyph axisGlyph = zinc.glyphmodule.findGlyphByName("axis");
	EXPECT_TRUE(axisGlyph.isValid());
	GlyphAxes axes = zinc.glyphmodule.createGlyphAxes(axisGlyph, 0.1);
	EXPECT_TRUE(axes.isValid());

	double axisWidth = axes.getAxisWidth();
	ASSERT_DOUBLE_EQ(0.1, axisWidth);
	EXPECT_EQ(CMZN_OK, axes.setAxisWidth(0.25));
	axisWidth = axes.getAxisWidth();
	ASSERT_DOUBLE_EQ(0.25, axisWidth);

	Materialmodule materialModule = zinc.context.getMaterialmodule();
	Material red = materialModule.findMaterialByName("red");
	EXPECT_TRUE(red.isValid());

	const char *axisLabels[3] = { "A", "B", "C" };
	for (int i = 1; i <= 3; ++i)
	{
		EXPECT_EQ(static_cast<char *>(0), axes.getAxisLabel(i));
		EXPECT_EQ(CMZN_OK, axes.setAxisLabel(i, axisLabels[i - 1]));
		char *label = axes.getAxisLabel(i);
		EXPECT_STREQ(axisLabels[i - 1], label);
		cmzn_deallocate(label);

		Material material = axes.getAxisMaterial(i);
		EXPECT_FALSE(material.isValid());
		EXPECT_EQ(CMZN_OK, axes.setAxisMaterial(i, red));
		material = axes.getAxisMaterial(i);
		EXPECT_EQ(red.getId(), material.getId());
	}
	// check can clear label
	EXPECT_EQ(CMZN_OK, axes.setAxisLabel(1, 0));
}

TEST(cmzn_glyph_axes, invalid_attributes)
{
	ZincTestSetup zinc;

	cmzn_glyph_id axisGlyph = cmzn_glyphmodule_find_glyph_by_name(zinc.glyphmodule, "axis");
	EXPECT_NE(static_cast<cmzn_glyph *>(0), axisGlyph);
	EXPECT_EQ(static_cast<cmzn_glyph *>(0), cmzn_glyphmodule_create_glyph_axes(0, axisGlyph, 0.1));
	EXPECT_EQ(static_cast<cmzn_glyph *>(0), cmzn_glyphmodule_create_glyph_axes(zinc.glyphmodule, 0, 0.1));
	EXPECT_EQ(static_cast<cmzn_glyph *>(0), cmzn_glyphmodule_create_glyph_axes(zinc.glyphmodule, axisGlyph, -0.1));
	cmzn_glyph_id glyph = cmzn_glyphmodule_create_glyph_axes(zinc.glyphmodule, axisGlyph, 0.1);
	cmzn_glyph_axes_id axes = cmzn_glyph_cast_axes(glyph);
	EXPECT_NE(static_cast<cmzn_glyph_axes *>(0), axes);

	double axisWidth = cmzn_glyph_axes_get_axis_width(0);
	ASSERT_DOUBLE_EQ(0.0, axisWidth);
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_glyph_axes_set_axis_width(0, 0.25));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_glyph_axes_set_axis_width(axes, -0.25));

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_glyph_axes_set_axis_label(0, 1, "X"));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_glyph_axes_set_axis_label(axes, 0, "X"));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_glyph_axes_set_axis_label(axes, 4, "X"));
	EXPECT_EQ(CMZN_OK, cmzn_glyph_axes_set_axis_label(axes, 1, "X"));

	EXPECT_EQ(static_cast<char *>(0), cmzn_glyph_axes_get_axis_label(0, 1));
	EXPECT_EQ(static_cast<char *>(0), cmzn_glyph_axes_get_axis_label(axes, 0));
	EXPECT_EQ(static_cast<char *>(0), cmzn_glyph_axes_get_axis_label(axes, 4));

	cmzn_materialmodule_id materialModule = cmzn_context_get_materialmodule(zinc.context);
	cmzn_material_id red = cmzn_materialmodule_find_material_by_name(materialModule, "red");
	EXPECT_NE(static_cast<cmzn_material *>(0), red);

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_glyph_axes_set_axis_material(0, 1, red));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_glyph_axes_set_axis_material(axes, 0, red));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_glyph_axes_set_axis_material(axes, 4, red));
	EXPECT_EQ(CMZN_OK, cmzn_glyph_axes_set_axis_material(axes, 1, red));

	EXPECT_EQ(static_cast<cmzn_material *>(0), cmzn_glyph_axes_get_axis_material(0, 1));
	EXPECT_EQ(static_cast<cmzn_material *>(0), cmzn_glyph_axes_get_axis_material(axes, 0));
	EXPECT_EQ(static_cast<cmzn_material *>(0), cmzn_glyph_axes_get_axis_material(axes, 4));

	cmzn_material_destroy(&red);
	cmzn_materialmodule_destroy(&materialModule);
	cmzn_glyph_axes_destroy(&axes);
	cmzn_glyph_destroy(&glyph);
	cmzn_glyph_destroy(&axisGlyph);
}

TEST(ZincGlyphAxes, invalid_attributes)
{
	ZincTestSetupCpp zinc;

	Glyph axisGlyph = zinc.glyphmodule.findGlyphByName("axis");
	EXPECT_TRUE(axisGlyph.isValid());
	GlyphAxes axes;
	axes = zinc.glyphmodule.createGlyphAxes(Glyph(), 0.1);
	EXPECT_FALSE(axes.isValid());
	axes = zinc.glyphmodule.createGlyphAxes(axisGlyph, -0.1);
	EXPECT_FALSE(axes.isValid());
	axes = zinc.glyphmodule.createGlyphAxes(axisGlyph, 0.1);
	EXPECT_TRUE(axes.isValid());

	GlyphAxes noAxes;
	double axisWidth = noAxes.getAxisWidth();
	ASSERT_DOUBLE_EQ(0.0, axisWidth);
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, noAxes.setAxisWidth(0.25));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, axes.setAxisWidth(-0.25));

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, noAxes.setAxisLabel(1, "X"));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, axes.setAxisLabel(0, "X"));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, axes.setAxisLabel(4, "X"));
	EXPECT_EQ(CMZN_OK, axes.setAxisLabel(1, "X"));

	EXPECT_EQ(static_cast<char *>(0), noAxes.getAxisLabel(1));
	EXPECT_EQ(static_cast<char *>(0), axes.getAxisLabel(0));
	EXPECT_EQ(static_cast<char *>(0), axes.getAxisLabel(4));

	Materialmodule materialModule = zinc.context.getMaterialmodule();
	Material red = materialModule.findMaterialByName("red");
	EXPECT_TRUE(red.isValid());

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, noAxes.setAxisMaterial(1, red));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, axes.setAxisMaterial(0, red));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, axes.setAxisMaterial(4, red));
	EXPECT_EQ(CMZN_OK, axes.setAxisMaterial(1, red));

	Material material;
	material = noAxes.getAxisMaterial(1);
	EXPECT_FALSE(material.isValid());
	material = axes.getAxisMaterial(0);
	EXPECT_FALSE(material.isValid());
	material = axes.getAxisMaterial(4);
	EXPECT_FALSE(material.isValid());
}
