/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldarithmeticoperators.hpp>
#include <opencmiss/zinc/fieldassignment.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldlogicaloperators.hpp>
#include <opencmiss/zinc/fieldsubobjectgroup.hpp>
#include <opencmiss/zinc/status.hpp>

#include "utilities/zinctestsetupcpp.hpp"
#include "test_resources.h"

TEST(ZincFieldassignment, cubeOffsetScale)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodes.isValid());

	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	const double scaleValues[3] = { 1.25, 2.0, 0.75 };
	Field scale = zinc.fm.createFieldConstant(3, scaleValues);
	EXPECT_TRUE(scale.isValid());
	const double offsetValues[3] = { 0.1, 0.2, -0.1 };
	Field offset = zinc.fm.createFieldConstant(3, offsetValues);
	EXPECT_TRUE(offset.isValid());
	Field newCoordinates = (coordinates*scale) + offset;
	EXPECT_TRUE(newCoordinates.isValid());
	EXPECT_EQ(3, newCoordinates.getNumberOfComponents());

	Fieldassignment fieldassignment = coordinates.createFieldassignment(newCoordinates);
	EXPECT_TRUE(fieldassignment.isValid());
	EXPECT_EQ(coordinates, fieldassignment.getTargetField());
	EXPECT_EQ(newCoordinates, fieldassignment.getSourceField());
	EXPECT_FALSE(fieldassignment.getConditionalField().isValid());
	EXPECT_FALSE(fieldassignment.getNodeset().isValid());

	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());

	const double expectedCoordinates[8][3] = {
		{ 0.0, 0.0, 0.0 },
		{ 1.0, 0.0, 0.0 },
		{ 0.0, 1.0, 0.0 },
		{ 1.0, 1.0, 0.0 },
		{ 0.0, 0.0, 1.0 },
		{ 1.0, 0.0, 1.0 },
		{ 0.0, 1.0, 1.0 },
		{ 1.0, 1.0, 1.0 }
	};

	double x[3], newx[3];
	for (int n =  0; n < 8; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n + 1);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, cache.setNode(node));
		EXPECT_EQ(RESULT_OK, coordinates.evaluateReal(cache, 3, x));
		EXPECT_EQ(RESULT_OK, newCoordinates.evaluateReal(cache, 3, newx));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(expectedCoordinates[n][c], x[c]);
			EXPECT_EQ(expectedCoordinates[n][c]*scaleValues[c] + offsetValues[c], newx[c]);
		}
	}

	EXPECT_EQ(RESULT_OK, fieldassignment.assign());

	for (int n = 0; n < 8; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n + 1);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, cache.setNode(node));
		EXPECT_EQ(RESULT_OK, coordinates.evaluateReal(cache, 3, x));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(expectedCoordinates[n][c]*scaleValues[c] + offsetValues[c], x[c]);
		}
	}

	// reset and try nodeset and conditional field options

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	FieldNodeGroup nodeGroup = zinc.fm.createFieldNodeGroup(nodes);
	EXPECT_TRUE(nodeGroup.isValid());
	NodesetGroup nodesetGroup = nodeGroup.getNodesetGroup();
	EXPECT_TRUE(nodesetGroup.isValid());
	for (int n = 0; n < 4; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n + 1);
		EXPECT_EQ(RESULT_OK, nodesetGroup.addNode(node));
	}
	EXPECT_EQ(RESULT_OK, fieldassignment.setNodeset(nodesetGroup));
	EXPECT_EQ(nodesetGroup, fieldassignment.getNodeset());
	EXPECT_EQ(RESULT_OK, fieldassignment.assign());

	for (int n = 0; n < 8; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n + 1);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, cache.setNode(node));
		EXPECT_EQ(RESULT_OK, coordinates.evaluateReal(cache, 3, x));
		for (int c = 0; c < 3; ++c)
		{
			if (n < 4)
			{
				EXPECT_EQ(expectedCoordinates[n][c]*scaleValues[c] + offsetValues[c], x[c]);
			}
			else
			{
				EXPECT_EQ(expectedCoordinates[n][c], x[c]);
			}
		}
	}

	Nodeset noNodeset;
	EXPECT_EQ(RESULT_OK, fieldassignment.setNodeset(noNodeset));
	EXPECT_FALSE(fieldassignment.getNodeset().isValid());
	FieldNot conditionalField = zinc.fm.createFieldNot(nodeGroup);
	EXPECT_EQ(RESULT_OK, fieldassignment.setConditionalField(conditionalField));
	EXPECT_EQ(conditionalField, fieldassignment.getConditionalField());
	EXPECT_EQ(RESULT_OK, fieldassignment.assign());
	for (int n = 0; n < 8; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n + 1);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, cache.setNode(node));
		EXPECT_EQ(RESULT_OK, coordinates.evaluateReal(cache, 3, x));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(expectedCoordinates[n][c]*scaleValues[c] + offsetValues[c], x[c]);
		}
	}
}
