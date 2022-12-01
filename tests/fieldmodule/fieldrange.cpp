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
//#include <sstream>

TEST(ZincFieldrange, invalid_arguments)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(
        resourcePath("fieldmodule/fieldrange_curve.exf").c_str()));
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
        resourcePath("fieldmodule/fieldrange_curve.exf").c_str()));
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
			{0.24912202784994236, -0.26974408821622964, 0.079226402736462781},
			{0.0, 0.0, 0.0}
			},
			{
			{0.38065230165043640, -0.23504256737755236, 0.033411092842231094},
			{0.0, 0.0, 0.0},
			{0.16260326644675097, -0.24701183785291625, 0.092122379179791616}
			},
			{0.0, 0.71985884667711109, 0.0},
			{1.0, 0.0, 0.50250124879658897},
			{0.0, -0.26974408821590712, 0.0},
			{0.38065230165043640, 0.0, 0.092122379179497921},
		},
		{
			{
			{0.38065230165043640, -0.23504256737755241, 0.033411092842231142},
			{0.38065230165043640, -0.23504256737755241, 0.033411092842231142},
			{0.51745482186161706, -0.15238572463860917, 0.0062385357698575257}
			},
			{
			{0.65712625341755504, 0.081157305028062343, 0.084431712549150828},
			{0.62809215828154263, 0.19247116043021367, 0.14673530043671223},
			{0.62809215828154263, 0.19247116043021367, 0.14673530043671223}
			},
			{0.0, 0.0, 0.27514890637427314},
			{0.79814032380632927, 1.0, 1.0},
			{0.38065230165043640, -0.23504256737755241, 0.0062385357700960034},
			{0.65712625341709818, 0.19247116043021367, 0.14673530043671223},
		}
	};
	const double TOL = 1.0E-6; // same as tolerance on find xi
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

TEST(ZincFieldrange, allshapes_quadratic_2d)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(
        resourcePath("fieldmodule/allshapes_quadratic.exf").c_str()));
	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh2d.isValid());

	Fieldcache fieldcache = zinc.fm.createFieldcache();
	EXPECT_TRUE(fieldcache.isValid());
	Fieldrange fieldrange = fieldcache.createFieldrange();
	EXPECT_TRUE(fieldrange.isValid());

	const int elementIdentifiers[5] = { 1, 8, 18, 22, 24 };
	struct
	{
		double componentMinimumValues[3][3];
		double componentMaximumValues[3][3];
		double componentMinimumXi[3][2];
		double componentMaximumXi[3][2];
		double minimumValues[3];
		double maximumValues[3];
	} expectedRanges[5] =
	{
		{  // square on cube
			{
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0}
			},
			{
			{0, 0, 0},
			{0, 1, 0},
			{0, 0, 1}
			},
			{
			{0, 0},
			{0, 0},
			{0, 0}
			},
			{
			{0, 0},
			{1, 0},
			{0, 1}
			},
			{0, 0, 0},
			{0, 1, 1}
		},
		{  // square on wedge12 face xi1 + xi2 = 1
			{
			{1, 1, 0},
			{2, 0, 0},
			{1, 1, 0}
			},
			{
			{2, 0, 0},
			{1, 1, 0},
			{1, 1, 1}
			},
			{
			{0, 0},
			{1, 0},
			{0, 0}
			},
			{
			{1, 0},
			{0, 0},
			{0, 1}
			},
			{1, 0, 0},
			{2, 1, 1}
		},
		{  // square on wedge13 face xi1 + xi3 = 1
			{
			{2, 0, 1},
			{2, 0, 1},
			{3, 0, 0}
			},
			{
			{3, 0, 0},
			{2, 1, 1},
			{2, 0, 1}
			},
			{
			{0, 0},
			{0, 0},
			{1, 0}
			},
			{
			{1, 0},
			{0, 1},
			{0, 0}
			},
			{2, 0, 0},
			{3, 1, 1}
		},
		{  // square on wedge23 face xi2 + xi3 = 1
			{
			{1, 1, 1},
			{1, 1, 1},
			{1, 2, 0}
			},
			{
			{2, 1, 1},
			{1, 2, 0},
			{1, 1, 1}
			},
			{
			{0, 0},
			{0, 0},
			{0, 1}
			},
			{
			{1, 0},
			{0, 1},
			{0, 0}
			},
			{1, 1, 0},
			{2, 2, 1}
		},
		{  // triangle on tetrahedron face xi1 + xi2 + xi3 = 1
			{
			{2, 1, 1},
			{2, 1, 1},
			{3, 1, 0}
			},
			{
			{3, 1, 0},
			{2, 2, 0},
			{2, 1, 1}
			},
			{
			{0, 0},
			{0, 0},
			{1, 0}
			},
			{
			{1, 0},
			{0, 1},
			{0, 0}
			},
			{2, 1, 0},
			{3, 2, 1}
		}
	};
	const double TOL = 1.0E-6; // same as tolerance on find xi
	double minimumValues[3], maximumValues[3], minimumXi[3], maximumXi[3];
	//std::stringstream output;
	//output << std::setprecision(10);
	for (int e = 0; e < 5; ++e)
	{
		Element element = mesh2d.findElementByIdentifier(elementIdentifiers[e]);
		EXPECT_TRUE(element.isValid());
		EXPECT_EQ(RESULT_OK, fieldcache.setElement(element));
		EXPECT_EQ(RESULT_OK, coordinates.evaluateFieldrange(fieldcache, fieldrange));
		EXPECT_TRUE(fieldrange.hasValidRange());
		EXPECT_EQ(coordinates, fieldrange.getField());
		//output << "\t\t{\n\t\t\t{\n";
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(RESULT_OK, fieldrange.getComponentMinimumValuesReal(c + 1, 3, minimumValues));
			//output << "\t\t\t{" << minimumValues[0] << ", " << minimumValues[1] << ", " << minimumValues[2] << "},\n";
			for (int d = 0; d < 3; ++d)
			{
				EXPECT_NEAR(expectedRanges[e].componentMinimumValues[c][d], minimumValues[d], TOL);
			}
		}
		//output << "\t\t\t},\n\t\t\t{\n";
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(RESULT_OK, fieldrange.getComponentMaximumValuesReal(c + 1, 3, maximumValues));
			//output << "\t\t\t{" << maximumValues[0] << ", " << maximumValues[1] << ", " << maximumValues[2] << "},\n";
			for (int d = 0; d < 3; ++d)
			{
				EXPECT_NEAR(expectedRanges[e].componentMaximumValues[c][d], maximumValues[d], TOL);
			}
		}
		//output << "\t\t\t},\n\t\t\t{\n";
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(element, fieldrange.getComponentMinimumMeshLocation(c + 1, 2, minimumXi));
			//output << "\t\t\t{" << minimumXi[0] << ", " << minimumXi[1] << "},\n";
			for (int d = 0; d < 2; ++d)
			{
				EXPECT_NEAR(expectedRanges[e].componentMinimumXi[c][d], minimumXi[d], TOL);
			}
		}
		//output << "\t\t\t},\n\t\t\t{\n";
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(element, fieldrange.getComponentMaximumMeshLocation(c + 1, 2, maximumXi));
			//output << "\t\t\t{" << maximumXi[0] << ", " << maximumXi[1] << "},\n";
			for (int d = 0; d < 2; ++d)
			{
				EXPECT_NEAR(expectedRanges[e].componentMaximumXi[c][d], maximumXi[d], TOL);
			}
		}
		//output << "\t\t\t},\n";
		EXPECT_EQ(RESULT_OK, fieldrange.getRangeReal(3, minimumValues, maximumValues));
		//output << "\t\t\t{" << minimumValues[0] << ", " << minimumValues[1] << ", " << minimumValues[2] << "},\n";
		//output << "\t\t\t{" << maximumValues[0] << ", " << maximumValues[1] << ", " << maximumValues[2] << "}\n";
		//output << "\t\t},\n";
		for (int d = 0; d < 3; ++d)
		{
			EXPECT_NEAR(expectedRanges[e].minimumValues[d], minimumValues[d], TOL);
			EXPECT_NEAR(expectedRanges[e].maximumValues[d], maximumValues[d], TOL);
		}
	}
	//std::cerr << output.str();
}

TEST(ZincFieldrange, allshapes_quadratic_deformed_2d)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(
        resourcePath("fieldmodule/allshapes_quadratic_deformed.exf").c_str()));
	Field deformed = zinc.fm.findFieldByName("deformed");
	EXPECT_TRUE(deformed.isValid());
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh2d.isValid());

	Fieldcache fieldcache = zinc.fm.createFieldcache();
	EXPECT_TRUE(fieldcache.isValid());
	Fieldrange fieldrange = fieldcache.createFieldrange();
	EXPECT_TRUE(fieldrange.isValid());

	const int elementIdentifiers[5] = { 1, 8, 18, 22, 24 };
	struct
	{
		double componentMinimumValues[3][3];
		double componentMaximumValues[3][3];
		double componentMinimumXi[3][2];
		double componentMaximumXi[3][2];
		double minimumValues[3];
		double maximumValues[3];
	} expectedRanges[5] =
	{
		{  // square on cube
			{
			{-0.3313611656, 0.71983615, 0.4905697351},
			{-0.05, -0.05, 0.5},
			{-0.1, 0, -0.1},
			},
			{
			{0.1, 1.1, 1},
			{0.1, 1.1, 1},
			{0, 0.5, 1.1},
			},
			{
			{0.7517595803, 0.4911085377},
			{0, 0.5},
			{0, 0},
			},
			{
			{1, 1},
			{1, 1},
			{0.5, 1},
			},
			{-0.3313611656, -0.05, -0.1},
			{0.1, 1.1, 1.1}
		},
		{  // square on wedge12 face xi1 + xi2 = 1
			{
			{1, 0.75, 0},
			{2, 0, 0},
			{1, 0.75, 0},
			},
			{
			{2.025, 0, 0.25},
			{1.25, 0.84375, 0},
			{1.5, 0.5, 1.2},
			},
			{
			{0, 0},
			{1, 0},
			{0, 0},
			},
			{
			{1, 0.25},
			{0.25, 0},
			{0.5, 1},
			},
			{1, 0, 0},
			{2.025, 0.84375, 1.2}
		},
		{  // square on wedge13 face xi1 + xi3 = 1
			{
			{1.8, 0, 1},
			{2.8, -0.1, 0},
			{2.8, 1, 0},
			},
			{
			{3.1, 0.5, 0},
			{2, 1, 1},
			{2, 0.5, 1.2},
			},
			{
			{0, 0},
			{1, 0},
			{1, 1},
			},
			{
			{1, 0.5},
			{0, 1},
			{0, 0.5},
			},
			{1.8, -0.1, 0},
			{3.1, 1, 1.2}
		},
		{  // square on wedge23 face xi2 + xi3 = 1
			{
			{1, 0.7, 1},
			{1, 0.7, 1},
			{1, 2, -0.02},
			},
			{
			{2.60025, 1.88984, 0.51},
			{2.340234375, 2.05625, 0.1875},
			{1.5, 1, 1.2},
			},
			{
			{0, 0},
			{0, 0},
			{0, 1},
			},
			{
			{1, 0.49},
			{1, 0.8125},
			{0.5, 0},
			},
			{1, 0.7, -0.02},
			{2.60025, 2.05625, 1.2}
		},
		{  // triangle on tetrahedron face xi1 + xi2 + xi3 = 1
			{
			{1.95, 2, 0},
			{2, 1, 1},
			{1.95, 2, 0},
			},
			{
			{3.161732456, 1.302955236, 0.4672691264},
			{2.340234375, 2.05625, 0.1875},
			{2.65, 1, 1.125},
			},
			{
			{0, 1},
			{0, 0},
			{0, 1},
			},
			{
			{0.5548239968, 0.22368449},
			{0, 0.8125},
			{0.25, 0},
			},
			{1.95, 1, 0},
			{3.161732456, 2.05625, 1.125}
		}
	};
	const double TOL = 1.0E-6; // same as tolerance on find xi
	double minimumValues[3], maximumValues[3], minimumXi[3], maximumXi[3];
	//std::stringstream output;
	//output << std::setprecision(10);
	for (int e = 0; e < 5; ++e)
	{
		Element element = mesh2d.findElementByIdentifier(elementIdentifiers[e]);
		EXPECT_TRUE(element.isValid());
		EXPECT_EQ(RESULT_OK, fieldcache.setElement(element));
		EXPECT_EQ(RESULT_OK, deformed.evaluateFieldrange(fieldcache, fieldrange));
		EXPECT_TRUE(fieldrange.hasValidRange());
		EXPECT_EQ(deformed, fieldrange.getField());
		//output << "\t\t{\n\t\t\t{\n";
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(RESULT_OK, fieldrange.getComponentMinimumValuesReal(c + 1, 3, minimumValues));
			//output << "\t\t\t{" << minimumValues[0] << ", " << minimumValues[1] << ", " << minimumValues[2] << "},\n";
			for (int d = 0; d < 3; ++d)
			{
				EXPECT_NEAR(expectedRanges[e].componentMinimumValues[c][d], minimumValues[d], TOL);
			}
		}
		//output << "\t\t\t},\n\t\t\t{\n";
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(RESULT_OK, fieldrange.getComponentMaximumValuesReal(c + 1, 3, maximumValues));
			//output << "\t\t\t{" << maximumValues[0] << ", " << maximumValues[1] << ", " << maximumValues[2] << "},\n";
			for (int d = 0; d < 3; ++d)
			{
				EXPECT_NEAR(expectedRanges[e].componentMaximumValues[c][d], maximumValues[d], TOL);
			}
		}
		//output << "\t\t\t},\n\t\t\t{\n";
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(element, fieldrange.getComponentMinimumMeshLocation(c + 1, 2, minimumXi));
			//output << "\t\t\t{" << minimumXi[0] << ", " << minimumXi[1] << "},\n";
			for (int d = 0; d < 2; ++d)
			{
				EXPECT_NEAR(expectedRanges[e].componentMinimumXi[c][d], minimumXi[d], TOL);
			}
		}
		//output << "\t\t\t},\n\t\t\t{\n";
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(element, fieldrange.getComponentMaximumMeshLocation(c + 1, 2, maximumXi));
			//output << "\t\t\t{" << maximumXi[0] << ", " << maximumXi[1] << "},\n";
			for (int d = 0; d < 2; ++d)
			{
				EXPECT_NEAR(expectedRanges[e].componentMaximumXi[c][d], maximumXi[d], TOL);
			}
		}
		//output << "\t\t\t},\n";
		EXPECT_EQ(RESULT_OK, fieldrange.getRangeReal(3, minimumValues, maximumValues));
		//output << "\t\t\t{" << minimumValues[0] << ", " << minimumValues[1] << ", " << minimumValues[2] << "},\n";
		//output << "\t\t\t{" << maximumValues[0] << ", " << maximumValues[1] << ", " << maximumValues[2] << "}\n";
		//output << "\t\t},\n";
		for (int d = 0; d < 3; ++d)
		{
			EXPECT_NEAR(expectedRanges[e].minimumValues[d], minimumValues[d], TOL);
			EXPECT_NEAR(expectedRanges[e].maximumValues[d], maximumValues[d], TOL);
		}
	}
	//std::cerr << output.str();
}

TEST(ZincFieldrange, allshapes_quadratic_3d)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(
        resourcePath("fieldmodule/allshapes_quadratic.exf").c_str()));
	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());

	Fieldcache fieldcache = zinc.fm.createFieldcache();
	EXPECT_TRUE(fieldcache.isValid());
	Fieldrange fieldrange = fieldcache.createFieldrange();
	EXPECT_TRUE(fieldrange.isValid());

	struct
	{
		double componentMinimumValues[3][3];
		double componentMaximumValues[3][3];
		double componentMinimumXi[3][3];
		double componentMaximumXi[3][3];
		double minimumValues[3];
		double maximumValues[3];
	} expectedRanges[6] =
	{
		{  // cube
			{
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0}
			},
			{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1}
			},
			{
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0}
			},
			{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1}
			},
			{0, 0, 0},
			{1, 1, 1}
		},
		{  // wedge12 #1
			{
			{1, 0, 0},
			{1, 0, 0},
			{1, 0, 0}
			},
			{
			{2, 0, 0},
			{1, 1, 0},
			{1, 0, 1}
			},
			{
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0}
			},
			{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1}
			},
			{1, 0, 0},
			{2, 1, 1}
		},
		{  // wedge12 #2
			{
			{1, 1, 0},
			{2, 0, 0},
			{1, 1, 0}
			},
			{
			{2, 0, 0},
			{1, 1, 0},
			{1, 1, 1}
			},
			{
			{0, 0, 0},
			{1, 0, 0},
			{0, 0, 0}
			},
			{
			{1, 0, 0},
			{0, 0, 0},
			{0, 0, 1}
			},
			{1, 0, 0},
			{2, 1, 1}
		},
		{  // wedge13
			{
			{2, 0, 0},
			{2, 0, 0},
			{2, 0, 0}
			},
			{
			{3, 0, 0},
			{2, 1, 0},
			{2, 0, 1}
			},
			{
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0}
			},
			{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1}
			},
			{2, 0, 0},
			{3, 1, 1}
		},
		{  // wedge23
			{
			{1, 1, 0},
			{1, 1, 0},
			{1, 1, 0}
			},
			{
			{2, 1, 0},
			{1, 2, 0},
			{1, 1, 1}
			},
			{
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0}
			},
			{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1}
			},
			{1, 1, 0},
			{2, 2, 1}
		},
		{  // tetrahedron
			{
			{2, 1, 0},
			{2, 1, 0},
			{2, 1, 0}
			},
			{
			{3, 1, 0},
			{2, 2, 0},
			{2, 1, 1}
			},
			{
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0}
			},
			{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1}
			},
			{2, 1, 0},
			{3, 2, 1}
		}
	};
	const double TOL = 1.0E-6; // same as tolerance on find xi
	double minimumValues[3], maximumValues[3], minimumXi[3], maximumXi[3];
	//std::stringstream output;
	//output << std::setprecision(10);
	for (int e = 0; e < 6; ++e)
	{
		Element element = mesh3d.findElementByIdentifier(e + 1);
		EXPECT_TRUE(element.isValid());
		EXPECT_EQ(RESULT_OK, fieldcache.setElement(element));
		EXPECT_EQ(RESULT_OK, coordinates.evaluateFieldrange(fieldcache, fieldrange));
		EXPECT_TRUE(fieldrange.hasValidRange());
		EXPECT_EQ(coordinates, fieldrange.getField());
		//output << "\t\t{\n\t\t\t{\n";
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(RESULT_OK, fieldrange.getComponentMinimumValuesReal(c + 1, 3, minimumValues));
			//output << "\t\t\t{" << minimumValues[0] << ", " << minimumValues[1] << ", " << minimumValues[2] << "},\n";
			for (int d = 0; d < 3; ++d)
			{
				EXPECT_NEAR(expectedRanges[e].componentMinimumValues[c][d], minimumValues[d], TOL);
			}
		}
		//output << "\t\t\t},\n\t\t\t{\n";
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(RESULT_OK, fieldrange.getComponentMaximumValuesReal(c + 1, 3, maximumValues));
			//output << "\t\t\t{" << maximumValues[0] << ", " << maximumValues[1] << ", " << maximumValues[2] << "},\n";
			for (int d = 0; d < 3; ++d)
			{
				EXPECT_NEAR(expectedRanges[e].componentMaximumValues[c][d], maximumValues[d], TOL);
			}
		}
		//output << "\t\t\t},\n\t\t\t{\n";
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(element, fieldrange.getComponentMinimumMeshLocation(c + 1, 3, minimumXi));
			//output << "\t\t\t{" << minimumXi[0] << ", " << minimumXi[1] << ", " << minimumXi[2] << "},\n";
			for (int d = 0; d < 3; ++d)
			{
				EXPECT_NEAR(expectedRanges[e].componentMinimumXi[c][d], minimumXi[d], TOL);
			}
		}
		//output << "\t\t\t},\n\t\t\t{\n";
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(element, fieldrange.getComponentMaximumMeshLocation(c + 1, 3, maximumXi));
			//output << "\t\t\t{" << maximumXi[0] << ", " << maximumXi[1] << ", " << maximumXi[2] << "},\n";
			for (int d = 0; d < 3; ++d)
			{
				EXPECT_NEAR(expectedRanges[e].componentMaximumXi[c][d], maximumXi[d], TOL);
			}
		}
		//output << "\t\t\t},\n";
		EXPECT_EQ(RESULT_OK, fieldrange.getRangeReal(3, minimumValues, maximumValues));
		//output << "\t\t\t{" << minimumValues[0] << ", " << minimumValues[1] << ", " << minimumValues[2] << "},\n";
		//output << "\t\t\t{" << maximumValues[0] << ", " << maximumValues[1] << ", " << maximumValues[2] << "}\n";
		//output << "\t\t},\n";
		for (int d = 0; d < 3; ++d)
		{
			EXPECT_NEAR(expectedRanges[e].minimumValues[d], minimumValues[d], TOL);
			EXPECT_NEAR(expectedRanges[e].maximumValues[d], maximumValues[d], TOL);
		}
	}
	//std::cerr << output.str();
}

TEST(ZincFieldrange, allshapes_quadratic_deformed_3d)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(
        resourcePath("fieldmodule/allshapes_quadratic_deformed.exf").c_str()));
	Field deformed = zinc.fm.findFieldByName("deformed");
	EXPECT_TRUE(deformed.isValid());
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());

	Fieldcache fieldcache = zinc.fm.createFieldcache();
	EXPECT_TRUE(fieldcache.isValid());
	Fieldrange fieldrange = fieldcache.createFieldrange();
	EXPECT_TRUE(fieldrange.isValid());

	struct
	{
		double componentMinimumValues[3][3];
		double componentMaximumValues[3][3];
		double componentMinimumXi[3][3];
		double componentMaximumXi[3][3];
		double minimumValues[3];
		double maximumValues[3];
	} expectedRanges[6] =
	{
		{  // cube
			{
			{-0.3313611656, 0.71983615, 0.4905697351},
			{-0.05, -0.05, 0.5},
			{-0.1, 0, -0.1},
			},
			{
			{1.1, 0.4, 0.5},
			{0.1, 1.1, 1},
			{0, 0.5, 1.1},
			},
			{
			{0, 0.7517595803, 0.4911085377},
			{0, 0, 0.5},
			{0, 0, 0},
			},
			{
			{1, 0.5, 0.5},
			{0, 1, 1},
			{0, 0.5, 1},
			},
			{-0.3313611656, -0.05, -0.1},
			{1.1, 1.1, 1.1}
		},
		{  // wedge12 #1
			{
			{0.95, 0, 0},
			{1.5, -0.05, -0.1},
			{1.5, -0.05, -0.1},
			},
			{
			{2.025, 0, 0.25},
			{1.25, 0.84375, 0},
			{1.5, 0.5, 1.2},
			},
			{
			{0, 0, 0},
			{0.5, 0, 0},
			{0.5, 0, 0},
			},
			{
			{1, 0, 0.25},
			{0.25, 0.75, 0},
			{0.5, 0.5, 1},
			},
			{0.95, -0.05, -0.1},
			{2.025, 0.84375, 1.2}
		},
		{  // wedge12 #2
			{
			{1, 0.75, 0},
			{2, 0, 0},
			{1.2375, 0.90625, -0.0125},
			},
			{
			{2.1, 1, 0.1},
			{1.745312501, 1.03828125, 0.8015603891},
			{1.688888889, 0.7, 1.266666667},
			},
			{
			{0, 0, 0},
			{1, 0, 0},
			{0, 0.25, 0},
			},
			{
			{0, 1, 0},
			{0, 0.7500000007, 0.7499983763},
			{0.3333333333, 0.3333333333, 1},
			},
			{1, 0, -0.0125},
			{2.1, 1.03828125, 1.266666667}
		},
		{  // wedge13
			{
			{1.8, 0, 1},
			{2.8, -0.1, 0},
			{1.9875, 0.25, -0.0125},
			},
			{
			{3.1, 0.5, 0},
			{2.1, 1, 0.1},
			{2, 0.5, 1.2},
			},
			{
			{0, 0, 1},
			{1, 0, 0},
			{0, 0.25, 0},
			},
			{
			{1, 0.5, 0},
			{0, 1, 0},
			{0, 0.5, 1},
			},
			{1.8, -0.1, -0.0125},
			{3.1, 1, 1.2}
		},
		{  // wedge23
			{
			{1, 0.75, 0},
			{1, 0.69375, 0.75},
			{1, 2, -0.02},
			},
			{
			{2.60025, 1.88984, 0.51},
			{2.340234375, 2.05625, 0.1875},
			{1.5, 1, 1.2},
			},
			{
			{0, 0, 0},
			{0, 0, 0.75},
			{0, 1, 0},
			},
			{
			{1, 0.49, 0.51},
			{1, 0.8125, 0.1875},
			{0.5, 0, 1},
			},
			{1, 0.69375, -0.02},
			{2.60025, 2.05625, 1.2}
		},
		{  // tetrahedron
			{
			{1.95, 2, 0},
			{2.1, 1, 0.1},
			{2.6625, 1, -0.0125},
			},
			{
			{3.161732456, 1.302954766, 0.4672687855},
			{2.340234375, 2.05625, 0.1875},
			{2.65, 1, 1.125},
			},
			{
			{0, 1, 0},
			{0, 0, 0},
			{0.75, 0, 0},
			},
			{
			{0.5548245479, 0.2236842162, 0.2214912359},
			{0, 0.8125, 0.1875},
			{0.25, 0, 0.75},
			},
			{1.95, 1, -0.0125},
			{3.161732456, 2.05625, 1.125}
		}
	};
	const double TOL = 1.0E-6; // same as tolerance on find xi
	double minimumValues[3], maximumValues[3], minimumXi[3], maximumXi[3];
	//std::stringstream output;
	//output << std::setprecision(10);
	for (int e = 0; e < 6; ++e)
	{
		Element element = mesh3d.findElementByIdentifier(e + 1);
		EXPECT_TRUE(element.isValid());
		EXPECT_EQ(RESULT_OK, fieldcache.setElement(element));
		EXPECT_EQ(RESULT_OK, deformed.evaluateFieldrange(fieldcache, fieldrange));
		EXPECT_TRUE(fieldrange.hasValidRange());
		EXPECT_EQ(deformed, fieldrange.getField());
		//output << "\t\t{\n\t\t\t{\n";
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(RESULT_OK, fieldrange.getComponentMinimumValuesReal(c + 1, 3, minimumValues));
			//output << "\t\t\t{" << minimumValues[0] << ", " << minimumValues[1] << ", " << minimumValues[2] << "},\n";
			for (int d = 0; d < 3; ++d)
			{
				EXPECT_NEAR(expectedRanges[e].componentMinimumValues[c][d], minimumValues[d], TOL);
			}
		}
		//output << "\t\t\t},\n\t\t\t{\n";
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(RESULT_OK, fieldrange.getComponentMaximumValuesReal(c + 1, 3, maximumValues));
			//output << "\t\t\t{" << maximumValues[0] << ", " << maximumValues[1] << ", " << maximumValues[2] << "},\n";
			for (int d = 0; d < 3; ++d)
			{
				EXPECT_NEAR(expectedRanges[e].componentMaximumValues[c][d], maximumValues[d], TOL);
			}
		}
		//output << "\t\t\t},\n\t\t\t{\n";
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(element, fieldrange.getComponentMinimumMeshLocation(c + 1, 3, minimumXi));
			//output << "\t\t\t{" << minimumXi[0] << ", " << minimumXi[1] << ", " << minimumXi[2] << "},\n";
			for (int d = 0; d < 3; ++d)
			{
				EXPECT_NEAR(expectedRanges[e].componentMinimumXi[c][d], minimumXi[d], TOL);
			}
		}
		//output << "\t\t\t},\n\t\t\t{\n";
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(element, fieldrange.getComponentMaximumMeshLocation(c + 1, 3, maximumXi));
			//output << "\t\t\t{" << maximumXi[0] << ", " << maximumXi[1] << ", " << maximumXi[2] << "},\n";
			for (int d = 0; d < 3; ++d)
			{
				EXPECT_NEAR(expectedRanges[e].componentMaximumXi[c][d], maximumXi[d], TOL);
			}
		}
		//output << "\t\t\t},\n";
		EXPECT_EQ(RESULT_OK, fieldrange.getRangeReal(3, minimumValues, maximumValues));
		//output << "\t\t\t{" << minimumValues[0] << ", " << minimumValues[1] << ", " << minimumValues[2] << "},\n";
		//output << "\t\t\t{" << maximumValues[0] << ", " << maximumValues[1] << ", " << maximumValues[2] << "}\n";
		//output << "\t\t},\n";
		for (int d = 0; d < 3; ++d)
		{
			EXPECT_NEAR(expectedRanges[e].minimumValues[d], minimumValues[d], TOL);
			EXPECT_NEAR(expectedRanges[e].maximumValues[d], maximumValues[d], TOL);
		}
	}
	//std::cerr << output.str();
}
