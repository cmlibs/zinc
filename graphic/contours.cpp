
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/region.h>
#include <zinc/fieldmodule.h>
#include <zinc/field.h>
#include <zinc/fieldconstant.h>
#include <zinc/graphic.h>

#include "zinctestsetup.hpp"

TEST(cmzn_graphic_contours, create)
{
	ZincTestSetup zinc;

	cmzn_graphic_contours_id is = cmzn_scene_create_graphic_contours(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphic_contours *>(0), is);

	EXPECT_EQ(CMZN_OK, cmzn_graphic_contours_destroy(&is));
}

TEST(cmzn_graphic_contours, cast_contours)
{
	ZincTestSetup zinc;

	cmzn_graphic_id gr = cmzn_graphic_contours_base_cast(
		cmzn_scene_create_graphic_contours(zinc.scene));

	cmzn_graphic_contours_id is = cmzn_graphic_cast_contours(gr);
	EXPECT_NE(static_cast<cmzn_graphic_contours *>(0), is);

	EXPECT_EQ(CMZN_OK, cmzn_graphic_contours_destroy(&is));
	EXPECT_EQ(CMZN_OK, cmzn_graphic_destroy(&gr));
}

TEST(cmzn_graphic_contours, base_cast)
{
	ZincTestSetup zinc;

	cmzn_graphic_contours_id is = cmzn_scene_create_graphic_contours(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphic_contours *>(0), is);

	// No need to destroy the return handle
	EXPECT_NE(static_cast<cmzn_graphic *>(0), cmzn_graphic_contours_base_cast(is));

	EXPECT_EQ(CMZN_OK, cmzn_graphic_contours_destroy(&is));
}

TEST(cmzn_graphic_contours, isoscalar_field)
{
	ZincTestSetup zinc;

	cmzn_graphic_contours_id is = cmzn_scene_create_graphic_contours(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphic_contours *>(0), is);

	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphic_contours_get_isoscalar_field(is));

	double values[] = {1.0};
	cmzn_field_id c = cmzn_fieldmodule_create_field_constant(zinc.fm, 1, values);
	EXPECT_EQ(CMZN_OK, cmzn_graphic_contours_set_isoscalar_field(is, c));

	cmzn_field_id temp_c = cmzn_graphic_contours_get_isoscalar_field(is);
	EXPECT_EQ(temp_c, c);
	cmzn_field_destroy(&temp_c);
	cmzn_field_destroy(&c);

	EXPECT_EQ(CMZN_OK, cmzn_graphic_contours_set_isoscalar_field(is, 0));
	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphic_contours_get_isoscalar_field(is));

	cmzn_field_destroy(&c);
	EXPECT_EQ(CMZN_OK, cmzn_graphic_contours_destroy(&is));
}

TEST(cmzn_graphic_contours, list_isovalues)
{
	ZincTestSetup zinc;

	cmzn_graphic_contours_id is = cmzn_scene_create_graphic_contours(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphic_contours *>(0), is);

	double outputValues[4];
	EXPECT_EQ(0, cmzn_graphic_contours_get_list_isovalues(is, 4, outputValues));

	const int num = 3;
	const double values[] = {1.0, 1.2, 3.4};
	EXPECT_EQ(CMZN_OK, cmzn_graphic_contours_set_list_isovalues(is, num, values));
	EXPECT_EQ(0, cmzn_graphic_contours_get_range_number_of_isovalues(is)); // = not set as range
	EXPECT_EQ(num, cmzn_graphic_contours_get_list_isovalues(is, 4, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[2], outputValues[2]);
	// can ask for just number:
	EXPECT_EQ(num, cmzn_graphic_contours_get_list_isovalues(is, 0, 0));

	EXPECT_EQ(CMZN_OK, cmzn_graphic_contours_destroy(&is));
}

TEST(cmzn_graphic_contours, list_isovalues_null)
{
	ZincTestSetup zinc;

	cmzn_graphic_contours_id is = cmzn_scene_create_graphic_contours(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphic_contours *>(0), is);

	int num = 3;
	double values[] = {1.0, 1.2, 3.4};
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphic_contours_set_list_isovalues(0, num, values));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphic_contours_set_list_isovalues(is, 5, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphic_contours_set_list_isovalues(is, -1, 0));

	double outputValues[4];
	EXPECT_EQ(0, cmzn_graphic_contours_get_list_isovalues(0, 4, outputValues));
	EXPECT_EQ(0, cmzn_graphic_contours_get_list_isovalues(is, 4, 0));
	EXPECT_EQ(0, cmzn_graphic_contours_get_list_isovalues(is, -1, 0));

	EXPECT_EQ(CMZN_OK, cmzn_graphic_contours_destroy(&is));
}

TEST(cmzn_graphic_contours, range_isovalues)
{
	ZincTestSetup zinc;

	cmzn_graphic_contours_id is = cmzn_scene_create_graphic_contours(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphic_contours *>(0), is);

	EXPECT_EQ(0, cmzn_graphic_contours_get_range_number_of_isovalues(is));
	EXPECT_EQ(0.0, cmzn_graphic_contours_get_range_first_isovalue(is));
	EXPECT_EQ(0.0, cmzn_graphic_contours_get_range_last_isovalue(is));

	const int num = 6;
	double first = 0.1, last = 0.55;
	EXPECT_EQ(CMZN_OK, cmzn_graphic_contours_set_range_isovalues(is, 1, 0.3, 0.3));
	EXPECT_EQ(CMZN_OK, cmzn_graphic_contours_set_range_isovalues(is, 1, 0.7, 0.7));
	EXPECT_EQ(CMZN_OK, cmzn_graphic_contours_set_range_isovalues(is, num, first, last));
	EXPECT_EQ(0, cmzn_graphic_contours_get_list_isovalues(is, 0, 0)); // = not set as list
	EXPECT_EQ(num, cmzn_graphic_contours_get_range_number_of_isovalues(is));
	EXPECT_EQ(first, cmzn_graphic_contours_get_range_first_isovalue(is));
	EXPECT_EQ(last, cmzn_graphic_contours_get_range_last_isovalue(is));

	EXPECT_EQ(CMZN_OK, cmzn_graphic_contours_destroy(&is));
}

TEST(cmzn_graphic_contours, range_isovalues_null)
{
	ZincTestSetup zinc;

	cmzn_graphic_contours_id is = cmzn_scene_create_graphic_contours(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphic_contours *>(0), is);

	const int num = 6;
	double first = 0.1, last = 0.55;
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphic_contours_set_range_isovalues(0, num, first, last));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphic_contours_set_range_isovalues(is, -1, first, last));
	EXPECT_EQ(CMZN_OK, cmzn_graphic_contours_set_range_isovalues(is, 0, first, last));
	EXPECT_EQ(CMZN_OK, cmzn_graphic_contours_set_range_isovalues(is, num, first, last));

	EXPECT_EQ(0, cmzn_graphic_contours_get_range_number_of_isovalues(0));
	EXPECT_EQ(0.0, cmzn_graphic_contours_get_range_first_isovalue(0));
	EXPECT_EQ(0.0, cmzn_graphic_contours_get_range_last_isovalue(0));

	EXPECT_EQ(CMZN_OK, cmzn_graphic_contours_destroy(&is));
}

