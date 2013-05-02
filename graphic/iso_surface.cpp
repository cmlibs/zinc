
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

#include "zinctestsetup.hpp"

TEST(Cmiss_rendition_create_graphic, iso_surface)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id is = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_ISO_SURFACES);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), is);

	Cmiss_graphic_destroy(&is);
}

TEST(Cmiss_rendition_create_graphic, cast_iso_surface)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_ISO_SURFACES);

	Cmiss_graphic_iso_surface_id is = Cmiss_graphic_cast_iso_surface(gr);
	EXPECT_NE(static_cast<Cmiss_graphic_iso_surface *>(0), is);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_destroy(&is));
	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_iso_surface, base_cast)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_ISO_SURFACES);

	Cmiss_graphic_iso_surface_id is = Cmiss_graphic_cast_iso_surface(gr);
	EXPECT_NE(static_cast<Cmiss_graphic_iso_surface *>(0), is);

	// No need to destroy the return handle
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), Cmiss_graphic_iso_surface_base_cast(is));

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_destroy(&is));
	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_iso_surface, iso_scalar_field)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_ISO_SURFACES);

	Cmiss_graphic_iso_surface_id is = Cmiss_graphic_cast_iso_surface(gr);
	EXPECT_NE(static_cast<Cmiss_graphic_iso_surface *>(0), is);

	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_iso_surface_get_iso_scalar_field(is));

	double values[] = {1.0};
	Cmiss_field_id c = Cmiss_field_module_create_constant(zinc.fm, 1, values);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_set_iso_scalar_field(is, c));

	Cmiss_field_id temp_c = Cmiss_graphic_iso_surface_get_iso_scalar_field(is);
	EXPECT_EQ(temp_c, c);
	Cmiss_field_destroy(&temp_c);
	Cmiss_field_destroy(&c);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_set_iso_scalar_field(is, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_iso_surface_get_iso_scalar_field(is));

	Cmiss_field_destroy(&c);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_destroy(&is));
	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_iso_surface, iso_values)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_ISO_SURFACES);

	Cmiss_graphic_iso_surface_id is = Cmiss_graphic_cast_iso_surface(gr);
	EXPECT_NE(static_cast<Cmiss_graphic_iso_surface *>(0), is);

	double outputValues[4];
	EXPECT_EQ(0, Cmiss_graphic_iso_surface_get_iso_values(is, 4, outputValues));

	const int num = 3;
	const double values[] = {1.0, 1.2, 3.4};
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_set_iso_values(is, num, values));
	EXPECT_EQ(num, Cmiss_graphic_iso_surface_get_iso_values(is, 4, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[2], outputValues[2]);
	// can ask for just number:
	EXPECT_EQ(num, Cmiss_graphic_iso_surface_get_iso_values(is, 0, 0));

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_destroy(&is));
	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_iso_surface, iso_values_null)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_ISO_SURFACES);

	Cmiss_graphic_iso_surface_id is = Cmiss_graphic_cast_iso_surface(gr);
	EXPECT_NE(static_cast<Cmiss_graphic_iso_surface *>(0), is);

	int num = 3;
	double values[] = {1.0, 1.2, 3.4};
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_iso_surface_set_iso_values(0, num, values));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_iso_surface_set_iso_values(is, 5, 0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_iso_surface_set_iso_values(is, -1, 0));

	double outputValues[4];
	EXPECT_EQ(0, Cmiss_graphic_iso_surface_get_iso_values(0, 4, outputValues));
	EXPECT_EQ(0, Cmiss_graphic_iso_surface_get_iso_values(is, 4, 0));
	EXPECT_EQ(0, Cmiss_graphic_iso_surface_get_iso_values(is, -1, 0));

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_destroy(&is));
	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_iso_surface, iso_range)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_ISO_SURFACES);

	Cmiss_graphic_iso_surface_id is = Cmiss_graphic_cast_iso_surface(gr);
	EXPECT_NE(static_cast<Cmiss_graphic_iso_surface *>(0), is);

	double outputFirst, outputLast;
	EXPECT_EQ(0, Cmiss_graphic_iso_surface_get_iso_range(is, &outputFirst, &outputLast));

	const int num = 6;
	double first = 0.1, last = 0.55;
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_set_iso_range(is, 1, 0.3, 0.3));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_set_iso_range(is, 1, 0.7, 0.7));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_set_iso_range(is, num, first, last));
	EXPECT_EQ(num, Cmiss_graphic_iso_surface_get_iso_range(is, &outputFirst, &outputLast));
	EXPECT_EQ(first, outputFirst);
	EXPECT_EQ(last, outputLast);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_destroy(&is));
	Cmiss_graphic_destroy(&gr);
}

TEST(Cmiss_graphic_iso_surface, iso_range_null)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_ISO_SURFACES);

	Cmiss_graphic_iso_surface_id is = Cmiss_graphic_cast_iso_surface(gr);
	EXPECT_NE(static_cast<Cmiss_graphic_iso_surface *>(0), is);

	const int num = 6;
	double first = 0.1, last = 0.55;//
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_iso_surface_set_iso_range(0, num, first, last));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_graphic_iso_surface_set_iso_range(is, -1, first, last));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_set_iso_range(is, 0, first, last));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_set_iso_range(is, num, first, last));

	double outputFirst, outputLast;
	EXPECT_EQ(0, Cmiss_graphic_iso_surface_get_iso_range(0, &outputFirst, &outputLast));
	EXPECT_EQ(0, Cmiss_graphic_iso_surface_get_iso_range(is, 0, &outputLast));
	EXPECT_EQ(0, Cmiss_graphic_iso_surface_get_iso_range(is, &outputFirst, 0));
	EXPECT_EQ(num, Cmiss_graphic_iso_surface_get_iso_range(is, &outputFirst, &outputLast));

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_iso_surface_destroy(&is));
	Cmiss_graphic_destroy(&gr);
}

