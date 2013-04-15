
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
