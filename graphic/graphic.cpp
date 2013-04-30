
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
#include "zinc/graphicsfont.hpp"

TEST(Cmiss_graphic_api, set_use_element_type)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id is = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_ISO_SURFACES);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), is);

	int result = Cmiss_graphic_set_use_element_type(is, CMISS_GRAPHIC_USE_ELEMENT_FACES);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_graphic_destroy(&is);
}

TEST(Cmiss_graphic_api, exterior)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id is = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_ISO_SURFACES);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), is);

	int result = Cmiss_graphic_set_exterior(is, 1);
	EXPECT_EQ(CMISS_OK, result);
	EXPECT_EQ(1, Cmiss_graphic_get_exterior(is));

	Cmiss_graphic_destroy(&is);
}

TEST(Cmiss_graphic_api, face)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id is = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_ISO_SURFACES);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), is);

	int result = Cmiss_graphic_set_face(is, CMISS_GRAPHIC_FACE_XI2_0);
	EXPECT_EQ(CMISS_OK, result);
	EXPECT_EQ(CMISS_GRAPHIC_FACE_XI2_0, Cmiss_graphic_get_face(is));

	Cmiss_graphic_destroy(&is);
}

TEST(Cmiss_graphic_api, coordinate_field)
{
	ZincTestSetup zinc;

	double values[] = { 1.0, 2.0, 3.0 };
	Cmiss_field_id coordinate_field = Cmiss_field_module_create_constant(zinc.fm,
		sizeof(values)/sizeof(double), values);
	EXPECT_NE(static_cast<Cmiss_field *>(0), coordinate_field);

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_NODE_POINTS);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_coordinate_field(gr, coordinate_field));

	// coordinate field cannot have more than 3 components
	double values4[] = { 1.0, 2.0, 3.0, 4.0 };
	Cmiss_field_id bad_coordinate_field = Cmiss_field_module_create_constant(zinc.fm,
		sizeof(values4)/sizeof(double), values4);
	EXPECT_NE(static_cast<Cmiss_field *>(0), bad_coordinate_field);
	// previous coordinate field should be left unchanged
	EXPECT_NE(CMISS_OK, Cmiss_graphic_set_coordinate_field(gr, bad_coordinate_field));
	Cmiss_field_destroy(&bad_coordinate_field);

	Cmiss_field_id temp_coordinate_field = Cmiss_graphic_get_coordinate_field(gr);
	EXPECT_EQ(temp_coordinate_field, coordinate_field);
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

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_NODE_POINTS);
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

TEST(Cmiss_graphic_api, point_attributes_glyph)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_POINT);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	Cmiss_graphic_point_attributes_id pointattr = Cmiss_graphic_get_point_attributes(gr);
	EXPECT_NE(static_cast<Cmiss_graphic_point_attributes *>(0), pointattr);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_point_attributes_set_glyph_type(pointattr, CMISS_GRAPHICS_GLYPH_TYPE_INVALID));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_glyph_type(pointattr, CMISS_GRAPHICS_GLYPH_SPHERE));

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
	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, point_attributes_glyph_cpp)
{
	ZincTestSetupCpp zinc;

	Graphic gr = zinc.ren.createGraphic(Graphic::GRAPHIC_POINT);
	EXPECT_EQ(true, gr.isValid());

	GraphicPointAttributes pointattr = gr.getPointAttributes();
	EXPECT_EQ(true, pointattr.isValid());

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, pointattr.setGlyphType(Graphic::GLYPH_TYPE_INVALID));
	EXPECT_EQ(CMISS_OK, pointattr.setGlyphType(Graphic::GLYPH_TYPE_SPHERE));

	double fieldValues[] = { 0.3, 0.4, 0.5 };
	Field field = zinc.fm.createConstant(sizeof(fieldValues)/sizeof(double), fieldValues);
	EXPECT_EQ(true, field.isValid());
	Field tempField;

	EXPECT_EQ(false, pointattr.getOrientationScaleField().isValid());
	EXPECT_EQ(CMISS_OK, pointattr.setOrientationScaleField(field));
	tempField = pointattr.getOrientationScaleField();
	EXPECT_EQ(tempField.getId(), field.getId());
	EXPECT_EQ(CMISS_OK, pointattr.setOrientationScaleField(Field())); // clear field
	EXPECT_EQ(false, pointattr.getOrientationScaleField().isValid());

	EXPECT_EQ(false, pointattr.getSignedScaleField().isValid());
	EXPECT_EQ(CMISS_OK, pointattr.setSignedScaleField(field));
	tempField = pointattr.getSignedScaleField();
	EXPECT_EQ(tempField.getId(), field.getId());
	EXPECT_EQ(CMISS_OK, pointattr.setSignedScaleField(Field())); // clear field
	EXPECT_EQ(false, pointattr.getSignedScaleField().isValid());

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

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_POINT);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	Cmiss_graphic_point_attributes_id pointattr = Cmiss_graphic_get_point_attributes(gr);
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

	// should start with a default font
	Cmiss_graphics_font_id font = Cmiss_graphic_point_attributes_get_font(pointattr);
	EXPECT_NE(static_cast<Cmiss_graphics_font *>(0), font);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_font(pointattr, 0));
	EXPECT_EQ(static_cast<Cmiss_graphics_font *>(0), Cmiss_graphic_point_attributes_get_font(pointattr));

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_point_attributes_set_font(pointattr, font));

	Cmiss_graphic_point_attributes_destroy(&pointattr);
	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_api, point_attributes_label_cpp)
{
	ZincTestSetupCpp zinc;

	Graphic gr = zinc.ren.createGraphic(Graphic::GRAPHIC_POINT);
	EXPECT_EQ(true, gr.isValid());

	GraphicPointAttributes pointattr = gr.getPointAttributes();
	EXPECT_EQ(true, pointattr.isValid());

	double values[] = { 1.0, 2.0, 3.0 };
	Field labelField = zinc.fm.createConstant(sizeof(values)/sizeof(double), values);
	EXPECT_EQ(true, labelField.isValid());

	EXPECT_EQ(CMISS_OK, pointattr.setLabelField(labelField));

	Field tempLabelField = pointattr.getLabelField();
	EXPECT_EQ(tempLabelField.getId(), labelField.getId());

	EXPECT_EQ(CMISS_OK, pointattr.setLabelField(Field())); // clear label field
	EXPECT_EQ(false, pointattr.getLabelField().isValid());

	// should start with a default font
	GraphicsFont font = pointattr.getFont();
	EXPECT_EQ(true, font.isValid());

	EXPECT_EQ(CMISS_OK, pointattr.setFont(GraphicsFont())); // clear font
	EXPECT_EQ(false, pointattr.getFont().isValid());

	EXPECT_EQ(CMISS_OK, pointattr.setFont(font));
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
	EXPECT_EQ(true, gr.isValid());

	GraphicLineAttributes lineattr = gr.getLineAttributes();
	EXPECT_EQ(true, lineattr.isValid());

	double value = 1.0;
	Field orientationScaleField = zinc.fm.createConstant(1, &value);
	EXPECT_EQ(true, orientationScaleField.isValid());

	EXPECT_EQ(false, lineattr.getOrientationScaleField().isValid());
	EXPECT_EQ(CMISS_OK, lineattr.setOrientationScaleField(orientationScaleField));

	Field tempOrientationScaleField = lineattr.getOrientationScaleField();
	EXPECT_EQ(tempOrientationScaleField.getId(), orientationScaleField.getId());

	EXPECT_EQ(CMISS_OK, lineattr.setOrientationScaleField(Field())); // clear field
	EXPECT_EQ(false, lineattr.getOrientationScaleField().isValid());

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
