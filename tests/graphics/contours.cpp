/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/status.h>
#include <opencmiss/zinc/core.h>
#include <opencmiss/zinc/context.h>
#include <opencmiss/zinc/region.h>
#include <opencmiss/zinc/fieldmodule.h>
#include <opencmiss/zinc/field.h>
#include <opencmiss/zinc/fieldconstant.h>
#include <opencmiss/zinc/graphics.h>

#include <opencmiss/zinc/changemanager.hpp>
#include <opencmiss/zinc/fieldarithmeticoperators.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldcomposite.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldtime.hpp>
#include <opencmiss/zinc/fieldtrigonometry.hpp>
#include <opencmiss/zinc/fieldvectoroperators.hpp>
#include <opencmiss/zinc/result.hpp>
#include <opencmiss/zinc/timekeeper.hpp>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "test_resources.h"

TEST(cmzn_graphics_contours, create_cast)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_contours(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);
	EXPECT_EQ(CMZN_GRAPHICS_TYPE_CONTOURS, cmzn_graphics_get_type(gr));

	cmzn_graphics_contours_id is = cmzn_graphics_cast_contours(gr);
	EXPECT_NE(static_cast<cmzn_graphics_contours *>(0), is);

	// Must not to destroy the returned base handle
	EXPECT_EQ(gr, cmzn_graphics_contours_base_cast(is));

	EXPECT_EQ(CMZN_OK, cmzn_graphics_contours_destroy(&is));
	EXPECT_EQ(CMZN_OK, cmzn_graphics_destroy(&gr));
}

TEST(ZincGraphicsContours, create)
{
	ZincTestSetupCpp zinc;

	GraphicsContours co = zinc.scene.createGraphicsContours();
	EXPECT_TRUE(co.isValid());
	EXPECT_EQ(Graphics::TYPE_CONTOURS, co.getType());
}

TEST(cmzn_graphics_contours, isoscalar_field)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_contours(zinc.scene);
	cmzn_graphics_contours_id is = cmzn_graphics_cast_contours(gr);
	cmzn_graphics_destroy(&gr);
	EXPECT_NE(static_cast<cmzn_graphics_contours *>(0), is);

	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphics_contours_get_isoscalar_field(is));

	double values[] = {1.0};
	cmzn_field_id c = cmzn_fieldmodule_create_field_constant(zinc.fm, 1, values);
	EXPECT_EQ(CMZN_OK, cmzn_graphics_contours_set_isoscalar_field(is, c));

	cmzn_field_id temp_c = cmzn_graphics_contours_get_isoscalar_field(is);
	EXPECT_EQ(temp_c, c);
	cmzn_field_destroy(&temp_c);
	cmzn_field_destroy(&c);

	EXPECT_EQ(CMZN_OK, cmzn_graphics_contours_set_isoscalar_field(is, 0));
	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphics_contours_get_isoscalar_field(is));

	cmzn_field_destroy(&c);
	EXPECT_EQ(CMZN_OK, cmzn_graphics_contours_destroy(&is));
}

TEST(cmzn_graphics_contours, list_isovalues)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_contours(zinc.scene);
	cmzn_graphics_contours_id is = cmzn_graphics_cast_contours(gr);
	cmzn_graphics_destroy(&gr);
	EXPECT_NE(static_cast<cmzn_graphics_contours *>(0), is);

	double outputValues[4];
	EXPECT_EQ(0, cmzn_graphics_contours_get_list_isovalues(is, 4, outputValues));

	const int num = 3;
	const double values[] = {1.0, 1.2, 3.4};
	EXPECT_EQ(CMZN_OK, cmzn_graphics_contours_set_list_isovalues(is, num, values));
	EXPECT_EQ(0, cmzn_graphics_contours_get_range_number_of_isovalues(is)); // = not set as range
	EXPECT_EQ(num, cmzn_graphics_contours_get_list_isovalues(is, 4, outputValues));
	EXPECT_EQ(values[0], outputValues[0]);
	EXPECT_EQ(values[1], outputValues[1]);
	EXPECT_EQ(values[2], outputValues[2]);
	// can ask for just number:
	EXPECT_EQ(num, cmzn_graphics_contours_get_list_isovalues(is, 0, 0));

	EXPECT_EQ(CMZN_OK, cmzn_graphics_contours_destroy(&is));
}

TEST(cmzn_graphics_contours, list_isovalues_null)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_contours(zinc.scene);
	cmzn_graphics_contours_id is = cmzn_graphics_cast_contours(gr);
	cmzn_graphics_destroy(&gr);
	EXPECT_NE(static_cast<cmzn_graphics_contours *>(0), is);

	int num = 3;
	double values[] = {1.0, 1.2, 3.4};
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphics_contours_set_list_isovalues(0, num, values));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphics_contours_set_list_isovalues(is, 5, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphics_contours_set_list_isovalues(is, -1, 0));

	double outputValues[4];
	EXPECT_EQ(0, cmzn_graphics_contours_get_list_isovalues(0, 4, outputValues));
	EXPECT_EQ(0, cmzn_graphics_contours_get_list_isovalues(is, 4, 0));
	EXPECT_EQ(0, cmzn_graphics_contours_get_list_isovalues(is, -1, 0));

	EXPECT_EQ(CMZN_OK, cmzn_graphics_contours_destroy(&is));
}

TEST(cmzn_graphics_contours, range_isovalues)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_contours(zinc.scene);
	cmzn_graphics_contours_id is = cmzn_graphics_cast_contours(gr);
	cmzn_graphics_destroy(&gr);
	EXPECT_NE(static_cast<cmzn_graphics_contours *>(0), is);

	EXPECT_EQ(0, cmzn_graphics_contours_get_range_number_of_isovalues(is));
	EXPECT_EQ(0.0, cmzn_graphics_contours_get_range_first_isovalue(is));
	EXPECT_EQ(0.0, cmzn_graphics_contours_get_range_last_isovalue(is));

	const int num = 6;
	double first = 0.1, last = 0.55;
	EXPECT_EQ(CMZN_OK, cmzn_graphics_contours_set_range_isovalues(is, 1, 0.3, 0.3));
	EXPECT_EQ(CMZN_OK, cmzn_graphics_contours_set_range_isovalues(is, 1, 0.7, 0.7));
	EXPECT_EQ(CMZN_OK, cmzn_graphics_contours_set_range_isovalues(is, num, first, last));
	EXPECT_EQ(0, cmzn_graphics_contours_get_list_isovalues(is, 0, 0)); // = not set as list
	EXPECT_EQ(num, cmzn_graphics_contours_get_range_number_of_isovalues(is));
	EXPECT_EQ(first, cmzn_graphics_contours_get_range_first_isovalue(is));
	EXPECT_EQ(last, cmzn_graphics_contours_get_range_last_isovalue(is));

	EXPECT_EQ(CMZN_OK, cmzn_graphics_contours_destroy(&is));
}

TEST(cmzn_graphics_contours, range_isovalues_null)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_contours(zinc.scene);
	cmzn_graphics_contours_id is = cmzn_graphics_cast_contours(gr);
	cmzn_graphics_destroy(&gr);
	EXPECT_NE(static_cast<cmzn_graphics_contours *>(0), is);

	const int num = 6;
	double first = 0.1, last = 0.55;
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphics_contours_set_range_isovalues(0, num, first, last));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_graphics_contours_set_range_isovalues(is, -1, first, last));
	EXPECT_EQ(CMZN_OK, cmzn_graphics_contours_set_range_isovalues(is, 0, first, last));
	EXPECT_EQ(CMZN_OK, cmzn_graphics_contours_set_range_isovalues(is, num, first, last));

	EXPECT_EQ(0, cmzn_graphics_contours_get_range_number_of_isovalues(0));
	EXPECT_EQ(0.0, cmzn_graphics_contours_get_range_first_isovalue(0));
	EXPECT_EQ(0.0, cmzn_graphics_contours_get_range_last_isovalue(0));

	EXPECT_EQ(CMZN_OK, cmzn_graphics_contours_destroy(&is));
}

TEST(cmzn_graphics_contours, description_io)
{
	ZincTestSetupCpp zinc;

	double value = 1.0;
	Field orientationScaleField = zinc.fm.createFieldConstant(1, &value);
	orientationScaleField.setName("my_orientation_field");
	EXPECT_TRUE(orientationScaleField.isValid());

	GraphicsContours gr = zinc.scene.createGraphicsContours();
	EXPECT_TRUE(gr.isValid());

	EXPECT_EQ(CMZN_OK, gr.setRangeIsovalues(1, 0.3, 0.3));
	EXPECT_EQ(CMZN_OK, gr.setIsoscalarField(orientationScaleField));

	GraphicsContours gr2 = zinc.scene.createGraphicsContours();
	EXPECT_TRUE(gr2.isValid());

	double dvalues[3] = {0.1, 0.2, 0.3};

	EXPECT_EQ(CMZN_OK, gr2.setListIsovalues(3, dvalues));
	EXPECT_EQ(CMZN_OK, gr2.setIsoscalarField(orientationScaleField));

	char *return_string = zinc.scene.writeDescription();
	EXPECT_TRUE(return_string != 0);

	zinc.scene.removeGraphics(gr2);
	zinc.scene.removeGraphics(gr);
	EXPECT_EQ(0, zinc.scene.getNumberOfGraphics());

	EXPECT_EQ(RESULT_OK, zinc.scene.readDescription(return_string, false));

	gr = zinc.scene.getFirstGraphics().castContours();
	gr2 = zinc.scene.getNextGraphics(gr).castContours();

	EXPECT_EQ(orientationScaleField.getId(), gr.getIsoscalarField().getId());
	EXPECT_EQ(1,  gr.getRangeNumberOfIsovalues());
	EXPECT_EQ(0.3, gr.getRangeFirstIsovalue());
	EXPECT_EQ(0.3, gr.getRangeLastIsovalue());

	EXPECT_EQ(orientationScaleField.getId(), gr2.getIsoscalarField().getId());
	EXPECT_EQ(3, gr2.getListIsovalues(3, dvalues));
	EXPECT_EQ(0.1, dvalues[0]);
	EXPECT_EQ(0.2, dvalues[1]);
	EXPECT_EQ(0.3, dvalues[2]);

	cmzn_deallocate(return_string);
}

// test accuracy of cmgui example a/analytic_isosurfaces
// option to turn on longer test for profiling
TEST(ZincGraphicsContours, analytic_isosurfaces)
{
	ZincTestSetupCpp zinc;

    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldmodule/allshapes.ex3").c_str()));

	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());

	Timekeeper timekeeper = zinc.context.getTimekeepermodule().getDefaultTimekeeper();
	EXPECT_TRUE(timekeeper.isValid());
	EXPECT_EQ(RESULT_OK, timekeeper.setMinimumTime(0.0));
	EXPECT_EQ(RESULT_OK, timekeeper.setMinimumTime(20.0));

	Field fun1;  // fun1 = z + 0.1*sin(magnitude(x - 0.75, y - 0.75)*t)
	Field fun2;  // fun2 = fun1*sin(4*z)
	{
		ChangeManager<Fieldmodule> changeFields(zinc.fm);

		const int componentIndexes[2] = { 1, 2 };
		FieldComponent xy = zinc.fm.createFieldComponent(coordinates, 2, componentIndexes);
		EXPECT_TRUE(xy.isValid());
		EXPECT_EQ(RESULT_OK, xy.setName("xy"));
		const double offsetValues[2] = { 0.75, 0.75 };
		FieldConstant offset075 = zinc.fm.createFieldConstant(2, offsetValues);
		EXPECT_TRUE(offset075.isValid());
		EXPECT_EQ(RESULT_OK, offset075.setName("offset075"));
		FieldSubtract oxy = xy - offset075;
		EXPECT_TRUE(oxy.isValid());
		EXPECT_EQ(RESULT_OK, oxy.setName("oxy"));
		FieldMagnitude moxy = zinc.fm.createFieldMagnitude(oxy);
		EXPECT_TRUE(moxy.isValid());
		EXPECT_EQ(RESULT_OK, moxy.setName("moxy"));
		FieldTimeValue t = zinc.fm.createFieldTimeValue(timekeeper);
		EXPECT_TRUE(t.isValid());
		EXPECT_EQ(RESULT_OK, t.setName("t"));
		FieldMultiply moxyt = moxy * t;
		EXPECT_TRUE(moxyt.isValid());
		EXPECT_EQ(RESULT_OK, moxyt.setName("moxyt"));
		FieldSin sin_moxyt = zinc.fm.createFieldSin(moxyt);
		EXPECT_TRUE(sin_moxyt.isValid());
		EXPECT_EQ(RESULT_OK, sin_moxyt.setName("sin_moxyt"));
		const double scale01Value = 0.1;
		FieldConstant scale01 = zinc.fm.createFieldConstant(1, &scale01Value);
		EXPECT_TRUE(scale01.isValid());
		EXPECT_EQ(RESULT_OK, scale01.setName("scale01"));
		FieldMultiply ssin_moxyt = sin_moxyt * scale01;
		EXPECT_TRUE(ssin_moxyt.isValid());
		EXPECT_EQ(RESULT_OK, ssin_moxyt.setName("ssin_moxyt"));
		FieldComponent z = zinc.fm.createFieldComponent(coordinates, 3);
		EXPECT_TRUE(z.isValid());
		EXPECT_EQ(RESULT_OK, z.setName("z"));
		fun1 = z + ssin_moxyt;
		EXPECT_TRUE(fun1.isValid());
		EXPECT_EQ(RESULT_OK, fun1.setName("fun1"));

		const double scale4Value = 4.0;
		FieldConstant scale4 = zinc.fm.createFieldConstant(1, &scale4Value);
		EXPECT_TRUE(scale4.isValid());
		EXPECT_EQ(RESULT_OK, scale4.setName("scale4"));
		FieldMultiply sz = z * scale4;
		EXPECT_TRUE(sz.isValid());
		EXPECT_EQ(RESULT_OK, sz.setName("sz"));
		FieldSin sin_sz = zinc.fm.createFieldSin(sz);
		EXPECT_TRUE(sin_sz.isValid());
		EXPECT_EQ(RESULT_OK, sin_sz.setName("sin_sz"));
		fun2 = fun1 * sin_sz;
		EXPECT_TRUE(fun2.isValid());
		EXPECT_EQ(RESULT_OK, fun2.setName("fun2"));
	}
	//char *fieldDescription = zinc.fm.writeDescription();
	//cmzn_deallocate(fieldDescription);

	GraphicsContours contours1 =  zinc.scene.createGraphicsContours();
	EXPECT_TRUE(contours1.isValid());
	EXPECT_EQ(RESULT_OK, contours1.setCoordinateField(coordinates));
	EXPECT_EQ(RESULT_OK, contours1.setIsoscalarField(fun1));
	const double value02 = 0.2;
	EXPECT_EQ(RESULT_OK, contours1.setListIsovalues(1, &value02));

	GraphicsContours contours2 = zinc.scene.createGraphicsContours();
	EXPECT_TRUE(contours2.isValid());
	EXPECT_EQ(RESULT_OK, contours2.setCoordinateField(coordinates));
	EXPECT_EQ(RESULT_OK, contours2.setIsoscalarField(fun2));
	const double value042 = 0.42;
	EXPECT_EQ(RESULT_OK, contours2.setListIsovalues(1, &value042));

	//char *sceneDescription = zinc.scene.writeDescription();
	//cmzn_deallocate(sceneDescription);

	Tessellation tessellation = zinc.context.getTessellationmodule().getDefaultTessellation();
	EXPECT_TRUE(tessellation.isValid());
	const int number6 = 6;
	EXPECT_EQ(RESULT_OK, tessellation.setMinimumDivisions(1, &number6));
	EXPECT_EQ(RESULT_OK, timekeeper.setTime(10.5));

	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	Element element5 = mesh3d.findElementByIdentifier(5);
	EXPECT_TRUE(element5.isValid());
	Fieldcache fieldcache = zinc.fm.createFieldcache();
	const double xi050505[3] = { 0.5, 0.5, 0.5 };
	EXPECT_EQ(RESULT_OK, fieldcache.setMeshLocation(element5, 3, xi050505));
	double valueOut;
	const double TOL = 1.0E-10;
	EXPECT_EQ(RESULT_OK, fun1.evaluateReal(fieldcache, 1, &valueOut));
	EXPECT_NEAR(0.57301596730940352, valueOut, TOL);
	EXPECT_EQ(RESULT_OK, fun2.evaluateReal(fieldcache, 1, &valueOut));
	EXPECT_NEAR(0.52104194460446962, valueOut, TOL);
	double minimums[3];
	double maximums[3];
	const double GTOL = 1.0E-6;  // as graphics may be in single precision
	EXPECT_EQ(RESULT_OK, zinc.scene.getCoordinatesRange(Scenefilter(), minimums, maximums));
	EXPECT_NEAR(-1.0, minimums[0], GTOL);
	EXPECT_NEAR(0.0, minimums[1], GTOL);
	EXPECT_NEAR(0.10024128109216690, minimums[2], GTOL);
	EXPECT_NEAR(1.8797646760940552, maximums[0], GTOL);
	EXPECT_NEAR(1.8797646760940552, maximums[1], GTOL);
	EXPECT_NEAR(0.60720294713973999, maximums[2], GTOL);

	// Larger test does the above 20 times; with this on, at time of writing
	// takes ~890-970ms on Windows Release running on Intel Core i9-8950HK CPU @ 2.90 GHz
	//const int number24 = 24;
	//EXPECT_EQ(RESULT_OK, tessellation.setMinimumDivisions(1, &number24));
	//for (int i = 0; i <= 20; i += 1)
	//{
	//	const double time = i*1.0;
	//	EXPECT_EQ(RESULT_OK, timekeeper.setTime(time));
	//	EXPECT_EQ(RESULT_OK, zinc.scene.getCoordinatesRange(Scenefilter(), minimums, maximums));
	//}
}
