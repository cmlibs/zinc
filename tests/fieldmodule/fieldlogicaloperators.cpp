/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/element.hpp>
#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldarithmeticoperators.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldfiniteelement.hpp>
#include <opencmiss/zinc/fieldlogicaloperators.hpp>
#include <opencmiss/zinc/region.hpp>

#include "zinctestsetupcpp.hpp"

#include "test_resources.h"

// test whether find mesh location field is defined for offset coordinates 
TEST(ZincFieldIsDefined, evaluate)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_EQ(1, mesh3d.getSize());
	Element element = mesh3d.findElementByIdentifier(1);
	EXPECT_TRUE(element.isValid());
	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());

	const int nPoints = 4; // ensure even value for logic below
	const double offsetValue = 1.0 / double(nPoints);
	const double offsetValues[3] = { offsetValue, offsetValue, offsetValue };
	Field offset = zinc.fm.createFieldConstant(3, offsetValues);
	EXPECT_TRUE(offset.isValid());

	Field offsetCoordinates = coordinates + offset;
	EXPECT_TRUE(offsetCoordinates.isValid());

	FieldFindMeshLocation findLocation = zinc.fm.createFieldFindMeshLocation(coordinates, offsetCoordinates, mesh3d);
	EXPECT_TRUE(findLocation.isValid());

	FieldIsDefined isDefined = zinc.fm.createFieldIsDefined(findLocation);
	EXPECT_TRUE(isDefined.isValid());

	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());

	double xi[3];
	int numPointsFound = 0;
	for (int k = 0; k < nPoints; ++k)
	{
		xi[2] = (k + 0.5)*offsetValue;
		for (int j = 0; j < nPoints; ++j)
		{
			xi[1] = (j + 0.5)*offsetValue;
			for (int i = 0; i < nPoints; ++i)
			{
				xi[0] = (i + 0.5)*offsetValue;
				EXPECT_EQ(OK, result = cache.setMeshLocation(element, 3, xi));
				double xiOut[3], isDefinedValueOut;
				EXPECT_EQ(OK, result = isDefined.evaluateReal(cache, 1, &isDefinedValueOut));
				Element elementOut = findLocation.evaluateMeshLocation(cache, 3, xiOut);
				bool isDefinedAtLocation = findLocation.isDefinedAtLocation(cache);
				EXPECT_TRUE(isDefined.isDefinedAtLocation(cache));
				if (k && j && i)
				{
					++numPointsFound;
					EXPECT_NE(0.0, isDefinedValueOut);
					EXPECT_TRUE(isDefinedAtLocation);
					EXPECT_EQ(element, elementOut);
					for (int c = 0; c < 3; ++c)
						ASSERT_DOUBLE_EQ(xi[0] - offsetValue, xiOut[0]);
				}
				else
				{
					EXPECT_EQ(0.0, isDefinedValueOut);
					EXPECT_FALSE(isDefinedAtLocation);
					EXPECT_FALSE(elementOut.isValid());
				}
			}
		}
	}
	EXPECT_EQ((nPoints - 1)*(nPoints - 1)*(nPoints - 1), numPointsFound);
}
