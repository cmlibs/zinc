
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
#include <zinc/spectrum.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/graphic.hpp"
#include "zinc/fieldtypesconstant.hpp"
#include "zinc/font.hpp"

TEST(Cmiss_graphic_api, set_use_element_type)
{
	ZincTestSetup zinc;

	Cmiss_graphic_contours_id is = Cmiss_rendition_create_graphic_contours(zinc.ren);
	EXPECT_NE(static_cast<Cmiss_graphic_contours *>(0), is);

	int result = Cmiss_graphic_set_domain_type(Cmiss_graphic_contours_base_cast(is), CMISS_FIELD_DOMAIN_ELEMENTS_2D);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_graphic_contours_destroy(&is);
}

TEST(Cmiss_graphic_api, exterior)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_SURFACES);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	EXPECT_EQ(0, Cmiss_graphic_get_exterior(gr));
	int result = Cmiss_graphic_set_exterior(gr, 1);
	EXPECT_EQ(CMISS_OK, result);
	EXPECT_EQ(1, Cmiss_graphic_get_exterior(gr));

	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, face)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_SURFACES);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	EXPECT_EQ(CMISS_ELEMENT_FACE_ALL, Cmiss_graphic_get_face(gr));
	int result = Cmiss_graphic_set_face(gr, CMISS_ELEMENT_FACE_XI2_0);
	EXPECT_EQ(CMISS_OK, result);
	EXPECT_EQ(CMISS_ELEMENT_FACE_XI2_0, Cmiss_graphic_get_face(gr));

	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, coordinate_field)
{
	ZincTestSetup zinc;

	const double values[] = { 1.0, 2.0, 3.0 };
	Cmiss_field_id coordinate_field = Cmiss_field_module_create_constant(zinc.fm,
		sizeof(values)/sizeof(double), values);
	EXPECT_NE(static_cast<Cmiss_field *>(0), coordinate_field);

	Cmiss_graphic_id gr = Cmiss_graphic_points_base_cast(Cmiss_rendition_create_graphic_points(zinc.ren));
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_domain_type(gr, CMISS_FIELD_DOMAIN_NODES));

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_coordinate_field(gr, coordinate_field));

	// coordinate field cannot have more than 3 components
	const double values4[] = { 1.0, 2.0, 3.0, 4.0 };
	Cmiss_field_id bad_coordinate_field = Cmiss_field_module_create_constant(zinc.fm,
		sizeof(values4)/sizeof(double), values4);
	EXPECT_NE(static_cast<Cmiss_field *>(0), bad_coordinate_field);
	// previous coordinate field should be left unchanged
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_set_coordinate_field(gr, bad_coordinate_field));
	Cmiss_field_destroy(&bad_coordinate_field);

	Cmiss_field_id temp_coordinate_field = Cmiss_graphic_get_coordinate_field(gr);
	EXPECT_EQ(coordinate_field, temp_coordinate_field);
	Cmiss_field_destroy(&temp_coordinate_field);
	Cmiss_field_destroy(&coordinate_field);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_coordinate_field(gr, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_get_coordinate_field(gr));

	// check coordinate_field removed as no longer used
	Cmiss_field_iterator_id iter = Cmiss_field_module_create_field_iterator(zinc.fm);
	EXPECT_NE(static_cast<Cmiss_field_iterator *>(0), iter);
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_field_iterator_next(iter));
	Cmiss_field_iterator_destroy(&iter);

	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, data_field)
{
	ZincTestSetup zinc;

	double values[] = { 1.0, 2.0, 3.0 };
	Cmiss_field_id data_field = Cmiss_field_module_create_constant(zinc.fm,
		sizeof(values)/sizeof(double), values);
	EXPECT_NE(static_cast<Cmiss_field *>(0), data_field);

	Cmiss_graphic_id gr = Cmiss_graphic_points_base_cast(Cmiss_rendition_create_graphic_points(zinc.ren));
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_data_field(gr, data_field));

	Cmiss_field_id temp_data_field = Cmiss_graphic_get_data_field(gr);
	EXPECT_EQ(temp_data_field, data_field);
	Cmiss_field_destroy(&temp_data_field);
	Cmiss_field_destroy(&data_field);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_data_field(gr, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_get_data_field(gr));

	// check data_field removed as no longer used
	Cmiss_field_iterator_id iter = Cmiss_field_module_create_field_iterator(zinc.fm);
	EXPECT_NE(static_cast<Cmiss_field_iterator *>(0), iter);
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_field_iterator_next(iter));
	Cmiss_field_iterator_destroy(&iter);

	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, material)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_SURFACES);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	Cmiss_graphics_material_module_id material_module = Cmiss_graphics_module_get_material_module(zinc.gm);
	Cmiss_graphics_material_id default_material = Cmiss_graphics_material_module_get_default_material(material_module);
	Cmiss_graphics_material_id temp_material = Cmiss_graphic_get_material(gr);
	EXPECT_EQ(default_material, temp_material);
	Cmiss_graphics_material_destroy(&temp_material);
	Cmiss_graphics_material_destroy(&default_material);

	Cmiss_graphics_material_id material = Cmiss_graphics_material_module_create_material(material_module);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_material(gr, material));
	temp_material = Cmiss_graphic_get_material(gr);
	EXPECT_EQ(material, temp_material);
	Cmiss_graphics_material_destroy(&temp_material);
	Cmiss_graphics_material_destroy(&material);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_set_material(gr, 0));

	Cmiss_graphics_material_module_destroy(&material_module);
	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, selected_material)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_SURFACES);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	Cmiss_graphics_material_module_id material_module = Cmiss_graphics_module_get_material_module(zinc.gm);
	Cmiss_graphics_material_id default_selected_material = Cmiss_graphics_material_module_get_default_selected_material(material_module);
	Cmiss_graphics_material_id temp_selected_material = Cmiss_graphic_get_selected_material(gr);
	EXPECT_EQ(default_selected_material, temp_selected_material);
	Cmiss_graphics_material_destroy(&temp_selected_material);
	Cmiss_graphics_material_destroy(&default_selected_material);

	Cmiss_graphics_material_id selected_material = Cmiss_graphics_material_module_create_material(material_module);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_selected_material(gr, selected_material));
	temp_selected_material = Cmiss_graphic_get_selected_material(gr);
	EXPECT_EQ(selected_material, temp_selected_material);
	Cmiss_graphics_material_destroy(&temp_selected_material);
	Cmiss_graphics_material_destroy(&selected_material);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_set_selected_material(gr, 0));

	Cmiss_graphics_material_module_destroy(&material_module);
	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, spectrum)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_SURFACES);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	Cmiss_spectrum_id spectrum = Cmiss_graphics_module_create_spectrum(zinc.gm);
	EXPECT_NE(static_cast<Cmiss_spectrum *>(0), spectrum);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_spectrum(gr, spectrum));

	Cmiss_spectrum_id temp_spectrum = Cmiss_graphic_get_spectrum(gr);
	EXPECT_EQ(temp_spectrum, spectrum);
	Cmiss_spectrum_destroy(&temp_spectrum);
	Cmiss_spectrum_destroy(&spectrum);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_spectrum(gr, 0));
	EXPECT_EQ(static_cast<Cmiss_spectrum *>(0), Cmiss_graphic_get_spectrum(gr));

	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, subgroup_field)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_graphic_points_base_cast(Cmiss_rendition_create_graphic_points(zinc.ren));
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_domain_type(gr, CMISS_FIELD_DOMAIN_NODES));

	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_get_subgroup_field(gr));

	const double value = 1.0;
	Cmiss_field_id subgroup_field = Cmiss_field_module_create_constant(zinc.fm, 1, &value);
	EXPECT_NE(static_cast<Cmiss_field *>(0), subgroup_field);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_subgroup_field(gr, subgroup_field));

	// subgroup field must be scalar
	double values2[] = { 1.0, 2.0 };
	Cmiss_field_id bad_subgroup_field = Cmiss_field_module_create_constant(zinc.fm,
		sizeof(values2)/sizeof(double), values2);
	EXPECT_NE(static_cast<Cmiss_field *>(0), bad_subgroup_field);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_set_subgroup_field(gr, bad_subgroup_field));
	Cmiss_field_destroy(&bad_subgroup_field);

	// previous subgroup field should be left unchanged
	Cmiss_field_id temp_subgroup_field = Cmiss_graphic_get_subgroup_field(gr);
	EXPECT_EQ(subgroup_field, temp_subgroup_field);
	Cmiss_field_destroy(&temp_subgroup_field);
	Cmiss_field_destroy(&subgroup_field);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_subgroup_field(gr, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_get_subgroup_field(gr));

	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, tessellation)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_SURFACES);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	Cmiss_tessellation_module_id tessellation_module = Cmiss_graphics_module_get_tessellation_module(zinc.gm);
	Cmiss_tessellation_id default_tessellation = Cmiss_tessellation_module_get_default_tessellation(tessellation_module);
	Cmiss_tessellation_id temp_tessellation = Cmiss_graphic_get_tessellation(gr);
	EXPECT_EQ(default_tessellation, temp_tessellation);
	Cmiss_tessellation_destroy(&temp_tessellation);
	Cmiss_tessellation_destroy(&default_tessellation);

	Cmiss_tessellation_id tessellation = Cmiss_tessellation_module_create_tessellation(tessellation_module);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_tessellation(gr, tessellation));
	temp_tessellation = Cmiss_graphic_get_tessellation(gr);
	EXPECT_EQ(tessellation, temp_tessellation);
	Cmiss_tessellation_destroy(&temp_tessellation);
	Cmiss_tessellation_destroy(&tessellation);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_tessellation(gr, 0));
	EXPECT_EQ(static_cast<Cmiss_tessellation_id>(0), Cmiss_graphic_get_tessellation(gr));

	Cmiss_tessellation_module_destroy(&tessellation_module);
	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, texture_coordinate_field)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_SURFACES);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	const double values[] = { 1.0, 2.0, 3.0 };
	Cmiss_field_id texture_coordinate_field = Cmiss_field_module_create_constant(zinc.fm,
		sizeof(values)/sizeof(double), values);
	EXPECT_NE(static_cast<Cmiss_field *>(0), texture_coordinate_field);

	EXPECT_EQ((Cmiss_field_id)0, Cmiss_graphic_get_texture_coordinate_field(gr));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_texture_coordinate_field(gr, texture_coordinate_field));
	Cmiss_field_id temp_texture_coordinate_field = Cmiss_graphic_get_texture_coordinate_field(gr);
	EXPECT_EQ(texture_coordinate_field, temp_texture_coordinate_field);
	Cmiss_field_destroy(&temp_texture_coordinate_field);

	// coordinate field cannot have more than 3 components
	const double values4[] = { 1.0, 2.0, 3.0, 4.0 };
	Cmiss_field_id bad_texture_coordinate_field = Cmiss_field_module_create_constant(zinc.fm,
		sizeof(values4)/sizeof(double), values4);
	EXPECT_NE(static_cast<Cmiss_field *>(0), bad_texture_coordinate_field);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_set_texture_coordinate_field(gr, bad_texture_coordinate_field));
	Cmiss_field_destroy(&bad_texture_coordinate_field);
	temp_texture_coordinate_field = Cmiss_graphic_get_texture_coordinate_field(gr);
	EXPECT_EQ(texture_coordinate_field, temp_texture_coordinate_field);
	Cmiss_field_destroy(&temp_texture_coordinate_field);
	Cmiss_field_destroy(&texture_coordinate_field);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_texture_coordinate_field(gr, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_get_texture_coordinate_field(gr));

	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, point_attributes_glyph)
{
	ZincTestSetup zinc;

	Cmiss_graphic_points_id gr = Cmiss_rendition_create_graphic_points(zinc.ren);
	EXPECT_NE(static_cast<Cmiss_graphic_points *>(0), gr);

	Cmiss_graphic_point_attributes_id pointattr =
		Cmiss_graphic_get_point_attributes(Cmiss_graphic_points_base_cast(gr));
	EXPECT_NE(static_cast<Cmiss_graphic_point_attributes *>(0), pointattr);

	Cmiss_glyph_id glyph = Cmiss_glyph_module_get_default_point_glyph(zinc.glyph_module);
	EXPECT_NE((Cmiss_glyph_id)0, glyph);
	Cmiss_glyph_id temp_glyph = Cmiss_graphic_point_attributes_get_glyph(pointattr);
	EXPECT_EQ(glyph, temp_glyph);
	Cmiss_glyph_destroy(&temp_glyph);
	Cmiss_glyph_destroy(&glyph);

	glyph = Cmiss_glyph_module_find_glyph_by_name(zinc.glyph_module, "sphere");
	EXPECT_NE((Cmiss_glyph_id)0, glyph);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_glyph(pointattr, glyph));
	temp_glyph = Cmiss_graphic_point_attributes_get_glyph(pointattr);
	EXPECT_EQ(glyph, temp_glyph);
	Cmiss_glyph_destroy(&temp_glyph);
	Cmiss_glyph_destroy(&glyph);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_glyph_type(pointattr, CMISS_GRAPHICS_GLYPH_TYPE_INVALID));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_glyph_type(pointattr, CMISS_GRAPHICS_GLYPH_SPHERE));

	EXPECT_EQ(CMISS_GLYPH_REPEAT_NONE, Cmiss_graphic_point_attributes_get_glyph_repeat_mode(pointattr));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_glyph_repeat_mode(0, CMISS_GLYPH_REPEAT_MIRROR));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_glyph_repeat_mode(pointattr, CMISS_GLYPH_REPEAT_MODE_INVALID));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_glyph_repeat_mode(pointattr, CMISS_GLYPH_REPEAT_MIRROR));
	EXPECT_EQ(CMISS_GLYPH_REPEAT_MIRROR, Cmiss_graphic_point_attributes_get_glyph_repeat_mode(pointattr));
	double fieldValues[] = { 0.3, 0.4, 0.5 };
	Cmiss_field_id field = Cmiss_field_module_create_constant(zinc.fm, 3, fieldValues);
	EXPECT_NE(static_cast<Cmiss_field *>(0), field);
	Cmiss_field_id temp_field = 0;

	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_point_attributes_get_orientation_scale_field(pointattr));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_orientation_scale_field(pointattr, field));
	temp_field = Cmiss_graphic_point_attributes_get_orientation_scale_field(pointattr);
	EXPECT_EQ(temp_field, field);
	Cmiss_field_destroy(&temp_field);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_orientation_scale_field(pointattr, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_point_attributes_get_orientation_scale_field(pointattr));

	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_point_attributes_get_signed_scale_field(pointattr));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_signed_scale_field(pointattr, field));
	temp_field = Cmiss_graphic_point_attributes_get_signed_scale_field(pointattr);
	EXPECT_EQ(temp_field, field);
	Cmiss_field_destroy(&temp_field);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_signed_scale_field(pointattr, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_point_attributes_get_signed_scale_field(pointattr));

	Cmiss_field_destroy(&field);

	const double values[] = { 0.5, 1.2 };
	double outputValues[3];

	// check default values = 0.0
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_get_base_size(pointattr, 3, outputValues));
	EXPECT_EQ(0.0, outputValues[0]);
	EXPECT_EQ(0.0, outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_base_size(pointattr, 0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_base_size(pointattr, 2, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_base_size(pointattr, 2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_get_base_size(pointattr, 0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_get_base_size(pointattr, 3, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_get_base_size(pointattr, 3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[1], outputValues[2]);

	// check default values = 0.0
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_get_offset(pointattr, 3, outputValues));
	EXPECT_EQ(0.0, outputValues[0]);
	EXPECT_EQ(0.0, outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_offset(pointattr, 0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_offset(pointattr, 2, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_offset(pointattr, 2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_get_offset(pointattr, 0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_get_offset(pointattr, 3, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_get_offset(pointattr, 3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);

	// check default values = 1.0
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_get_scale_factors(pointattr, 3, outputValues));
	EXPECT_EQ(1.0, outputValues[0]);
	EXPECT_EQ(1.0, outputValues[1]);
	EXPECT_EQ(1.0, outputValues[2]);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_scale_factors(pointattr, 0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_scale_factors(pointattr, 2, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_scale_factors(pointattr, 2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_get_scale_factors(pointattr, 0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_get_scale_factors(pointattr, 3, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_get_scale_factors(pointattr, 3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[1], outputValues[2]);

	Cmiss_graphic_point_attributes_destroy(&pointattr);
	Cmiss_graphic_points_destroy(&gr);
}

TEST(Cmiss_graphic_api, point_attributes_glyph_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicPoints gr = zinc.ren.createGraphicPoints();
	EXPECT_TRUE(gr.isValid());
	// test can assign to base class handle
	Graphic tmp(gr);
	EXPECT_TRUE(tmp.isValid());

	GraphicPointAttributes pointattr = gr.getPointAttributes();
	EXPECT_TRUE(pointattr.isValid());

	Glyph glyph = zinc.glyphModule.getDefaultPointGlyph();
	EXPECT_TRUE(glyph.isValid());
	Glyph tempGlyph = pointattr.getGlyph();
	EXPECT_EQ(glyph.getId(), tempGlyph.getId());

	glyph = zinc.glyphModule.findGlyphByName("sphere");
	EXPECT_TRUE(glyph.isValid());
	EXPECT_EQ(CMISS_OK, pointattr.setGlyph(glyph));
	tempGlyph = pointattr.getGlyph();
	EXPECT_EQ(glyph.getId(), tempGlyph.getId());

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setGlyphType(Graphic::GLYPH_TYPE_INVALID));
	EXPECT_EQ(CMISS_OK, pointattr.setGlyphType(Graphic::GLYPH_TYPE_SPHERE));

	EXPECT_EQ(Glyph::REPEAT_NONE, pointattr.getGlyphRepeatMode());
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setGlyphRepeatMode(Glyph::REPEAT_MODE_INVALID));
	EXPECT_EQ(CMISS_OK, pointattr.setGlyphRepeatMode(Glyph::REPEAT_MIRROR));
	EXPECT_EQ(Glyph::REPEAT_MIRROR, pointattr.getGlyphRepeatMode());

	double fieldValues[] = { 0.3, 0.4, 0.5 };
	Field field = zinc.fm.createConstant(sizeof(fieldValues)/sizeof(double), fieldValues);
	EXPECT_TRUE(field.isValid());
	Field tempField;

	EXPECT_FALSE(pointattr.getOrientationScaleField().isValid());
	EXPECT_EQ(CMISS_OK, pointattr.setOrientationScaleField(field));
	tempField = pointattr.getOrientationScaleField();
	EXPECT_EQ(tempField.getId(), field.getId());
	Field noField;
	EXPECT_EQ(CMISS_OK, pointattr.setOrientationScaleField(noField)); // clear field
	EXPECT_FALSE(pointattr.getOrientationScaleField().isValid());

	EXPECT_FALSE(pointattr.getSignedScaleField().isValid());
	EXPECT_EQ(CMISS_OK, pointattr.setSignedScaleField(field));
	tempField = pointattr.getSignedScaleField();
	EXPECT_EQ(tempField.getId(), field.getId());
	EXPECT_EQ(CMISS_OK, pointattr.setSignedScaleField(noField)); // clear field
	EXPECT_FALSE(pointattr.getSignedScaleField().isValid());

	const double values[] = { 0.5, 1.2 };
	double outputValues[3];

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setBaseSize(0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setBaseSize(2, 0));
	EXPECT_EQ(CMISS_OK, pointattr.setBaseSize(2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.getBaseSize(0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.getBaseSize(3, 0));
	EXPECT_EQ(CMISS_OK, pointattr.getBaseSize(3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[1], outputValues[2]);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setOffset(0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setOffset(2, 0));
	EXPECT_EQ(CMISS_OK, pointattr.setOffset(2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.getOffset(0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.getOffset(3, 0));
	EXPECT_EQ(CMISS_OK, pointattr.getOffset(3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setScaleFactors(0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setScaleFactors(2, 0));
	EXPECT_EQ(CMISS_OK, pointattr.setScaleFactors(2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.getScaleFactors(0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.getScaleFactors(3, 0));
	EXPECT_EQ(CMISS_OK, pointattr.getScaleFactors(3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[1], outputValues[2]);
}

TEST(Cmiss_graphic_api, point_attributes_label)
{
	ZincTestSetup zinc;

	Cmiss_graphic_points_id gr = Cmiss_rendition_create_graphic_points(zinc.ren);
	EXPECT_NE(static_cast<Cmiss_graphic_points *>(0), gr);

	Cmiss_graphic_point_attributes_id pointattr = Cmiss_graphic_get_point_attributes(Cmiss_graphic_points_base_cast(gr));
	EXPECT_NE(static_cast<Cmiss_graphic_point_attributes *>(0), pointattr);

	double values[] = { 1.0, 2.0, 3.0 };
	Cmiss_field_id label_field = Cmiss_field_module_create_constant(zinc.fm,
		sizeof(values)/sizeof(double), values);
	EXPECT_NE(static_cast<Cmiss_field *>(0), label_field);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_label_field(pointattr, label_field));

	Cmiss_field_id temp_label_field = Cmiss_graphic_point_attributes_get_label_field(pointattr);
	EXPECT_EQ(temp_label_field, label_field);
	Cmiss_field_destroy(&temp_label_field);
	Cmiss_field_destroy(&label_field);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_label_field(pointattr, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_point_attributes_get_label_field(pointattr));

	double outputValues[3];
	// check default values = 0.0
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_get_label_offset(pointattr, 3, outputValues));
	EXPECT_EQ(0.0, outputValues[0]);
	EXPECT_EQ(0.0, outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_label_offset(pointattr, 0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_label_offset(pointattr, 2, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_label_offset(pointattr, 2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_get_label_offset(pointattr, 0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_get_label_offset(pointattr, 3, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_get_label_offset(pointattr, 3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);

	// should start with a default font
	Cmiss_font_id font = Cmiss_graphic_point_attributes_get_font(pointattr);
	EXPECT_NE(static_cast<Cmiss_font *>(0), font);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_font(pointattr, 0));
	EXPECT_EQ(static_cast<Cmiss_font *>(0), Cmiss_graphic_point_attributes_get_font(pointattr));

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_font(pointattr, font));

	const char *text = "ABC";
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_label_text(0, 1, text));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_label_text(pointattr, 0, text));
	EXPECT_EQ((char *)0, Cmiss_graphic_point_attributes_get_label_text(0, 1));
	EXPECT_EQ((char *)0, Cmiss_graphic_point_attributes_get_label_text(pointattr, 0));
	for (int labelNumber = 1; labelNumber <= 3; ++labelNumber)
	{
		char *outText;
		EXPECT_EQ((char *)0, Cmiss_graphic_point_attributes_get_label_text(pointattr, labelNumber));
		EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_label_text(pointattr, labelNumber, text));
		outText = Cmiss_graphic_point_attributes_get_label_text(pointattr, labelNumber);
		EXPECT_STREQ(text, outText);
		Cmiss_deallocate(outText);
	}
	Cmiss_font_destroy(&font);
	Cmiss_graphic_point_attributes_destroy(&pointattr);
	Cmiss_graphic_points_destroy(&gr);
}

TEST(Cmiss_graphic_api, point_attributes_label_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicPoints gr = zinc.ren.createGraphicPoints();
	EXPECT_TRUE(gr.isValid());

	GraphicPointAttributes pointattr = gr.getPointAttributes();
	EXPECT_TRUE(pointattr.isValid());

	double values[] = { 1.0, 2.0, 3.0 };
	Field labelField = zinc.fm.createConstant(sizeof(values)/sizeof(double), values);
	EXPECT_TRUE(labelField.isValid());

	EXPECT_EQ(CMISS_OK, pointattr.setLabelField(labelField));

	Field tempLabelField = pointattr.getLabelField();
	EXPECT_EQ(tempLabelField.getId(), labelField.getId());

	Field noField;
	EXPECT_EQ(CMISS_OK, pointattr.setLabelField(noField)); // clear label field
	EXPECT_FALSE(pointattr.getLabelField().isValid());

	double outputValues[3];
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setLabelOffset(0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setLabelOffset(2, 0));
	EXPECT_EQ(CMISS_OK, pointattr.setLabelOffset(2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.getLabelOffset(0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.getLabelOffset(3, 0));
	EXPECT_EQ(CMISS_OK, pointattr.getLabelOffset(3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);

	// should start with a default font
	Font font = pointattr.getFont();
	EXPECT_TRUE(font.isValid());

	Font noFont;
	EXPECT_EQ(CMISS_OK, pointattr.setFont(noFont)); // clear font
	EXPECT_FALSE(pointattr.getFont().isValid());

	EXPECT_EQ(CMISS_OK, pointattr.setFont(font));

	const char *text = "ABC";
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setLabelText(0, text));
	EXPECT_EQ((char *)0, pointattr.getLabelText(0));
	for (int labelNumber = 1; labelNumber <= 3; ++labelNumber)
	{
		char *outText;
		EXPECT_EQ((char *)0, pointattr.getLabelText(labelNumber));
		EXPECT_EQ(CMISS_OK, pointattr.setLabelText(labelNumber, text));
		outText = pointattr.getLabelText(labelNumber);
		EXPECT_STREQ(text, outText);
		Cmiss_deallocate(outText);
	}
}

TEST(Cmiss_graphic_api, line_attributes)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_CYLINDERS);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	Cmiss_graphic_line_attributes_id lineattr = Cmiss_graphic_get_line_attributes(gr);
	EXPECT_NE(static_cast<Cmiss_graphic_line_attributes *>(0), lineattr);

	double value = 1.0;
	Cmiss_field_id orientation_scale_field = Cmiss_field_module_create_constant(zinc.fm, 1, &value);
	EXPECT_NE(static_cast<Cmiss_field *>(0), orientation_scale_field);

	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_line_attributes_get_orientation_scale_field(lineattr));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_line_attributes_set_orientation_scale_field(lineattr, orientation_scale_field));

	Cmiss_field_id temp_orientation_scale_field = Cmiss_graphic_line_attributes_get_orientation_scale_field(lineattr);
	EXPECT_EQ(temp_orientation_scale_field, orientation_scale_field);
	Cmiss_field_destroy(&temp_orientation_scale_field);
	Cmiss_field_destroy(&orientation_scale_field);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_line_attributes_set_orientation_scale_field(lineattr, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_line_attributes_get_orientation_scale_field(lineattr));

	const double values[] = { 0.5, 1.2 };
	double outputValues[3];

	// check default values = 0.0
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_line_attributes_get_base_size(lineattr, 3, outputValues));
	EXPECT_EQ(0.0, outputValues[0]);
	EXPECT_EQ(0.0, outputValues[1]);
	EXPECT_EQ(0.0, outputValues[2]);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_line_attributes_set_base_size(lineattr, 0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_line_attributes_set_base_size(lineattr, 2, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_line_attributes_set_base_size(lineattr, 2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_line_attributes_get_base_size(lineattr, 0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_line_attributes_get_base_size(lineattr, 3, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_line_attributes_get_base_size(lineattr, 3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[1], outputValues[2]);

	// check default values = 1.0
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_line_attributes_get_scale_factors(lineattr, 3, outputValues));
	EXPECT_EQ(1.0, outputValues[0]);
	EXPECT_EQ(1.0, outputValues[1]);
	EXPECT_EQ(1.0, outputValues[2]);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_line_attributes_set_scale_factors(lineattr, 0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_line_attributes_set_scale_factors(lineattr, 2, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_line_attributes_set_scale_factors(lineattr, 2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_line_attributes_get_scale_factors(lineattr, 0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_line_attributes_get_scale_factors(lineattr, 3, 0));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_line_attributes_get_scale_factors(lineattr, 3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[1], outputValues[2]);

	Cmiss_graphic_line_attributes_destroy(&lineattr);
	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, line_attributes_cpp)
{
	ZincTestSetupCpp zinc;

	Graphic gr = zinc.ren.createGraphic(Graphic::GRAPHIC_CYLINDERS);
	EXPECT_TRUE(gr.isValid());

	GraphicLineAttributes lineattr = gr.getLineAttributes();
	EXPECT_TRUE(lineattr.isValid());

	double value = 1.0;
	Field orientationScaleField = zinc.fm.createConstant(1, &value);
	EXPECT_TRUE(orientationScaleField.isValid());

	EXPECT_FALSE(lineattr.getOrientationScaleField().isValid());
	EXPECT_EQ(CMISS_OK, lineattr.setOrientationScaleField(orientationScaleField));

	Field tempOrientationScaleField = lineattr.getOrientationScaleField();
	EXPECT_EQ(tempOrientationScaleField.getId(), orientationScaleField.getId());

	Field noField;
	EXPECT_EQ(CMISS_OK, lineattr.setOrientationScaleField(noField)); // clear field
	EXPECT_FALSE(lineattr.getOrientationScaleField().isValid());

	const double values[] = { 0.5, 1.2 };
	double outputValues[3];

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, lineattr.setBaseSize(0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, lineattr.setBaseSize(2, 0));
	EXPECT_EQ(CMISS_OK, lineattr.setBaseSize(2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, lineattr.getBaseSize(0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, lineattr.getBaseSize(3, 0));
	EXPECT_EQ(CMISS_OK, lineattr.getBaseSize(3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[1], outputValues[2]);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, lineattr.setScaleFactors(0, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, lineattr.setScaleFactors(2, 0));
	EXPECT_EQ(CMISS_OK, lineattr.setScaleFactors(2, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, lineattr.getScaleFactors(0, outputValues));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, lineattr.getScaleFactors(3, 0));
	EXPECT_EQ(CMISS_OK, lineattr.getScaleFactors(3, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[1], outputValues[2]);
}

