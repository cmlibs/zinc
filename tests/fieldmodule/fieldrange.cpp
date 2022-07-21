/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldrange.hpp>

#include "zinctestsetupcpp.hpp"

#include "test_resources.h"

TEST(ZincFieldrange, invalid_arguments)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_EX3_FIELDRANGE_CURVE_RESOURCE)));
	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_TRUE(mesh1d.isValid());

	double minimumValues[3], maximumValues[3], minimumXi, maximumXi;
	Fieldrange noFieldrange;
	EXPECT_FALSE(noFieldrange.hasValidRange());
	EXPECT_FALSE(noFieldrange.getField().isValid());
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, noFieldrange.getComponentMinimumValuesReal(1, 3, minimumValues));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, noFieldrange.getComponentMaximumValuesReal(1, 3, maximumValues));
	EXPECT_FALSE(noFieldrange.getComponentMinimumMeshLocation(1, 1, &minimumXi).isValid());
	EXPECT_FALSE(noFieldrange.getComponentMaximumMeshLocation(1, 1, &maximumXi).isValid());
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, noFieldrange.getRangeReal(3, minimumValues, maximumValues));

	Fieldcache fieldcache = zinc.fm.createFieldcache();
	EXPECT_TRUE(fieldcache.isValid());
	Fieldrange fieldrange = fieldcache.createFieldrange();
	EXPECT_TRUE(fieldrange.isValid());
	EXPECT_FALSE(fieldrange.hasValidRange());
	EXPECT_FALSE(fieldrange.getField().isValid());
	EXPECT_EQ(RESULT_ERROR_NOT_FOUND, fieldrange.getComponentMinimumValuesReal(1, 3, minimumValues));
	EXPECT_EQ(RESULT_ERROR_NOT_FOUND, fieldrange.getComponentMaximumValuesReal(1, 3, maximumValues));
	EXPECT_FALSE(fieldrange.getComponentMinimumMeshLocation(1, 1, &minimumXi).isValid());
	EXPECT_FALSE(fieldrange.getComponentMaximumMeshLocation(1, 1, &maximumXi).isValid());
	EXPECT_EQ(RESULT_ERROR_NOT_FOUND, fieldrange.getRangeReal(3, minimumValues, maximumValues));

	Element element = mesh1d.findElementByIdentifier(1);
	EXPECT_TRUE(element.isValid());
	Field noField;
	Fieldcache noFieldcache;
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, noField.evaluateFieldrange(fieldcache, fieldrange));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, coordinates.evaluateFieldrange(noFieldcache, fieldrange));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, coordinates.evaluateFieldrange(fieldcache, noFieldrange));
	EXPECT_EQ(RESULT_ERROR_NOT_IMPLEMENTED, coordinates.evaluateFieldrange(fieldcache, fieldrange));
	EXPECT_EQ(RESULT_OK, fieldcache.setElement(element));
	EXPECT_EQ(RESULT_OK, coordinates.evaluateFieldrange(fieldcache, fieldrange));
	EXPECT_TRUE(fieldrange.hasValidRange());
	EXPECT_EQ(coordinates, fieldrange.getField());

	EXPECT_EQ(RESULT_ERROR_ARGUMENT, fieldrange.getComponentMinimumValuesReal(0, 3, minimumValues));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, fieldrange.getComponentMinimumValuesReal(1, 2, minimumValues));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, fieldrange.getComponentMinimumValuesReal(1, 3, nullptr));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, fieldrange.getComponentMaximumValuesReal(0, 3, minimumValues));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, fieldrange.getComponentMaximumValuesReal(1, 2, minimumValues));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, fieldrange.getComponentMaximumValuesReal(1, 3, nullptr));
	EXPECT_FALSE(fieldrange.getComponentMinimumMeshLocation(0, 1, &minimumXi).isValid());
	EXPECT_FALSE(fieldrange.getComponentMinimumMeshLocation(1, 0, &minimumXi).isValid());
	EXPECT_FALSE(fieldrange.getComponentMinimumMeshLocation(1, 1, nullptr).isValid());
	EXPECT_FALSE(fieldrange.getComponentMaximumMeshLocation(0, 1, &maximumXi).isValid());
	EXPECT_FALSE(fieldrange.getComponentMaximumMeshLocation(1, 0, &maximumXi).isValid());
	EXPECT_FALSE(fieldrange.getComponentMaximumMeshLocation(1, 1, nullptr).isValid());
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, fieldrange.getRangeReal(2, minimumValues, maximumValues));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, fieldrange.getRangeReal(3, nullptr, maximumValues));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, fieldrange.getRangeReal(3, minimumValues, nullptr));
}

TEST(ZincFieldrange, curve1d)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_EX3_FIELDRANGE_CURVE_RESOURCE)));
	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_TRUE(mesh1d.isValid());

	Fieldcache fieldcache = zinc.fm.createFieldcache();
	EXPECT_TRUE(fieldcache.isValid());
	Fieldrange fieldrange = fieldcache.createFieldrange();
	EXPECT_TRUE(fieldrange.isValid());

	struct
	{
		double componentMinimumValues[3][3];
		double componentMaximumValues[3][3];
		double minimumXi[3];
		double maximumXi[3];
		double minimumValues[3];
		double maximumValues[3];
	} expectedRanges[2] =
	{
		{
			{
			{0.0, 0.0, 0.0},
			{0.24912167208471314, -0.26974408821590712, 0.079226496617421316},
			{0.0, 0.0, 0.0}
			},
			{
			{0.38065230165043640, -0.23504256737755236, 0.033411092842231094},
			{0.0, 0.0, 0.0},
			{0.16260363326815244, -0.24701204822831715, 0.092122379179497921}
			},
			{0.0, 0.71985801315759601, 0.0},
			{1.0, 0.0, 0.50250223707268582},
			{0.0, -0.26974408821590712, 0.0},
			{0.38065230165043640, 0.0, 0.092122379179497921},
		},
		{
			{
			{0.38065230165043640, -0.23504256737755241, 0.033411092842231142},
			{0.38065230165043640, -0.23504256737755241, 0.033411092842231142},
			{0.51745520896557362, -0.15238542343978836, 0.0062385357700960034}
			},
			{
			{0.65712625341709818, 0.081156865972584369, 0.084431483970522886},
			{0.62809215828154263, 0.19247116043021367, 0.14673530043671223},
			{0.62809215828154263, 0.19247116043021367, 0.14673530043671223}
			},
			{0.0, 0.0, 0.27514975168948341},
			{0.79813948955319292, 1.0, 1.0},
			{0.38065230165043640, -0.23504256737755241, 0.0062385357700960034},
			{0.65712625341709818, 0.19247116043021367, 0.14673530043671223},
		}
	};
	const double TOL = 1.0E-8;
	double minimumValues[3], maximumValues[3], minimumXi, maximumXi;
	for (int e = 0; e < 2; ++e)
	{
		Element element = mesh1d.findElementByIdentifier(e + 1);
		EXPECT_TRUE(element.isValid());
		EXPECT_EQ(RESULT_OK, fieldcache.setElement(element));
		EXPECT_EQ(RESULT_OK, coordinates.evaluateFieldrange(fieldcache, fieldrange));
		EXPECT_TRUE(fieldrange.hasValidRange());
		EXPECT_EQ(coordinates, fieldrange.getField());
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(RESULT_OK, fieldrange.getComponentMinimumValuesReal(c + 1, 3, minimumValues));
			EXPECT_EQ(RESULT_OK, fieldrange.getComponentMaximumValuesReal(c + 1, 3, maximumValues));
			for (int d = 0; d < 3; ++d)
			{
				EXPECT_NEAR(expectedRanges[e].componentMinimumValues[c][d], minimumValues[d], TOL);
				EXPECT_NEAR(expectedRanges[e].componentMaximumValues[c][d], maximumValues[d], TOL);
			}
			EXPECT_EQ(element, fieldrange.getComponentMinimumMeshLocation(c + 1, 1, &minimumXi));
			EXPECT_EQ(element, fieldrange.getComponentMaximumMeshLocation(c + 1, 1, &maximumXi));
			EXPECT_NEAR(expectedRanges[e].minimumXi[c], minimumXi, TOL);
			EXPECT_NEAR(expectedRanges[e].maximumXi[c], maximumXi, TOL);
		}
		EXPECT_EQ(RESULT_OK, fieldrange.getRangeReal(3, minimumValues, maximumValues));
		for (int d = 0; d < 3; ++d)
		{
			EXPECT_NEAR(expectedRanges[e].minimumValues[d], minimumValues[d], TOL);
			EXPECT_NEAR(expectedRanges[e].maximumValues[d], maximumValues[d], TOL);
		}
	}
}
