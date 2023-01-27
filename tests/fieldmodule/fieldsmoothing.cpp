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
#include <opencmiss/zinc/fieldfiniteelement.hpp>
#include <opencmiss/zinc/fieldsmoothing.hpp>
#include <opencmiss/zinc/node.hpp>
#include <opencmiss/zinc/status.hpp>

#include "zinctestsetupcpp.hpp"

#include "test_resources.h"

namespace {

void Field_zeroNodeDerivatives(Field &field)
{
	Fieldmodule fm = field.getFieldmodule();
	fm.beginChange();
	FieldFiniteElement feField = field.castFiniteElement();
	Nodeset nodeset = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());
	Fieldcache cache = fm.createFieldcache();
	const double zero3[3] = { 0.0, 0.0, 0.0 };
	Nodeiterator iter = nodeset.createNodeiterator();
	Node node;
	while ((node = iter.next()).isValid())
	{
		EXPECT_EQ(OK, cache.setNode(node));
		EXPECT_EQ(OK, feField.setNodeParameters(cache, /*componentNumber=all*/-1, Node::VALUE_LABEL_D_DS1, /*version*/1, 3, zero3));
		EXPECT_EQ(OK, feField.setNodeParameters(cache, /*componentNumber=all*/-1, Node::VALUE_LABEL_D_DS2, /*version*/1, 3, zero3));
		EXPECT_EQ(OK, feField.setNodeParameters(cache, /*componentNumber=all*/-1, Node::VALUE_LABEL_D_DS3, /*version*/1, 3, zero3));
	}
	fm.endChange();
}

}

TEST(ZincFieldsmoothing, smoothHermiteCubes)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("fieldio/twohermitecubes_noscalefactors.exfile").c_str()));
	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	Mesh mesh = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh.isValid());
	Element element[2];
	for (int e = 0; e < 2; ++e)
	{
		element[e] = mesh.findElementByIdentifier(e + 1);
		EXPECT_TRUE(element[e].isValid());
	};
	const double xi[3][3] = 
	{
		{ 0.1, 0.1, 0.1 },
		{ 0.5, 0.5, 0.5 },
		{ 0.9, 0.2, 0.9 }
	};
	Fieldcache cache = zinc.fm.createFieldcache();
	//test derivatives?
	Fieldsmoothing smoothing = zinc.fm.createFieldsmoothing();
	EXPECT_TRUE(smoothing.isValid());
	EXPECT_EQ(OK, smoothing.setAlgorithm(Fieldsmoothing::ALGORITHM_AVERAGE_DELTA_DERIVATIVES_UNSCALED));
	EXPECT_EQ(ERROR_ARGUMENT, smoothing.setAlgorithm(Fieldsmoothing::ALGORITHM_INVALID));
	const double tol = 1.0E-7;

	const double expectedValues1[6][3] =
	{
		{ 0.1, 0.1, 0.1 },
		{ 0.5, 0.5, 0.5 },
		{ 0.9, 0.2, 0.9 },
		{ 1.1, 0.1, 0.1 },
		{ 1.5, 0.5, 0.5 },
		{ 1.9, 0.2, 0.9 }
	};
	double values1[6][3];
	for (int v = 0; v < 6; ++v)
	{
		EXPECT_EQ(OK, cache.setMeshLocation(element[v / 3], 3, xi[v % 3]));
		EXPECT_EQ(OK, coordinates.evaluateReal(cache, 3, values1[v]));
		for (int c = 0; c < 3; ++c)
			EXPECT_NEAR(expectedValues1[v][c], values1[v][c], tol);
	}

	Field_zeroNodeDerivatives(coordinates);

	const double expectedValues2[6][3] =
	{
		{ 0.028, 0.028, 0.028 },
		{ 0.5, 0.5, 0.5 },
		{ 0.972, 0.104, 0.972 },
		{ 1.028, 0.028, 0.028 },
		{ 1.5, 0.5, 0.5 },
		{ 1.972, 0.104, 0.972 },
	};
	double values2[6][3];
	for (int v = 0; v < 6; ++v)
	{
		EXPECT_EQ(OK, cache.setMeshLocation(element[v / 3], 3, xi[v % 3]));
		EXPECT_EQ(OK, coordinates.evaluateReal(cache, 3, values2[v]));
		for (int c = 0; c < 3; ++c)
			EXPECT_NEAR(expectedValues2[v][c], values2[v][c], tol);
	}

	EXPECT_EQ(OK, coordinates.smooth(smoothing));

	for (int v = 0; v < 6; ++v)
	{
		EXPECT_EQ(OK, cache.setMeshLocation(element[v / 3], 3, xi[v % 3]));
		EXPECT_EQ(OK, coordinates.evaluateReal(cache, 3, values1[v]));
		for (int c = 0; c < 3; ++c)
			EXPECT_NEAR(expectedValues1[v][c], values1[v][c], tol);
	}

	// distort the mesh by moving one node then re-smooth
	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());
	Node node8 = nodeset.findNodeByIdentifier(8);
	EXPECT_TRUE(node8.isValid());
	const double node8_coordinates[3] = { 1.4, -0.4, 1.5 };
	EXPECT_EQ(OK, cache.setNode(node8));
	EXPECT_EQ(OK, coordinates.assignReal(cache, 3, node8_coordinates));
	EXPECT_EQ(OK, coordinates.smooth(smoothing));
	
	const double expectedValues3[6][3] =
	{
		{ 0.10194785920000003, 0.098052140799999965, 0.10243482399999999 },
		{ 0.56250000000000011, 0.43750000000000028, 0.57812500000000000 },
		{ 1.1803838976000014, -0.080383897599999393, 1.2504798719999988 },
		{ 1.1371055167999995, 0.062894483200000004, 0.14638189599999998 },
		{ 1.5624999999999993, 0.43749999999999994, 0.57812500000000000 },
		{ 1.9362041343999974, 0.16379586559999926, 0.94525516800000020 }
	};
	double values3[6][3];
	for (int v = 0; v < 6; ++v)
	{
		EXPECT_EQ(OK, cache.setMeshLocation(element[v / 3], 3, xi[v % 3]));
		EXPECT_EQ(OK, coordinates.evaluateReal(cache, 3, values3[v]));
		for (int c = 0; c < 3; ++c)
			EXPECT_NEAR(expectedValues3[v][c], values3[v][c], tol);
	}
}
